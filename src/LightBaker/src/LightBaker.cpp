#include "LightBaker.h"

#include "Console.h"
#include "Game/World.h"
#include "Game/Entity.h"
#include "Game/EntityComponent.h"
#include "Game/Components/ModelComponent.h"
#include "Game/Components/PrimitiveComponent.h"
#include "Rendering/RenderScene.h"
#include "Rendering/RenderProxies.h"
#include "Assets/ModelAsset.h"
#include "Assets/Scene.h"
#include "Math/Math.h"
#include <Util/FStream.h>
#include <cstring>
#include <limits>
#include <iomanip>
#include <iostream>
#include <random>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <algorithm>

#define STBRP_STATIC
#define STB_RECT_PACK_IMPLEMENTATION
#include "ImGui/imstb_rectpack.h"

static uint32 LIGHTDATA_MAGIC = 0x544C4442; // "TLDB"
static uint16 LIGHTDATA_VERSION = 2;
static float SHADOW_BIAS = 0.5f;
static uint32 ATLAS_PADDING = 2;
static uint32 LIGHTMAP_BLEED_PIXELS = 2;

struct FLightDataHeader
{
	uint32 magic = LIGHTDATA_MAGIC;
	uint16 version = LIGHTDATA_VERSION;
	uint16 reserved = 0;
	uint32 atlasCount = 0;
	uint32 meshCount = 0;
};

struct FBakeLight
{
	CLightProxy::EType type = CLightProxy::POINT_LIGHT;
	ELightBakeMode bakeMode = LIGHT_BAKE_NONE;
	FVector position;
	FVector direction;
	FVector color;
	float range = 0.0f;
	float intensity = 1.0f;
	bool bCastShadows = false;
	bool bEnabled = false;
};

struct FBakeMeshTask
{
	CModelComponent* modelComp = nullptr;
	CModelAsset* model = nullptr;
	const FMesh* mesh = nullptr;
	uint32 meshIndex = 0;
	FMatrix transform;
	FQuaternion rotation;
	FVector scale;
	uint32 rectW = 0;
	uint32 rectH = 0;
	int32 atlasIndex = -1;
	int32 rectX = 0;
	int32 rectY = 0;
	FVector2 lightmapPos = FVector2(0.0f);
	FVector2 lightmapScale = FVector2(1.0f);
};

static bool RayCastScene(CWorld* world, const FVector& raypos, const FVector& dir, FPrimitiveHitInfo* outHit, float maxDistance = 0.0f, const CPrimitiveProxy* ignoreProxy = nullptr)
{
	float closesthit = maxDistance > 0.f ? maxDistance : FLT_MAX;
	bool r = false;

	for (auto* p : world->GetPrimitives())
	{
		if (ignoreProxy && p == ignoreProxy)
			continue;
		CModelComponent* modelComp = Cast<CModelComponent>(p->GetOwner());
		if (modelComp && (modelComp->renderLayer & R_LAYER_LIGHTMAP) != 0)
		{
			if (!FMath::RayAABB(modelComp->Bounds(), FRay(raypos, dir)))
				continue;

			TObjectPtr<CModelAsset> model = modelComp->GetModel();
			if (model)
			{
				const TArray<FMesh>& meshes = model->GetMeshes();
				FVector position = modelComp->GetWorldPosition();
				FQuaternion rotation = modelComp->GetWorldRotation();
				FVector scale = modelComp->GetWorldScale();
				FMatrix transform = (FMatrix(1.0f).Translate(position) * rotation).Scale(scale);

				for (auto& mesh : meshes)
				{
					if (mesh.bSkinnedMesh || !mesh.vertexData || !mesh.indexData || mesh.topologyType != FMesh::TOPOLOGY_TRIANGLES)
						continue;

					uint faceCount = mesh.numIndexData / 3;
					for (uint i = 0; i < faceCount; ++i)
					{
						uint i0 = mesh.indexData[(i * 3)];
						uint i1 = mesh.indexData[(i * 3) + 1];
						uint i2 = mesh.indexData[(i * 3) + 2];
						if (i0 > mesh.numVertexData)
							continue;

						FVector v0 = ((FVertex*)mesh.vertexData)[i0].position;
						FVector v1 = ((FVertex*)mesh.vertexData)[i1].position;
						FVector v2 = ((FVertex*)mesh.vertexData)[i2].position;

						v0 = v0 * transform;
						v1 = v1 * transform;
						v2 = v2 * transform;

						float dist;
						FVector pos;
						FVector normal;

						bool bHit = FMath::RayTriangle(v0, v1, v2, FRay(raypos, dir), dist, &pos, &normal);
						if (bHit && dist < closesthit && dist > 0.f)
						{
							r = true;
							closesthit = dist;
							if (outHit)
							{
								outHit->distance = dist;
								outHit->hitProxy = p;
								outHit->normal = normal;
								outHit->position = pos;
								outHit->hitFace = i * 3;
								outHit->materialIndex = mesh.materialIndex;
							}
						}
					}
				}
			}
			continue;
		}
	}

	return r;
}

static float Cross2D(const FVector2& a, const FVector2& b)
{
	return a.x * b.y - a.y * b.x;
}

static float TriangleArea2D(const FVector2& a, const FVector2& b, const FVector2& c)
{
	return FMath::Abs(Cross2D(b - a, c - a));
}

static float Rand01()
{
	thread_local std::mt19937 rng(std::random_device{}());
	thread_local std::uniform_real_distribution<float> dist(0.0f, 1.0f);
	return dist(rng);
}

static void PrintProgress(const char* label, SizeType current, SizeType total)
{
	if (total == 0)
		return;

	const int barWidth = 30;
	float ratio = (float)current / (float)total;
	int filled = (int)(ratio * barWidth);
	int percent = (int)(ratio * 100.0f);

	static int lastPercent = -1;
	static char lastLabel[126] = { 0 };
	if (strcmp(lastLabel, label) != 0)
	{
		strcpy(lastLabel, label);
		lastPercent = -1;
	}

	if (percent <= lastPercent)
		return;

	lastPercent = percent;

	std::cout << "\r" << label << " [";
	for (int i = 0; i < barWidth; ++i)
		std::cout << (i < filled ? '#' : '-');
	std::cout << "] " << std::setw(3) << percent << "%" << std::flush;
	if (current >= total)
		std::cout << std::endl;
}

static uint16 FloatToHalfBits(float v)
{
	uint32 x = 0;
	std::memcpy(&x, &v, sizeof(float));
	uint32 sign = (x >> 16) & 0x8000;
	uint32 mantissa = x & 0x007fffff;
	int exp = ((int)(x >> 23) & 0xff) - 127 + 15;

	if (exp <= 0)
	{
		if (exp < -10)
			return (uint16)sign;

		mantissa = (mantissa | 0x00800000) >> (1 - exp);
		if (mantissa & 0x00001000)
			mantissa += 0x00002000;
		return (uint16)(sign | (mantissa >> 13));
	}
	else if (exp >= 31)
	{
		return (uint16)(sign | 0x7c00);
	}

	if (mantissa & 0x00001000)
	{
		mantissa += 0x00002000;
		if (mantissa & 0x00800000)
		{
			mantissa = 0;
			exp += 1;
			if (exp >= 31)
				return (uint16)(sign | 0x7c00);
		}
	}

	return (uint16)(sign | ((uint32)exp << 10) | (mantissa >> 13));
}

static void BuildOrthonormalBasis(const FVector& n, FVector& t, FVector& b)
{
	FVector up = (FMath::Abs(n.z) < 0.999f) ? FVector(0.0f, 0.0f, 1.0f) : FVector(0.0f, 1.0f, 0.0f);
	t = FVector::Cross(up, n);
	float tLen = t.Magnitude();
	if (tLen > 0.0f)
		t /= tLen;
	b = FVector::Cross(n, t);
}

static FVector SampleHemisphereCosine(const FVector& normal)
{
	float r1 = Rand01();
	float r2 = Rand01();
	float phi = 2.0f * (float)FMath::Pi() * r1;
	float r = FMath::Sqrt(r2);
	float x = r * FMath::Cos(phi);
	float y = r * FMath::Sin(phi);
	float z = FMath::Sqrt(FMath::Max(0.0f, 1.0f - r2));

	FVector t, b;
	BuildOrthonormalBasis(normal, t, b);
	return (t * x + b * y + normal * z).Normalize();
}

static bool Barycentric2D(const FVector2& p, const FVector2& a, const FVector2& b, const FVector2& c, float& w0, float& w1, float& w2)
{
	FVector2 v0 = b - a;
	FVector2 v1 = c - a;
	FVector2 v2 = p - a;

	float denom = Cross2D(v0, v1);
	if (FMath::Abs(denom) < 1e-8f)
		return false;

	w1 = Cross2D(v2, v1) / denom;
	w2 = Cross2D(v0, v2) / denom;
	w0 = 1.0f - w1 - w2;
	return (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f);
}

static void BleedAtlas(FLightmapAtlas& atlas, TArray<uint8>& coverage, uint32 bleedPixels)
{
	if (bleedPixels == 0 || atlas.data.Size() == 0)
		return;

	const uint32 width = atlas.width;
	const uint32 height = atlas.height;

	TArray<FVector> srcData = atlas.data;
	TArray<uint8> srcCoverage = coverage;
	TArray<FVector> dstData;
	TArray<uint8> dstCoverage;
	dstData.Resize(srcData.Size());
	dstCoverage.Resize(srcCoverage.Size());

	for (uint32 step = 0; step < bleedPixels; ++step)
	{
		bool anyFilled = false;
		std::memcpy(dstData.Data(), srcData.Data(), srcData.Size() * sizeof(FVector));
		std::memcpy(dstCoverage.Data(), srcCoverage.Data(), srcCoverage.Size() * sizeof(uint8));

		for (uint32 y = 0; y < height; ++y)
		{
			for (uint32 x = 0; x < width; ++x)
			{
				SizeType idx = (SizeType)y * width + x;
				if (srcCoverage[idx] != 0)
					continue;

				FVector sum(0.0f);
				uint32 count = 0;
				for (int32 oy = -1; oy <= 1; ++oy)
				{
					int32 ny = (int32)y + oy;
					if (ny < 0 || ny >= (int32)height)
						continue;
					for (int32 ox = -1; ox <= 1; ++ox)
					{
						int32 nx = (int32)x + ox;
						if (nx < 0 || nx >= (int32)width)
							continue;
						if (ox == 0 && oy == 0)
							continue;

						SizeType nidx = (SizeType)ny * width + (SizeType)nx;
						if (srcCoverage[nidx] == 0)
							continue;

						sum += srcData[nidx];
						++count;
					}
				}

				if (count > 0)
				{
					dstData[idx] = sum / (float)count;
					dstCoverage[idx] = 1;
					anyFilled = true;
				}
			}
		}

		if (!anyFilled)
			break;

		srcData = dstData;
		srcCoverage = dstCoverage;
	}

	atlas.data = srcData;
	coverage = srcCoverage;
}

static bool BakeModeAllowsDirect(ELightBakeMode mode)
{
	return mode == LIGHT_BAKE_DIRECT || mode == LIGHT_BAKE_ALL;
}

static bool BakeModeAllowsIndirect(ELightBakeMode mode)
{
	return mode == LIGHT_BAKE_INDIRECT || mode == LIGHT_BAKE_ALL;
}

static FVector ComputeDirectLighting(const FVector& position, const FVector& normal, const TArray<FBakeLight>& lights, CWorld* scene, bool bIndirectBounce, const CPrimitiveProxy* ignoreProxy = nullptr)
{
	FVector sum(0.0f);
	for (auto& light : lights)
	{
		if (!light.bEnabled)
			continue;
		if (light.bakeMode == LIGHT_BAKE_NONE)
			continue;
		if (bIndirectBounce)
		{
			if (!BakeModeAllowsIndirect(light.bakeMode))
				continue;
		}
		else
		{
			if (!BakeModeAllowsDirect(light.bakeMode))
				continue;
		}

		FVector dir;
		float dist = 0.0f;
		float atten = 1.0f;

		if (light.type == CLightProxy::DIRECTIONAL_LIGHT)
		{
			dir = -light.direction;
			float len = dir.Magnitude();
			if (len > 0.0f)
				dir /= len;
			dist = 100000.0f;
		}
		else
		{
			dir = light.position - position;
			dist = dir.Magnitude();
			if (dist <= 0.0001f)
				continue;
			if (light.range > 0.0f && dist > light.range)
				continue;

			dir /= dist;
			atten = 1.0f / FMath::Max(dist * dist, 0.0001f);
			if (light.range > 0.0f)
				atten *= FMath::Saturate(1.0f - (dist / light.range));
		}

		float nDotL = FMath::Max(FVector::Dot(normal, dir), 0.0f);
		if (nDotL <= 0.0f)
			continue;

		if (light.bCastShadows && scene)
		{
			FPrimitiveHitInfo hit;
			FVector start = position + normal * SHADOW_BIAS;
			if (RayCastScene(scene, start, dir, &hit, dist - SHADOW_BIAS, ignoreProxy))
			{
				if (hit.distance > (SHADOW_BIAS * 2.0f))
					continue;
			}
		}

		FVector lightColor = light.color * (light.intensity * atten * nDotL);
		sum += lightColor;
	}

	return sum;
}

static FVector ComputeIndirectLighting(const FVector& position, const FVector& normal, const TArray<FBakeLight>& lights, CWorld* scene, const FVector& skyColor, uint32 samples, uint32 maxBounces)
{
	if (!scene || samples == 0 || maxBounces == 0)
		return FVector(0.0f);

	FVector sum(0.0f);
	for (uint32 s = 0; s < samples; ++s)
	{
		FVector rayPos = position + normal * SHADOW_BIAS;
		FVector rayNormal = normal;
		FVector throughput(1.0f);

		for (uint32 bounce = 0; bounce < maxBounces; ++bounce)
		{
			FVector rayDir = SampleHemisphereCosine(rayNormal);
			FPrimitiveHitInfo hit;
			if (!RayCastScene(scene, rayPos, rayDir, &hit, 0.0f))
			{
				sum += skyColor * throughput;
				break;
			}

			FVector direct = ComputeDirectLighting(hit.position, hit.normal, lights, scene, true, nullptr);
			sum += direct * throughput;

			throughput *= 0.5f;
			rayPos = hit.position + hit.normal * SHADOW_BIAS;
			rayNormal = hit.normal;
		}
	}

	return sum / (float)samples;
}

CLightBaker::CLightBaker(CWorld* w) : world(w)
{
}

void CLightBaker::BakeAll(const FLightmapSettings& s)
{
	settings = s;
	result.bSuccess = false;
	BakeLightmaps();
	BakeLightProbes();
}

void CLightBaker::BakeLightmaps()
{
	result.lightmaps.Clear();
	result.meshRefs.Clear();

	if (!world)
		return;

	std::cout << "[LightBaker] Gathering lights and meshes" << std::endl;

	//CRenderScene* renderScene = world->GetRenderScene();
	const TArray<CLightProxy*>& worldLights = world->GetLights();
	TArray<FBakeLight> lights;
	for (auto* light : worldLights)
	{
		if (!light)
			continue;

		light->FetchData();

		if (light->bakingMode == LIGHT_BAKE_NONE)
			continue;

		FBakeLight bakeLight;
		bakeLight.type = light->type;
		bakeLight.bakeMode = light->bakingMode;
		bakeLight.position = light->position;
		bakeLight.direction = light->direction;
		bakeLight.color = light->color;
		bakeLight.range = light->range;
		bakeLight.intensity = light->intensity;
		bakeLight.bCastShadows = light->CastShadows();
		bakeLight.bEnabled = light->Enabled();
		lights.Add(bakeLight);
	}

	if (lights.Size() == 0)
	{
		std::cout << "[LightBaker] No bakeable lights found in scene" << std::endl;
		return;
	}

	TArray<FBakeMeshTask> tasks;
	TArray<CModelAsset*> modelsToClear;

	const TArray<CPrimitiveProxy*>& primitives = world->GetPrimitives();
	for (auto* prim : primitives)
	{
		if (!prim)
			continue;

		CModelComponent* modelComp = Cast<CModelComponent>(prim->GetOwner());
		if (!modelComp)
			continue;
		if (modelComp->GetEntity()->GetType() != ENTITY_STATIC || (modelComp->renderLayer & R_LAYER_LIGHTMAP) == 0)
			continue;

		TObjectPtr<CModelAsset> model = modelComp->GetModel();
		if (!model)
			continue;

		model->LoadMeshData();
		if (modelsToClear.Find(model) == modelsToClear.end())
			modelsToClear.Add(model);

		const TArray<FMesh>& meshes = model->GetMeshes();
		FVector position = modelComp->GetWorldPosition();
		FQuaternion rotation = modelComp->GetWorldRotation();
		FVector scale = modelComp->GetWorldScale();
		FMatrix transform = (FMatrix(1.0f).Translate(position) * rotation).Scale(scale);

		for (SizeType meshIndex = 0; meshIndex < meshes.Size(); ++meshIndex)
		{
			const FMesh& mesh = meshes[meshIndex];
			if (mesh.bSkinnedMesh)
				continue;
			if (!mesh.vertexData || !mesh.indexData || mesh.topologyType != FMesh::TOPOLOGY_TRIANGLES)
				continue;

			FVector size = mesh.bounds.Size();
			float sx = FMath::Abs(size.x * scale.x);
			float sy = FMath::Abs(size.y * scale.y);
			float sz = FMath::Abs(size.z * scale.z);
			float area = 2.0f * (sx * sy + sy * sz + sx * sz);
			float linear = FMath::Sqrt(FMath::Max(area, 0.0001f)) * settings.lightmapDensity;
			float minRect = (float)(ATLAS_PADDING * 2 + 8);
			float maxRect = (float)(settings.lightmapSize - ATLAS_PADDING * 2);
			float rectSizeF = FMath::Clamp(FMath::Ceil(linear), minRect, maxRect);

			FBakeMeshTask task;
			task.modelComp = modelComp;
			task.model = model;
			task.mesh = &mesh;
			task.meshIndex = (uint32)meshIndex;
			task.transform = transform;
			task.rotation = rotation;
			task.scale = scale;
			task.rectW = (uint32)rectSizeF;
			task.rectH = (uint32)rectSizeF;
			tasks.Add(task);
		}
	}

	if (tasks.Size() == 0)
	{
		std::cout << "[LightBaker] No bakeable meshes found in scene" << std::endl;
		for (auto* mdl : modelsToClear)
			mdl->ClearMeshData();
		return;
	}

	std::cout << "[LightBaker] Bake tasks: " << tasks.Size() << std::endl;

	TArray<int> remaining;
	remaining.Resize(tasks.Size());
	for (SizeType i = 0; i < tasks.Size(); ++i)
		remaining[i] = (int)i;

	while (remaining.Size() > 0 && result.lightmaps.Size() < settings.maxLightmapCount)
	{
		CreateAtlas();
		int32 atlasIndex = (int32)(result.lightmaps.Size() - 1);
		FLightmapAtlas& atlas = result.lightmaps[atlasIndex];
		std::cout << "[LightBaker] Packing atlas " << atlasIndex << " (" << atlas.width << "x" << atlas.height << ")" << std::endl;

		TArray<stbrp_node> nodes;
		nodes.Resize(atlas.width);

		stbrp_context context;
		stbrp_init_target(&context, (int)atlas.width, (int)atlas.height, nodes.Data(), (int)nodes.Size());
		stbrp_setup_allow_out_of_mem(&context, 1);

		TArray<stbrp_rect> rects;
		rects.Resize(remaining.Size());
		for (SizeType i = 0; i < remaining.Size(); ++i)
		{
			int taskIndex = remaining[i];
			stbrp_rect rect{};
			rect.id = taskIndex;
			rect.w = (int)tasks[taskIndex].rectW;
			rect.h = (int)tasks[taskIndex].rectH;
			rects[i] = rect;
		}

		stbrp_pack_rects(&context, rects.Data(), (int)rects.Size());

		bool anyPacked = false;
		TArray<int> newRemaining;
		for (SizeType i = 0; i < rects.Size(); ++i)
		{
			const stbrp_rect& rect = rects[i];
			if (!rect.was_packed)
			{
				newRemaining.Add(rect.id);
				continue;
			}

			anyPacked = true;
			FBakeMeshTask& task = tasks[rect.id];
			task.atlasIndex = atlasIndex;
			task.rectX = rect.x;
			task.rectY = rect.y;
			float usableW = (float)(rect.w - ATLAS_PADDING * 2);
			float usableH = (float)(rect.h - ATLAS_PADDING * 2);
			usableW = FMath::Max(usableW, 1.0f);
			usableH = FMath::Max(usableH, 1.0f);
			task.lightmapPos = FVector2(
				(float)(rect.x + (int)ATLAS_PADDING) / (float)atlas.width,
				(float)(rect.y + (int)ATLAS_PADDING) / (float)atlas.height);
			task.lightmapScale = FVector2(
				usableW / (float)atlas.width,
				usableH / (float)atlas.height);
		}

		if (!anyPacked)
		{
			CONSOLE_LogWarning("LightBaker", "Atlas packing failed. Rectangles are too large for the atlas size.");
			break;
		}

		remaining = newRemaining;
	}

	if (remaining.Size() > 0)
		CONSOLE_LogWarning("LightBaker", "Not all meshes could be packed. Increase maxLightmapCount or atlas size.");

	SizeType totalVerts = 0;
	for (auto& task : tasks)
		if (task.atlasIndex >= 0)
			totalVerts += task.mesh ? task.mesh->numIndexData : 0;

	SizeType bakedVerts = 0;

	SizeType degenerateUv2Count = 0;
	SizeType degenerateUv1Count = 0;
	std::atomic<SizeType> bakedVertsAtomic{ 0 };
	std::atomic<SizeType> degenerateUv2CountAtomic{ 0 };
	std::atomic<SizeType> degenerateUv1CountAtomic{ 0 };

	TArray<SizeType> taskIndices;
	taskIndices.Reserve(tasks.Size());
	for (SizeType i = 0; i < tasks.Size(); ++i)
	{
		if (tasks[i].atlasIndex >= 0)
			taskIndices.Add(i);
	}

	TArray<TArray<uint8>> atlasCoverage;
	atlasCoverage.Resize(result.lightmaps.Size());
	for (SizeType i = 0; i < result.lightmaps.Size(); ++i)
	{
		FLightmapAtlas& atlas = result.lightmaps[i];
		atlasCoverage[i].Resize((SizeType)atlas.width * atlas.height);
		std::memset(atlasCoverage[i].Data(), 0, atlasCoverage[i].Size() * sizeof(uint8));
	}

	std::mutex progressMutex;
	const SizeType reportStep = std::max<SizeType>(totalVerts / 100, 1);
	std::atomic<SizeType> nextReport{ reportStep };

	auto ReportBakeProgress = [&](SizeType current)
	{
		if (totalVerts == 0)
			return;

		SizeType target = nextReport.load(std::memory_order_relaxed);
		while (current >= target)
		{
			if (nextReport.compare_exchange_weak(target, target + reportStep, std::memory_order_relaxed))
			{
				std::lock_guard<std::mutex> lock(progressMutex);
				PrintProgress("[LightBaker] Baking", current, totalVerts);
				break;
			}
		}
	};

	auto BakeTask = [&](FBakeMeshTask& task)
	{
		FLightmapAtlas& atlas = result.lightmaps[task.atlasIndex];
		TArray<uint8>& atlasCover = atlasCoverage[task.atlasIndex];
		TArray<uint8> coverage;
		coverage.Resize(atlas.width * atlas.height);
		std::memset(coverage.Data(), 0, coverage.Size() * sizeof(uint8));

		const FVertex* verts = task.mesh->vertexData;
		const uint* indices = task.mesh->indexData;
		uint32 faceCount = task.mesh->numIndexData / 3;

		FVector invScale(
			task.scale.x != 0.0f ? 1.0f / task.scale.x : 0.0f,
			task.scale.y != 0.0f ? 1.0f / task.scale.y : 0.0f,
			task.scale.z != 0.0f ? 1.0f / task.scale.z : 0.0f);

		const float invW = 1.0f / (float)atlas.width;
		const float invH = 1.0f / (float)atlas.height;

		for (uint32 face = 0; face < faceCount; ++face)
		{
			uint i0 = indices[face * 3];
			uint i1 = indices[face * 3 + 1];
			uint i2 = indices[face * 3 + 2];
			if (i0 >= task.mesh->numVertexData || i1 >= task.mesh->numVertexData || i2 >= task.mesh->numVertexData)
				continue;

			SizeType progress = bakedVertsAtomic.fetch_add(3, std::memory_order_relaxed) + 3;
			ReportBakeProgress(progress);

			FVector2 uv0(verts[i0].uv2[0], verts[i0].uv2[1]);
			FVector2 uv1(verts[i1].uv2[0], verts[i1].uv2[1]);
			FVector2 uv2(verts[i2].uv2[0], verts[i2].uv2[1]);

			if (TriangleArea2D(uv0, uv1, uv2) < 1e-8f)
			{
				FVector2 uv0Alt(verts[i0].uv1[0], verts[i0].uv1[1]);
				FVector2 uv1Alt(verts[i1].uv1[0], verts[i1].uv1[1]);
				FVector2 uv2Alt(verts[i2].uv1[0], verts[i2].uv1[1]);
				if (TriangleArea2D(uv0Alt, uv1Alt, uv2Alt) >= 1e-8f)
				{
					uv0 = uv0Alt;
					uv1 = uv1Alt;
					uv2 = uv2Alt;
					degenerateUv2CountAtomic.fetch_add(1, std::memory_order_relaxed);
				}
				else
				{
					degenerateUv1CountAtomic.fetch_add(1, std::memory_order_relaxed);
					continue;
				}
			}

			if (uv0.x < 0.0f || uv0.x > 1.0f || uv0.y < 0.0f || uv0.y > 1.0f)
				continue;
			if (uv1.x < 0.0f || uv1.x > 1.0f || uv1.y < 0.0f || uv1.y > 1.0f)
				continue;
			if (uv2.x < 0.0f || uv2.x > 1.0f || uv2.y < 0.0f || uv2.y > 1.0f)
				continue;

			FVector2 a0 = uv0 * task.lightmapScale + task.lightmapPos;
			FVector2 a1 = uv1 * task.lightmapScale + task.lightmapPos;
			FVector2 a2 = uv2 * task.lightmapScale + task.lightmapPos;

			float minU = FMath::Min(a0.x, FMath::Min(a1.x, a2.x));
			float maxU = FMath::Max(a0.x, FMath::Max(a1.x, a2.x));
			float minV = FMath::Min(a0.y, FMath::Min(a1.y, a2.y));
			float maxV = FMath::Max(a0.y, FMath::Max(a1.y, a2.y));

			int x0 = (int)FMath::Clamp(FMath::Floor(minU * atlas.width), 0.0f, (float)(atlas.width - 1));
			int x1 = (int)FMath::Clamp(FMath::Ceil(maxU * atlas.width), 0.0f, (float)(atlas.width - 1));
			int y0 = (int)FMath::Clamp(FMath::Floor(minV * atlas.height), 0.0f, (float)(atlas.height - 1));
			int y1 = (int)FMath::Clamp(FMath::Ceil(maxV * atlas.height), 0.0f, (float)(atlas.height - 1));

			FVector p0 = verts[i0].position * task.transform;
			FVector p1 = verts[i1].position * task.transform;
			FVector p2 = verts[i2].position * task.transform;

			FVector n0 = task.rotation.Rotate(verts[i0].normal * invScale);
			FVector n1 = task.rotation.Rotate(verts[i1].normal * invScale);
			FVector n2 = task.rotation.Rotate(verts[i2].normal * invScale);

			for (int y = y0; y <= y1; ++y)
			{
				for (int x = x0; x <= x1; ++x)
				{
					SizeType idx = (SizeType)y * atlas.width + (SizeType)x;
					if (coverage[idx] != 0)
						continue;

					FVector2 p((x + 0.5f) * invW, (y + 0.5f) * invH);
					float w0, w1, w2;
					if (!Barycentric2D(p, a0, a1, a2, w0, w1, w2))
						continue;

					FVector pos = p0 * w0 + p1 * w1 + p2 * w2;
					FVector normal = n0 * w0 + n1 * w1 + n2 * w2;
					float nLen = normal.Magnitude();
					if (nLen > 0.0f)
						normal /= nLen;

					FVector direct = ComputeDirectLighting(pos, normal, lights, world, false, task.modelComp->PrimitiveProxy());
					FVector indirect = ComputeIndirectLighting(pos, normal, lights, world, settings.skyColor, settings.indirectSamples, settings.maxBounces);
					atlas.data[idx] = direct + indirect;
					coverage[idx] = 1;
					atlasCover[idx] = 1;
				}
			}
		}
	};

	SizeType taskCount = taskIndices.Size();
	uint32 hwThreads = std::thread::hardware_concurrency();
	uint32 workerCount = hwThreads > 0 ? hwThreads : 4;
	workerCount = (uint32)std::min<SizeType>((SizeType)workerCount, taskCount);
	if (workerCount == 0)
		workerCount = 1;

	std::atomic<SizeType> nextTask{ 0 };
	std::vector<std::thread> workers;
	if (workerCount > 1)
		workers.reserve(workerCount - 1);

	auto WorkerFn = [&]()
	{
		for (;;)
		{
			SizeType idx = nextTask.fetch_add(1, std::memory_order_relaxed);
			if (idx >= taskCount)
				break;
			FBakeMeshTask& task = tasks[taskIndices[idx]];
			BakeTask(task);
		}
	};

	for (uint32 t = 1; t < workerCount; ++t)
		workers.emplace_back(WorkerFn);

	WorkerFn();
	for (auto& worker : workers)
		worker.join();

	bakedVerts = bakedVertsAtomic.load(std::memory_order_relaxed);
	degenerateUv2Count = degenerateUv2CountAtomic.load(std::memory_order_relaxed);
	degenerateUv1Count = degenerateUv1CountAtomic.load(std::memory_order_relaxed);

	if (bakedVerts >= totalVerts)
		PrintProgress("[LightBaker] Baking", bakedVerts, totalVerts);

	uint32 bleedPixels = (uint32)FMath::Min((int32)LIGHTMAP_BLEED_PIXELS, (int32)ATLAS_PADDING);
	for (SizeType i = 0; i < result.lightmaps.Size(); ++i)
		BleedAtlas(result.lightmaps[i], atlasCoverage[i], bleedPixels);

	for (auto& task : tasks)
	{
		if (task.atlasIndex < 0)
			continue;

		FLightmapMeshRef ref;
		TObjectPtr<CEntity> ent = task.modelComp->GetEntity();
		if (!ent)
			continue;
		ref.entityId = ent->EntityId();
		ref.componentId = task.modelComp->ComponentId();
		ref.meshIndex = task.meshIndex;
		ref.lightmapId = task.atlasIndex;
		ref.lightmapPos = task.lightmapPos;
		ref.lightmapScale = task.lightmapScale;
		result.meshRefs.Add(ref);
	}

	for (auto* mdl : modelsToClear)
		mdl->ClearMeshData();

	result.bSuccess = true;
}

void CLightBaker::BakeLightProbes()
{
	// TODO: Implement probe baking. 
}

void CLightBaker::SaveData()
{
	if (!world)
		return;

	CScene* scene = world->GetScene();
	FString scenePath = scene ? scene->GetPath() : FString();
	FFile* sceneFile = scene->File();
	FString outputPath = sceneFile->Mod()->Path() + "/" + sceneFile->Dir()->GetPath() + "/" + sceneFile->Name() + "_lightdata.bin";

	CFStream stream(outputPath, "wb");
	if (!stream.IsOpen())
	{
		CONSOLE_LogError("LightBaker", "Failed to open output file for light data: " + outputPath);
		return;
	}

	std::cout << "[LightBaker] Writing light data: " << outputPath.c_str() << std::endl;

	FLightDataHeader header;
	header.atlasCount = (uint32)result.lightmaps.Size();
	header.meshCount = (uint32)result.meshRefs.Size();
	stream << &header;

	for (auto& ref : result.meshRefs)
	{
		stream << &ref.entityId;
		stream << &ref.componentId;
		stream << &ref.meshIndex;
		stream << &ref.lightmapId;
		stream << &ref.lightmapPos;
		stream << &ref.lightmapScale;
	}

	SizeType atlasIndex = 0;
	SizeType atlasCount = result.lightmaps.Size();
	for (auto& atlas : result.lightmaps)
	{
		stream << &atlas.width;
		stream << &atlas.height;
		uint32 texelCount = atlas.width * atlas.height;
		stream << &texelCount;
		if (texelCount > 0)
		{
			TArray<uint16> packed;
			packed.Resize((SizeType)texelCount * 3);
			for (SizeType i = 0; i < texelCount; ++i)
			{
				const FVector& c = atlas.data[i];
				SizeType base = i * 3;
				packed[base + 0] = FloatToHalfBits(c.x);
				packed[base + 1] = FloatToHalfBits(c.y);
				packed[base + 2] = FloatToHalfBits(c.z);
			}
			stream.Write(packed.Data(), packed.Size() * sizeof(uint16));
		}
		atlasIndex++;
		PrintProgress("[LightBaker] Writing", atlasIndex, atlasCount);
	}
}

void CLightBaker::CreateAtlas()
{
	FLightmapAtlas atlas;
	atlas.width = settings.lightmapSize;
	atlas.height = settings.lightmapSize;
	atlas.data.Resize(atlas.width * atlas.height);

	for (SizeType i = 0; i < atlas.data.Size(); ++i)
		atlas.data[i] = FVector(0.0f);

	result.lightmaps.Add(atlas);
}
