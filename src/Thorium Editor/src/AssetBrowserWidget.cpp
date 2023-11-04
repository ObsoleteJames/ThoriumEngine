
#include "AssetBrowserWidget.h"
#include "Registry/FileSystem.h"
#include "Math/Math.h"
#include "Resources/ResourceManager.h"
#include "Resources/Asset.h"
#include "Rendering/Shader.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_thorium.h"
#include "EditorWidgets.h"

void CAssetBrowserWidget::RenderUI(float width, float height)
{
	ImVec2 size = width == 0.f ? ImGui::GetContentRegionAvail() : ImVec2(width, height);
	sizeR = size.x - sizeL;

	ImGui::Splitter("##assetBorwserSplitter", false, 4.f, &sizeL, &sizeR);

	auto& mods = CFileSystem::GetMods();

	if (ImGui::BeginChild("assetBrowserTree", ImVec2(sizeL, height), false, ImGuiWindowFlags_AlwaysUseWindowPadding))
	{
		for (auto& m : mods)
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
			if (dir.IsEmpty() && m->Name() == mod)
				flags |= ImGuiTreeNodeFlags_Selected;

			bool bOpen = ImGui::TreeNodeEx(ToFString(m->Name()).c_str(), flags);

			if (ImGui::IsItemClicked())
				SetDir(m->Name(), L"");

			if (bOpen)
			{
				FDirectory* root = m->GetRootDir();
				//DrawDirTree(root, nullptr);
				for (auto d : root->GetSubDirectories())
					DrawDirTree(d, root, m);

				ImGui::TreePop();
			}
		}
	}
	ImGui::EndChild();

	ImGui::SameLine();

	if (ImGui::BeginChild("assetBrowserView", ImVec2(sizeR,	height), false, ImGuiWindowFlags_AlwaysUseWindowPadding))
	{
		bool bHovered = ImGui::IsItemHovered();
		if (ImGui::Button("<##_browserBack") || (ImGui::IsKeyPressed(ImGuiKey_MouseX1)))
			Back();

		ImGui::SameLine();

		if (ImGui::Button(">##_browserForward"))
			Forward();

		ImGui::SameLine();

		if (ImGui::Button("-##_browserRoot"))
			Root();

		ImGui::SameLine();

		if (ImGui::InputText("##_dirInput", &dirInput))
		{
			WString _d;
			WString _m;

			if (ExtractPath(ToWString(dirInput), _m, _d))
			{
				FMod* m = CFileSystem::FindMod(_m);
				if (m)
				{
					mod = m->Name();
					
					if (_d.IsEmpty() || m->FindDirectory(_d))
					{
						dir = _d;
					}
				}
			}
		}

		ImVec2 itemSize = ImVec2(48 + (48 * iconsSize) / 2, 80 + (80 * iconsSize) / 2);
		ImVec2 contentSize = ImGui::GetContentRegionAvail();
		int columns = FMath::Max((int)(contentSize.x / (itemSize.x + 10.f)), 1);

		FMod* curMod = CFileSystem::FindMod(mod);
		FDirectory* curDir = dir.IsEmpty() ? curMod->GetRootDir() : curMod->FindDirectory(dir);

		if (ImGui::BeginTable("tableAssetBrowser", columns, ImGuiTableFlags_None))
		{
			for (auto& d : curDir->GetSubDirectories())
			{
				ImGui::TableNextColumn();
				//if (ImGui::Selectable(ToFString(d->GetName()).c_str(), false, ImGuiSelectableFlags_None, ImVec2(itemSize.x, itemSize.x)))
				if (ImGui::ButtonEx(ToFString(d->GetName()).c_str(), ImVec2(itemSize.x, itemSize.x), ImGuiButtonFlags_PressedOnDoubleClick))
				{
					SetDir(mod, d->GetPath());
				}
				if (ImGui::IsItemClicked())
					SetSelectedFile(nullptr);
			}

			for (auto& f : curDir->GetFiles())
			{
				FAssetClass* type = CResourceManager::GetResourceTypeByFile(f);

				if (viewFilter && viewFilter != type)
					continue;

				ImGui::TableNextColumn();
				ImVec2 cursor = ImGui::GetCursorScreenPos();
				
				bool bSelected = IsFileSelected(f);

				if (bSelected)
					ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
				if (ImGui::ButtonEx((ToFString(f->Name()) + "##_" + FString::ToString((SizeType)f)).c_str(), itemSize))
				{
					if (ImGui::IsKeyDown(ImGuiKey_ModCtrl))
					{
						if (!bSelected)
							AddSelectedFile(f);
						else
							RemoveSelectedFile(f);
					}
					else
						SetSelectedFile(f);

					// TODO: open file editor
				}
				if (ImGui::BeginPopupContextItem())
				{
					ImGui::MenuItem("Rename");
					ImGui::MenuItem("Delete");
					if (type == (FAssetClass*)CShaderSource::StaticClass())
					{
						ImGui::Separator();
						if (ImGui::MenuItem("Compile"))
						{
							auto shader = CResourceManager::GetResource<CShaderSource>(f->Path());
							shader->Compile();
						}
					}
					ImGui::EndPopup();
				}

				if (!bDoubleClickedFile)
					bDoubleClickedFile = ImGui::IsItemClicked() && ImGui::IsMouseDoubleClicked(0);

				if (bSelected)
					ImGui::PopStyleColor();

				if (ImGui::BeginDragDropSource())
				{
					if (type)
						ImGui::SetDragDropPayload("THORIUM_ASSET_FILE", &f, sizeof(void*));
					else
						ImGui::SetDragDropPayload("THORIUM_GENERIC_FILE", &f, sizeof(void*));

					ImGui::EndDragDropSource();
				}

				uint32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(0.06f, 0.06f, 0.06f, 1.00f));
				ImGui::RenderFrame(cursor, cursor + ImVec2(itemSize.x, itemSize.x), col, false, ImGui::GetStyle().FrameRounding);
				ImGui::RenderFrame(cursor + ImVec2(0, ImGui::GetStyle().FrameRounding + 2.f), cursor + ImVec2(itemSize.x, itemSize.x), col, false);

				FString name = ToFString(f->Name());

				ImGui::RenderTextWrapped(cursor + ImVec2(5, itemSize.x + 5), name.c_str(), name.last()++, itemSize.x - 5);

				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 0.4f));
				if (!type || type == (FAssetClass*)CAsset::StaticClass())
				{
					FString ext = type ? "Generic" : ToFString(f->Extension());
					ImGui::RenderTextClipped(cursor + ImVec2(5, itemSize.y - 18), cursor + ImVec2(itemSize.x - 5, itemSize.y), ext.c_str(), nullptr, nullptr);
				}
				else
					ImGui::RenderTextClipped(cursor + ImVec2(5, itemSize.y - 18), cursor + ImVec2(itemSize.x - 5, itemSize.y), type->GetName().c_str(), nullptr, nullptr);
				ImGui::PopStyleColor();

			}

			ImGui::EndTable();
		}
	}
	ImGui::EndChild();
}

void CAssetBrowserWidget::SetDir(FMod* m, FDirectory* d)
{
	SetDir(m->Name(), d->GetPath());
}

void CAssetBrowserWidget::SetDir(const WString& m, const WString& d)
{
	mod = m;
	dir = d;
	WString p = mod + L":\\" + dir;
	dirInput = ToFString(p);

	if (historyIndex < historyList.Size())
		historyList.Erase(historyList.begin() + historyIndex, historyList.end());

	historyList.Add(p);
	historyIndex = historyList.Size();

	SetSelectedFile(nullptr);
}

void CAssetBrowserWidget::Back()
{
	if (historyIndex < 2)
		return;

	historyIndex--;
	ExtractPath(historyList[historyIndex - 1], mod, dir);

	if (dir == L"\\")
		dir.Clear();

	WString p = mod + L":\\" + dir;
	dirInput = ToFString(p);
}

void CAssetBrowserWidget::Forward()
{
	if (historyIndex == historyList.Size())
		return;

	historyIndex++;
	ExtractPath(historyList[historyIndex - 1], mod, dir);

	if (dir == L"\\")
		dir.Clear();

	WString p = mod + L":\\" + dir;
	dirInput = ToFString(p);
}

void CAssetBrowserWidget::Root()
{
	FMod* m = CFileSystem::FindMod(mod);
	FDirectory* d = m->FindDirectory(dir);
	if (d && d->Parent())
		SetDir(mod, d->Parent()->GetPath());
}

void CAssetBrowserWidget::DrawDirTree(FDirectory* _dir, FDirectory* parent, FMod* _mod)
{
	bool bHasChildren = _dir->GetSubDirectories().Size() > 0;
	bool bSelected = _mod->Name() == mod && _dir->GetPath() == dir;

	ImGuiTreeNodeFlags flags = (!bHasChildren ? ImGuiTreeNodeFlags_Leaf : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
	if (bSelected)
		flags |= ImGuiTreeNodeFlags_Selected;

	bool bOpen = ImGui::TreeNodeEx(ToFString(_dir->GetName() + L"##_" + _mod->Name() + _dir->GetPath()).c_str(), flags);

	if (ImGui::IsItemClicked())
		SetDir(_mod, _dir);

	if (bOpen)
	{
		for (auto d : _dir->GetSubDirectories())
		{
			DrawDirTree(d, _dir, _mod);
		}
		ImGui::TreePop();
	}
}

void CAssetBrowserWidget::SetSelectedFile(FFile* file)
{
	selectedFiles.Clear();
	if (file)
		selectedFiles.Add(file);
}

void CAssetBrowserWidget::AddSelectedFile(FFile* file)
{
	selectedFiles.Add(file);
}

void CAssetBrowserWidget::RemoveSelectedFile(FFile* file)
{
	auto it = selectedFiles.Find(file);
	if (it != selectedFiles.end())
		selectedFiles.Erase(it);
}

bool CAssetBrowserWidget::IsFileSelected(FFile* file)
{
	auto it = selectedFiles.Find(file);
	if (it != selectedFiles.end())
		return true;
	return false;
}

bool CAssetBrowserWidget::ExtractPath(const WString& combined, WString& outMod, WString& outDir)
{
	SizeType colon = combined.FindFirstOf(':');
	if (colon != -1)
	{
		outDir = combined;
		outMod = combined;
		SizeType i = outDir.Size() > colon + 2 ? 2 : 1;
		outDir.Erase(outDir.begin(), outDir.begin() + colon + i);
		outMod.Erase(outMod.begin() + colon, outMod.end());
		return true;
	}
	return false;
}
