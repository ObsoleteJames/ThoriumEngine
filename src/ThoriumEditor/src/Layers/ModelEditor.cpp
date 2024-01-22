
#include "ModelEditor.h"
#include "Game/World.h"
#include "Game/Entities/ModelEntity.h"
#include "Game/Components/SkyboxComponent.h"
#include "Game/Components/PointLightComponent.h"
#include "Rendering/RenderScene.h"

#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "EditorEngine.h"

#include "Platform/Windows/DirectX/DirectXFrameBuffer.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGui/ImGui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_thorium.h"
#include "EditorWidgets.h"

CModelEditor::CModelEditor()
{
	scene = CreateObject<CWorld>();
	scene->InitWorld(CWorld::InitializeInfo().CreateRenderScene(true).RegisterForRendering(true));

	auto skyEnt = scene->CreateEntity<CEntity>();
	skyEnt->AddComponent<CSkyboxComponent>("skybox");

	modelEnt = scene->CreateEntity<CModelEntity>();

	framebuffer = gRenderer->CreateFrameBuffer(1280, 720, TEXTURE_FORMAT_RGBA8_UNORM);
	scene->GetRenderScene()->SetFrameBuffer(framebuffer);

	camera = new CCameraProxy();
	camera->position = { 0, 0, -2 };

	//scene->SetPrimaryCamera(camera);

	light1 = modelEnt->AddComponent<CPointLightComponent>("Light1");
	light1->AttachTo(modelEnt->RootComponent());
	light1->SetPosition({ -1, 1, -1 });

	openMdlId = "MdlEditorOpenMdl" + FString::ToString((SizeType)this);
	saveMdlId = "MdlEditorSaveMdl" + FString::ToString((SizeType)this);
}

void CModelEditor::SetModel(CModelAsset* mdl)
{
	this->mdl = mdl;
	modelEnt->SetModel(mdl);
}

void CModelEditor::OnUpdate(double dt)
{
	int w, h;
	framebuffer->GetSize(w, h);
	if (w != viewportWidth || h != viewportHeight)
	{
		framebuffer->Resize(FMath::Max(viewportWidth, 16), FMath::Max(viewportHeight, 16));
	}

	scene->Update(dt);
	scene->GetRenderScene()->SetFrameBuffer(framebuffer);

	scene->SetPrimaryCamera(camera);
}

void CModelEditor::OnUIRender()
{
	FString title = "Model Editor";
	if (mdl && mdl->File())
		title += " - " + mdl->File()->Name();
	else if (mdl)
		title += " - New Model";

	title += "###modelEditor_" + FString::ToString((SizeType)this);

	bool bOpen = true;
	ImGui::SetNextWindowSize(ImVec2(1280, 720), ImGuiCond_FirstUseEver);

	auto flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoSavedSettings;
	if (!bSaved)
		flags |= ImGuiWindowFlags_UnsavedDocument;

	if (ImGui::Begin(title.c_str(), &bOpen, flags))
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New", "Ctrl+N"))
				{
					if (mdl && !bSaved)
					{
						// Save first
					}
					else
					{
						SetModel(CreateObject<CModelAsset>());
						bSaved = false;
					}
				}
				if (ImGui::MenuItem("Open", "Ctrl+O"))
				{

				}
				if (ImGui::MenuItem("Save", "Ctrl+S"))
				{
					if (mdl)
					{
						if (!mdl->File())
						{
							
						}

						if (mdl->File())
						{
							mdl->Save();
							bSaved = true;
						}
					}
				}
				if (ImGui::MenuItem("Save As", "Ctrl+Shift+S"));

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Recalculate Bounds") && mdl)
				{
					for (auto& m : mdl->GetMeshes())
						m.CalculateBounds();
					mdl->CalculateBounds();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImVec2 size = ImGui::GetContentRegionAvail();
		sizeR = size.x - sizeL;

		ImGui::Splitter("##mdleditSplitter", false, 4.f, &sizeL, &sizeR, 100);

		if (ImGui::BeginChild("mdleditProps", ImVec2(sizeL, 0), false, ImGuiWindowFlags_AlwaysUseWindowPadding))
		{
			ImGui::BeginDisabled(mdl == nullptr);

			ImVec2 mdlPropsSize = ImGui::GetContentRegionAvail();

			if (ImGui::Button(bCompiled ? "Compiled!" : "Needs Compiling!", ImVec2(mdlPropsSize.x, 28)))
				Compile();

			if (ImGui::BeginTable("entityComponentsEdit", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY))
			{
				ImGui::TableSetupColumn("Name");
				ImGui::TableSetupColumn("Value");
				ImGui::TableHeadersRow();

				ImVec2 cursor = ImGui::GetCursorPos();
				ImVec2 availSize = ImGui::GetContentRegionAvail();

				bool bOpen = ImGui::TableTreeHeader("Meshes", ImGuiTreeNodeFlags_AllowOverlap);
				if (bOpen)
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();
				
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.180f, 0.180f, 0.180f, 1.000f));
					if (ImGui::Button("Add##meshAdd", ImVec2(0, 24)))
					{
						bCompiled = false;
						bSaved = false;
						meshFiles.Add();
					}
					ImGui::SameLine();
					if (ImGui::Button("Clear##meshClear", ImVec2(0, 24)))
					{
						bCompiled = false;
						bSaved = false;
						meshFiles.Clear();
					}
					ImGui::PopStyleColor();

					int i = 0;
					int remove = -1;
					for (auto& mesh : meshFiles)
					{
						/*ImGui::TableNextRow();
						ImGui::TableNextColumn();*/

						bOpen = ImGui::TableTreeHeader(mesh.name.IsEmpty() ? ("New Mesh##" + FString::ToString(i)).c_str() : (mesh.name + "##" + FString::ToString(i)).c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap);
						ImGui::TableNextColumn();
						ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
						if (ImGui::Button("Remove"))
							remove = i;
						ImGui::PopStyleColor();

						if (bOpen)
						{
							ImGui::TableNextRow();
							ImGui::TableNextColumn();
							ImGui::Text("File");
							ImGui::TableNextColumn();

							if (ImGui::InputText(("##meshFile" + FString::ToString((SizeType)&mesh)).c_str(), &mesh.file, ImGuiInputTextFlags_EnterReturnsTrue))
							{
								LoadMeshFile(mesh);
							}

							ImGui::SameLine();

							if (ImGui::Button(("Browse##browseMesh" + FString::ToString(i)).c_str()))
							{
								const char* f = "FBX (.fbx)\0*.fbx\0Wavefront (.obj)\0*.obj\0glTF (.gltf)\0*.gltf\0\0";
								FString filter;
								filter.Resize(61);
								memcpy(filter.Data(), f, 61);

								FString file = CEngine::OpenFileDialog(filter);
								if (!file.IsEmpty())
								{
									mesh.file = file;
									LoadMeshFile(mesh);
								}
							}

							/*if (ImGui::TableTreeHeader(("Import##" + FString::ToString((SizeType)&mesh)).c_str(), 0, true))
							{
								ImGui::TableNextRow();
								ImGui::TableNextColumn();
								ImGui::Text("Import Textures");
								ImGui::TableNextColumn();

								ImGui::Checkbox(("##importTex" + FString::ToString((SizeType)&mesh)).c_str(), &mesh.bImportTextures);

								ImGui::TableNextRow();
								ImGui::TableNextColumn();
								ImGui::Text("Import Materials");
								ImGui::TableNextColumn();

								ImGui::Checkbox(("##importMats" + FString::ToString((SizeType)&mesh)).c_str(), &mesh.bImportMaterials);

								ImGui::TableNextRow();
								ImGui::TableNextColumn();
								ImGui::Text("Import Animations");
								ImGui::TableNextColumn();

								ImGui::Checkbox(("##importAnims" + FString::ToString((SizeType)&mesh)).c_str(), &mesh.bImportAnimations);

								ImGui::TableNextRow();
								ImGui::TableNextColumn();
								ImGui::Text("Import Tangents");
								ImGui::TableNextColumn();

								ImGui::Checkbox(("##importTan" + FString::ToString((SizeType)&mesh)).c_str(), &mesh.bImportTangents);

								ImGui::TreePop();
							}*/

							DrawMeshResources(mesh);

							ImGui::TreePop();
						}
						i++;
					}

					if (remove != -1)
						meshFiles.Erase(meshFiles.At(remove));

					ImGui::TreePop();
				}

				if (ImGui::TableTreeHeader("Materials", ImGuiTreeNodeFlags_AllowOverlap))
				{
					if (mdl)
					{
						for (auto& mat : mdl->materials)
						{
							ImGui::TableNextRow();
							ImGui::TableNextColumn();
							ImGui::Text(mat.name.c_str());
							ImGui::TableNextColumn();

							auto* matObj = (TObjectPtr<CObject>*)&mat.obj;
							if (ImGui::ObjectPtrWidget(("##_matObjPtr" + mat.name).c_str(), &matObj, 1, CMaterial::StaticClass()))
								mat.path = mat.obj->File() ? mat.obj->File()->Path() : FString();
						}
					}

					ImGui::TreePop();
				}

				if (ImGui::TableTreeHeader("Colliders", ImGuiTreeNodeFlags_AllowOverlap))
				{


					ImGui::TreePop();
				}

				if (ImGui::TableTreeHeader("Body Groups", ImGuiTreeNodeFlags_AllowOverlap))
				{


					ImGui::TreePop();
				}

				if (ImGui::TableTreeHeader("LOD Groups", ImGuiTreeNodeFlags_AllowOverlap))
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();

					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.180f, 0.180f, 0.180f, 1.000f));
					if (ImGui::Button("Add##lodAdd", ImVec2(0, 24)))
					{
						bCompiled = false;
						bSaved = false;
						mdl->numLODs++;
						if (mdl->numLODs > 6)
							mdl->numLODs = 6;
						else
							mdl->LODs[mdl->numLODs - 1] = FLODGroup();
					}
					ImGui::SameLine();
					if (ImGui::Button("Clear##lodClear", ImVec2(0, 24)))
					{
						bCompiled = false;
						bSaved = false;
						mdl->numLODs = 0;
					}
					ImGui::PopStyleColor();

					for (int i = 0; i < mdl->numLODs; i++)
					{
						auto& lod = mdl->LODs[i];
						if (ImGui::TableTreeHeader(("LOD " + FString::ToString(i)).c_str(), 0, true))
						{
							ImGui::TableNextRow();
							ImGui::TableNextColumn();
							ImGui::Text("Distance Bias");
							ImGui::TableNextColumn();
							ImGui::DragFloat(("##lodBias" + FString::ToString(i)).c_str(), &mdl->LODs[i].distanceBias, 0.1f);

							ImGui::TableNextRow();
							ImGui::TableNextColumn();
							ImGui::Text("Meshes");
							ImGui::TableNextColumn();

							const char* preview = lod.meshIndices.Size() == 0 ? "none" : (lod.meshIndices.Size() > 1 ? "Multiple" : mdl->meshes[lod.meshIndices[0]].meshName.c_str());
							if (ImGui::BeginCombo(("##lodMeshes" + FString::ToString(i)).c_str(), ""))
							{
								for (int m = 0; m < mdl->meshes.Size(); m++)
								{
									bool bSelected = false;
									for (auto& ii : lod.meshIndices)
										if (ii == m)
											bSelected = true;

									if (ImGui::Selectable(("##_comboMesh" + FString::ToString(i)).c_str(), bSelected))
									{
										if (bSelected)
											lod.meshIndices.Erase(lod.meshIndices.Find(m));
										else
											lod.meshIndices.Add(m);
									}
								}

								ImGui::EndCombo();
							}

							ImGui::TreePop();
						}
					}

					ImGui::TreePop();
				}

				ImGui::EndTable();
			}

			ImGui::EndDisabled();
		}
		ImGui::EndChild();

		ImGui::SameLine();

		auto wndSize = ImGui::GetContentRegionAvail();
		auto cursorPos = ImGui::GetCursorScreenPos();
		viewportX = cursorPos.x;
		viewportY = cursorPos.y;

		DirectXFrameBuffer* fb = (DirectXFrameBuffer*)framebuffer;
		ImGui::Image(fb->view, { wndSize.x, wndSize.y });

		if (ImGui::BeginDragDropTarget())
		{
			if (auto* p = ImGui::AcceptDragDropPayload("THORIUM_ASSET_FILE"); p != nullptr)
			{
				FFile* file = *(FFile**)p->Data;
				FAssetClass* type = CResourceManager::GetResourceTypeByFile(file);
				if (type == (FAssetClass*)CModelAsset::StaticClass())
				{
					SetModel(CResourceManager::GetResource<CModelAsset>(file->Path()));
				}
			}
			ImGui::EndDragDropTarget();
		}

		viewportWidth = FMath::Max((int)wndSize.x, 32);
		viewportHeight = FMath::Max((int)wndSize.y, 32);
	}
	ImGui::End();

	if (!bOpen)
	{
		bExit = true;

		gEditorEngine()->PollRemoveLayer(this);
	}
}

void CModelEditor::OnDetach()
{
	scene->Delete();
	delete framebuffer;
	delete camera;
}

void CModelEditor::LoadMeshFile(FMeshFile& m)
{
	unsigned int flags = aiProcess_Triangulate | aiProcess_MakeLeftHanded | aiProcess_OptimizeMeshes | aiProcess_FlipUVs | aiProcess_PopulateArmatureData;

	//if (!m.bImportTangents)
		flags |= aiProcess_CalcTangentSpace;

	m.scene = m.importer.ReadFile(m.file.c_str(), flags);
	if (!m.scene)
		return;

	m.name = m.file;
	
	if (auto i = m.name.FindLastOf("/\\"); i != -1)
		m.name.Erase(m.name.begin(), m.name.begin() + i + 1);

	bCompiled = false;
}

void CModelEditor::Compile()
{

}

void CModelEditor::DrawMeshResources(FMeshFile& m)
{
	const aiScene* scene = m.scene;

	if (!scene)
		return;

	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::Text("Meshes");
	ImGui::TableNextColumn();
	ImGui::Text("%d", scene->mNumMeshes);

	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::Text("Materials");
	ImGui::TableNextColumn();
	ImGui::Text("%d", scene->mNumMaterials);

	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::Text("Textures");
	ImGui::TableNextColumn();
	ImGui::Text("%d", scene->mNumTextures);

	if (scene->mRootNode)
		DrawAiNode(scene, scene->mRootNode);
}

void CModelEditor::DrawAiNode(const aiScene* scene, aiNode* node)
{
	if (ImGui::TableTreeHeader(node->mName.C_Str(), 0, true))
	{
		if (node->mNumMeshes)
		{
			if (ImGui::TableTreeHeader("Meshes", 0, true))
			{
				for (int i = 0; i < node->mNumMeshes; i++)
				{
					aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
					if (ImGui::TableTreeHeader(mesh->mName.C_Str(), 0, true))
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("Vertices");
						ImGui::TableNextColumn();
						ImGui::Text("%d", mesh->mNumVertices);

						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("Faces");
						ImGui::TableNextColumn();
						ImGui::Text("%d", mesh->mNumFaces);

						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("Has Normals");
						ImGui::TableNextColumn();
						ImGui::Text(mesh->mNormals ? "true" : "false");

						ImGui::TableNextRow();
						ImGui::TableNextColumn();
						ImGui::Text("Has Tangents");
						ImGui::TableNextColumn();
						ImGui::Text(mesh->mTangents ? "true" : "false");

						ImGui::TreePop();
					}
				}

				ImGui::TreePop();
			}
		}

		if (node->mNumChildren > 0)
		{
			if (ImGui::TableTreeHeader("Children", 0, true))
			{
				for (int i = 0; i < node->mNumChildren; i++)
					DrawAiNode(scene, node->mChildren[i]);

				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}
}
