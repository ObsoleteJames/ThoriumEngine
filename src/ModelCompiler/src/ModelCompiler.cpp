
#include "ModelCompiler.h"
#include "Assets/Animation.h"
#include "Assets/Material.h"
#include "Rendering/GraphicsInterface.h"
#include "Object/Variant.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

#include <Util/KeyValue.h>

void LoadMeshFile(FMeshFile& m)
{
	unsigned int flags = aiProcess_Triangulate | aiProcess_MakeLeftHanded | aiProcess_OptimizeMeshes | aiProcess_FlipUVs | aiProcess_PopulateArmatureData;

	//if (!m.bImportTangents)
	flags |= aiProcess_CalcTangentSpace;

	if (!m.importer)
		m.importer = new Assimp::Importer();

	m.scene = m.importer->ReadFile(m.file.c_str(), flags);
	if (!m.scene)
	{
		m.bLoadFailed = true;
		return;
	}

	m.bLoadFailed = false;

	m.name = m.file;

	if (auto i = m.name.FindLastOf("/\\"); i != -1)
		m.name.Erase(m.name.begin(), m.name.begin() + i + 1);
}

aiMatrix4x4 GetNodeWorldTransform(aiNode* node)
{
	if (node->mParent)
		return node->mTransformation * GetNodeWorldTransform(node->mParent);

	return node->mTransformation;
}

bool CModelCompiler::Compile(CModelAsset* _mdl, FMeshFile* meshFiles, int numMeshFiles, const FModelCompileSettings& settings)
{
	SetModel(_mdl);

	TArray<FMaterial> oldMats = mdl->materials;

	mdl->ClearMeshData();
	mdl->meshes.Clear();
	//mdl->ClearMeshes();
	mdl->materials.Clear();
	mdl->skeleton.bones.Clear();

	SizeType meshesOffset = 0;
	SizeType materialsOffset = 0;
	SizeType boneOffset = 0;

	for (int i = 0; i < numMeshFiles; i++)
	{
		auto& file = meshFiles[i];

		if (!file.scene)
			LoadMeshFile(file);

		if (file.scene)
		{
			auto* scene = file.scene;
			auto* root = scene->mRootNode;

			for (uint i = 0; i < scene->mNumMaterials; i++)
			{
				aiMaterial* sMat = scene->mMaterials[i];

				FMaterial mat;
				mat.name = sMat->GetName().C_Str();

				if (settings.bCreateMaterials)
				{
					if (mat.name.IsEmpty())
						mat.name = "Material_" + FString::ToString(i);

					FString matPath = mdl->File()->Path() + "/";
					if (!settings.materialsOut.IsEmpty())
						matPath += settings.materialsOut + "/";
					matPath += mat.name + ".thasset";

					if (!CAssetManager::GetAssetData(matPath))
					{
						auto matObj = CAssetManager::CreateAsset<CMaterial>(matPath, mdl->File()->Mod()->Name());


					}
					else
						mat.path = matPath;
				}

				mdl->materials.Add(mat);
			}

			/*for (uint i = 0; i < scene->mNumSkeletons; i++)
			{
				aiSkeleton* skeleton = scene->mSkeletons[i];

				for (int ii = 0; ii < skeleton->mNumBones; ii++)
				{
					aiSkeletonBone* bone = skeleton->mBones[ii];
					aiMatrix4x4& mat = bone->mLocalMatrix;

					aiVector3D scale;
					aiVector3D pos;
					aiQuaternion rot;

					mat.Decompose(scale, rot, pos);

					FBone b;
					b.name = bone->mNode ? bone->mNode->mName.C_Str() : "BONE";
					b.position = { pos.x, pos.y, pos.z };
					b.rotation = { rot.x, rot.y, rot.z, rot.w };

					b.parent = bone->mParent == -1 ? -1 : bone->mParent + (int)boneOffset;

					mdl->skeleton.bones.Add(b);
				}
			}*/

			TArray<TPair<int, aiBone*>> bones;

			CompileNode(file, scene, root, meshesOffset, materialsOffset, bones);

			TMap<aiNode*, int> boneLUT;

			for (auto& b : bones)
			{
				FBone* newBone = nullptr;
				int bIndex = boneLUT[b.Value->mNode];
				if (bIndex == 0)
				{
					mdl->skeleton.bones.Add();

					bIndex = mdl->skeleton.bones.Size();
					boneLUT[b.Value->mNode] = bIndex;
				}
				newBone = &mdl->skeleton.bones[bIndex - 1];
				newBone->name = b.Value->mName.C_Str();

				aiVector3D scale;
				aiVector3D pos;
				aiQuaternion rot;

				b.Value->mNode->mTransformation.Decompose(scale, rot, pos);
				newBone->position = { pos.x, pos.y, pos.z };
				newBone->rotation = { rot.x, rot.y, rot.z, rot.w };

				auto& mesh = mdl->meshes[b.Key];

				for (int i = 0; i < b.Value->mNumWeights; i++)
				{
					if (!mesh.vertexData)
						continue;

					auto& weight = b.Value->mWeights[i];
					if (weight.mVertexId >= mesh.numVertexData)
						continue;

					FSkinnedVertex& vertex = ((FSkinnedVertex*)mesh.vertexData)[weight.mVertexId];

					for (int x = 0; x < 4; x++)
					{
						if (vertex.bones[x] == -1)
						{
							vertex.bones[x] = bIndex - 1;
							vertex.boneInfluence[x] = weight.mWeight;
							break;
						}
					}
				}
			}

			// Resolve bone parents
			for (int i = 0; i < mdl->skeleton.bones.Size(); i++)
			{
				if (i >= bones.Size())
					continue;

				aiNode* parent = bones[i].Value->mNode->mParent;
				if (parent)
					mdl->skeleton.bones[i].parent = mdl->GetBoneIndex(parent->mName.C_Str());
			}

			for (int i = 0; i < mdl->meshes.Size(); i++)
			{
				if (mdl->meshNames[i].IsEmpty())
					mdl->meshNames[i] = "Mesh " + FString::ToString(i);

				FBufferDescriptor desc{};
				desc.type = TH_BUFFER_TYPE_VERTEX_BUFFER;
				desc.data = mdl->meshes[i].vertexData;
				desc.bufferSize = (uint32)(mdl->meshes[i].numVertexData * (mdl->meshes[i].bSkinnedMesh ? sizeof(FSkinnedVertex) : sizeof(FVertex)));
				desc.dataStride = (uint32)(mdl->meshes[i].bSkinnedMesh ? sizeof(FSkinnedVertex) : sizeof(FVertex));
				mdl->meshes[i].vertexBuffer = gGHI->CreateBuffer(desc);

				desc.type = TH_BUFFER_TYPE_INDEX_BUFFER;
				desc.data = mdl->meshes[i].indexData;
				desc.bufferSize = (uint32)(mdl->meshes[i].numIndexData * sizeof(uint));
				desc.dataStride = sizeof(uint);
				mdl->meshes[i].indexBuffer = gGHI->CreateBuffer(desc);
			}

			mdl->_SetLod(0);

			materialsOffset = mdl->materials.Size();
			meshesOffset = mdl->meshes.Size();
			boneOffset = mdl->skeleton.bones.Size();
		}
		else
		{
			error = file.importer->GetErrorString();
			return false;
		}
	}

	for (auto& mat : mdl->materials)
	{
		for (auto& m : oldMats)
		{
			if (mat.name == m.name)
			{
				mat.obj = m.obj;
				mat.path = m.path;
				break;
			}
		}
	}

	mdl->CalculateBounds();
	mdl->UpdateBoneMatrices();
	return true;
}

bool CModelCompiler::CompileFromCfgFile(CModelAsset* _mdl, const FString& file, bool bFullRecompile)
{
	SetModel(_mdl);

	FKeyValue cfg(file);
	if (!cfg.IsOpen())
	{
		error = "Failed to open config file '" + file + "'";
		return false;
	}

	auto* cmeshes = cfg.GetCategory("meshes");
	if (!cmeshes || cmeshes->GetCategories().Size() == 0)
	{
		error = "Config file has no meshes listed";
		return false;
	}

	TArray<FMeshFile> meshes;
	meshes.Reserve(cmeshes->GetCategories().Size());
	
	for (auto* m : cmeshes->GetCategories())
	{
		FMeshFile data{};
		data.file = *m->GetValue("file");
		data.name = m->GetName();

		data.transform.position = FVariant::FromString(*m->GetValue("position")).AsVector();
		data.transform.scale = FVariant::FromString(*m->GetValue("scale")).AsVector();
		data.transform.rotation = FQuaternion::EulerAngles(FVariant::FromString(*m->GetValue("rotation")).AsVector());

		if (data.transform.scale == FVector::zero)
			data.transform.scale = FVector::one;

		meshes.Add(data);
	}

	if (!Compile(mdl, meshes.Data(), meshes.Size()))
		return false;

	if (!bFullRecompile)
		return true;

	// Compile bodygroups, lodgroups, and materials from the cfg file
	auto* bodyGroups = cfg.GetCategory("bodygroups");
	if (bodyGroups)
	{
		mdl->bodyGroups.Clear();
		for (auto bodyGroup : bodyGroups->GetCategories())
		{
			FBodyGroup bg{};
			bg.name = bodyGroup->GetName();

			for (auto& option : bodyGroup->GetArrays())
			{
				FBodyGroupOption o{};
				o.name = option.Key;

				for (auto& m : option.Value)
					if (auto i = GetMeshIndex(m); i != -1)
						o.meshIndices.Add(i);
			}
		}
	}

	auto* lods = cfg.GetCategory("lods");
	if (lods)
	{
		mdl->numLODs = 0;
	
		int lastLod = -1;
		for (int i = 0; i < 6; i++)
		{
			auto* lod = lods->GetArray("lod" + FString::ToString(i));
			if (!lod)
				continue;

			if (lastLod != i - 1)
			{
				error = "Invalid LODs, missing lod" + FString::ToString(i - 1);
				return false;
			}
			lastLod = i;
			mdl->numLODs++;

			for (auto& l : *lod)
				if (auto m = GetMeshIndex(l); m != -1)
					mdl->LODs[i].meshIndices.Add(m);

			mdl->LODs[i].distanceBias = lods->GetValue("lod" + FString::ToString(i) + "_distance")->AsFloat();
		}
	}

	auto* mats = cfg.GetCategory("materials");
	if (mats)
	{
		FString _default = *mats->GetValue("_default");
		for (auto& m : mats->GetValues())
		{
			if (auto* mat = GetMaterial(m.Key); mat != nullptr)
				mat->path = m.Value;
		}

		// default material for any that doesn't have a name set.
		for (auto& m : mdl->GetMaterials())
			if (m.name.IsEmpty() || m.path.IsEmpty())
				m.path = _default;
	}
	
	// TODO: add colliders

	return true;
}

void CModelCompiler::CompileNode(FMeshFile& file, const aiScene* scene, aiNode* node, SizeType& meshOffset, SizeType& matOffset, TArray<TPair<int, aiBone*>>& outBones)
{
	for (uint i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		aiMatrix4x4 transform = GetNodeWorldTransform(node);

		FMatrix mat = file.transform.ToMatrix();
		transform = (*(aiMatrix4x4*)&mat) * transform;

		FMesh fmesh;
		//fmesh.meshName = mesh->mName.C_Str();
		fmesh.materialIndex = mesh->mMaterialIndex + (int)matOffset;

		if (mesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE)
			fmesh.topologyType = FMesh::TOPOLOGY_TRIANGLES;
		else if (mesh->mPrimitiveTypes == aiPrimitiveType_LINE)
			fmesh.topologyType = FMesh::TOPOLOGY_LINES;
		else if (mesh->mPrimitiveTypes == aiPrimitiveType_POINT)
			fmesh.topologyType = FMesh::TOPOLOGY_POINTS;

		TArray<uint> indices;
		fmesh.bSkinnedMesh = mesh->mNumBones > 0;
		if (mesh->mNumBones > 0)
		{
			TArray<FSkinnedVertex> vertices;

			vertices.Reserve(mesh->mNumVertices);
			indices.Reserve(mesh->mNumFaces * 3);

			for (uint i = 0; i < mesh->mNumVertices; i++)
			{
				FSkinnedVertex v;
				v.bones[0] = -1;
				v.bones[1] = -1;
				v.bones[2] = -1;
				v.bones[3] = -1;
				v.boneInfluence[0] = 0.f;
				v.boneInfluence[1] = 0.f;
				v.boneInfluence[2] = 0.f;
				v.boneInfluence[3] = 0.f;

				auto vPos = mesh->mVertices[i];
				auto vNormal = mesh->mNormals ? mesh->mNormals[i] : aiVector3D(0, 1, 0);
				auto vTangent = mesh->mTangents ? mesh->mTangents[i] : aiVector3D(1, 0, 0);

				vPos *= transform;
				vNormal *= transform;
				vTangent *= transform;

				v.position = *(FVector*)&vPos;
				if (mesh->mNormals)
					v.normal = *(FVector*)&vNormal;
				if (mesh->mTangents)
					v.tangent = *(FVector*)&vTangent;
				if (mesh->GetNumColorChannels() > 0)
					v.color = *(FVector*)&mesh->mColors[0][i];

				if (mesh->GetNumUVChannels() > 0)
				{
					v.uv1[0] = mesh->mTextureCoords[0][i].x;
					v.uv1[1] = mesh->mTextureCoords[0][i].y;
				}
				if (mesh->GetNumUVChannels() > 1)
				{
					v.uv2[0] = mesh->mTextureCoords[1][i].x;
					v.uv2[1] = mesh->mTextureCoords[1][i].y;
				}

				vertices.Add(v);
			}

			fmesh.vertexData = (FVertex*)malloc(vertices.Size() * sizeof(FSkinnedVertex));
			fmesh.numVertexData = vertices.Size();

			memcpy(fmesh.vertexData, vertices.Data(), vertices.Size() * sizeof(FSkinnedVertex));
		}
		else
		{
			TArray<FVertex> vertices;

			vertices.Reserve(mesh->mNumVertices);
			indices.Reserve(mesh->mNumFaces * 3);

			for (uint i = 0; i < mesh->mNumVertices; i++)
			{
				FVertex v;

				auto vPos = mesh->mVertices[i];
				auto vNormal = mesh->mNormals ? mesh->mNormals[i] : aiVector3D(0, 1, 0);
				auto vTangent = mesh->mTangents ? mesh->mTangents[i] : aiVector3D(1, 0, 0);

				vPos *= transform;
				vNormal *= transform;
				vTangent *= transform;

				v.position = *(FVector*)&vPos;
				if (mesh->mNormals)
					v.normal = *(FVector*)&vNormal;
				if (mesh->mTangents)
					v.tangent = *(FVector*)&vTangent;
				if (mesh->GetNumColorChannels() > 0)
					v.color = *(FVector*)&mesh->mColors[0][i];

				if (mesh->GetNumUVChannels() > 0)
				{
					v.uv1[0] = mesh->mTextureCoords[0][i].x;
					v.uv1[1] = mesh->mTextureCoords[0][i].y;
				}
				if (mesh->GetNumUVChannels() > 1)
				{
					v.uv2[0] = mesh->mTextureCoords[1][i].x;
					v.uv2[1] = mesh->mTextureCoords[1][i].y;
				}

				vertices.Add(v);
			}

			fmesh.vertexData = (FVertex*)malloc(vertices.Size() * sizeof(FVertex));
			fmesh.numVertexData = vertices.Size();
			fmesh.numVertices = vertices.Size();

			memcpy(fmesh.vertexData, vertices.Data(), vertices.Size() * sizeof(FVertex));
		}

		for (uint i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace& face = mesh->mFaces[i];

			if (face.mNumIndices == 3)
			{
				indices.Add(face.mIndices[0]);
				indices.Add(face.mIndices[2]);
				indices.Add(face.mIndices[1]);
			}
			else if (face.mNumIndices == 2)
			{
				indices.Add(face.mIndices[0]);
				indices.Add(face.mIndices[1]);
			}
			else
				indices.Add(face.mIndices[0]);
		}

		for (uint i = 0; i < mesh->mNumBones; i++)
		{
			auto* b = mesh->mBones[i];

			outBones.Add({ (int)mdl->meshes.Size(), b });
		}

		fmesh.indexData = (uint*)malloc(indices.Size() * sizeof(uint));
		fmesh.numIndexData = indices.Size();

		memcpy(fmesh.indexData, indices.Data(), indices.Size() * sizeof(uint));

		fmesh.numIndices = indices.Size();

		fmesh.CalculateBounds();

		mdl->meshes.Add(fmesh);
		mdl->meshNames.Add(mesh->mName.C_Str());
	}

	for (int i = 0; i < node->mNumChildren; i++)
		CompileNode(file, scene, node->mChildren[i], meshOffset, matOffset, outBones);
}

SizeType CModelCompiler::GetMeshIndex(const FString& name)
{
	for (int i = 0; i < mdl->meshNames.Size(); i++)
	{
		if (mdl->meshNames[i] == name)
			return i;
	}

	return -1;
}

FMaterial* CModelCompiler::GetMaterial(const FString& name)
{
	for (auto& m : mdl->GetMaterials())
		if (m.name == name)
			return &m;

	return nullptr;
}

bool CModelCompiler::GenerateLODGroups(FString suffix /*= "_LOD"*/)
{
	// Meshes without suffix
	TArray<FMesh*> unkownMeshes;

	int meshesAdded = 0;
	int meshIndex = 0;
	for (auto& mesh : mdl->meshes)
	{
		const FString& meshName = mdl->meshNames[meshIndex];
		meshIndex++;
		//SizeType i = mesh.meshName.Find(suffix);
		SizeType i = meshName.Find(suffix);

		if (i == -1)
		{
			unkownMeshes.Add(&mesh);
			continue;
		}

		FString index = meshName;
		index.Erase(index.begin(), index.begin() + i + suffix.Size());

		int lodIndex = -1;
		try
		{
			lodIndex = FMath::Clamp(index.ToInt(), 0, 5);
		}
		catch (std::exception& e) {}

		if (lodIndex == -1)
		{
			unkownMeshes.Add(&mesh);
			continue;
		}

		mdl->numLODs = lodIndex + 1;
		mdl->LODs[lodIndex].meshIndices.Add(meshIndex - 1);
		meshesAdded++;
	}

	if (meshesAdded > 0)
	{
		FBounds bounds = mdl->GetBounds();
		for (int i = 0; i < mdl->numLODs; i++)
		{
			mdl->LODs[i].distanceBias = bounds.Size().Magnitude() * 2 * i;
		}
	}

	return meshesAdded > 0;
}

bool CModelCompiler::GenerateConvexCollision()
{
	return false;
}

void CModelCompiler::SaveModel(FMeshFile* meshFiles, int numMeshFiles)
{
	FKeyValue kv(mdl->File()->GetSdkPath(".meta"));
	for (int i = 0; i < numMeshFiles; i++)
	{
		auto* cat = kv.GetCategory(meshFiles[i].name + "_" + FString::ToString(i), true);

		cat->SetValue("file", meshFiles[i].file);

		FVector& pos = meshFiles[i].transform.position;
		FVector& rot = meshFiles[i].rotation;
		FVector& scl = meshFiles[i].transform.scale;

		cat->SetValue("position", (std::to_string(pos.x) + "," + std::to_string(pos.y) + "," + std::to_string(pos.z)).c_str());
		cat->SetValue("rotation", (std::to_string(rot.x) + "," + std::to_string(rot.y) + "," + std::to_string(rot.z)).c_str());
		cat->SetValue("scale", (std::to_string(scl.x) + "," + std::to_string(scl.y) + "," + std::to_string(scl.z)).c_str());
	}
	kv.Save();

	mdl->Save();
}

bool CModelCompiler::ExportAnimation(aiAnimation* anim, const FAnimationImportSettings& settings)
{
	bool bNew = false;
	TObjectPtr<CAnimation> out = CAssetManager::GetAsset<CAnimation>(settings.path + ".thasset");
	if (!out.IsValid())
	{
		bNew = true;
		out = CreateObject<CAnimation>();
	}

	out->ClearChannels();

	//out->SetFrameRate(anim->mTicksPerSecond);
	for (int i = 0; i < anim->mNumChannels; i++)
	{
		auto* channel = out->AddChannel(anim->mChannels[i]->mNodeName.C_Str());
		channel->behaviour = KEYFRAME_INTERP_LINEAR;
		channel->type = KEYFRAME_BONE;

		//int keyFrames = FMath::Max(FMath::Max(anim->mChannels[i]->mNumPositionKeys, anim->mChannels[i]->mNumRotationKeys), anim->mChannels[i]->mScalingKeys);

		TMap<float, FKeyframe> keyframes;

		for (int ii = 0; ii < anim->mChannels[i]->mNumPositionKeys; ii++)
		{
			auto& key = anim->mChannels[i]->mPositionKeys[ii];
			//channel->keyframes.last()->time = key.mTime;
			//channel->keyframes.last()->keyBone.position = *(FVector*)&key.mValue;

			keyframes[key.mTime / 33.3333f].keyBone.position = *(FVector*)&key.mValue;
		}
		for (int ii = 0; ii < anim->mChannels[i]->mNumRotationKeys; ii++)
		{
			auto& key = anim->mChannels[i]->mRotationKeys[ii];
			//channel->keyframes[ii].keyBone.rotation = *(FQuaternion*)&key.mValue;
			FQuaternion value = FQuaternion(key.mValue.x, key.mValue.y, key.mValue.z, key.mValue.w);
			keyframes[key.mTime / 33.3333f].keyBone.rotation = value;
		}
		for (int ii = 0; ii < anim->mChannels[i]->mNumScalingKeys; ii++)
		{
			auto& key = anim->mChannels[i]->mScalingKeys[ii];
			//channel->keyframes[ii].keyBone.scale = *(FVector*)&key.mValue;
			keyframes[key.mTime / 33.3333f].keyBone.scale = *(FVector*)&key.mValue;
		}

		for (auto& k : keyframes)
		{
			channel->keyframes.Add();
			auto& frame = channel->keyframes.last();
			frame->time = k.first;
			frame->keyBone = k.second.keyBone;
		}
	}

	if (bNew)
		CAssetManager::RegisterNewAsset(out, settings.path, settings.mod);

	out->Save();

	return true;
}
