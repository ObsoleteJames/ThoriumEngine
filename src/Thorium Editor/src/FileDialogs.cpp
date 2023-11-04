
#include "EditorEngine.h"
#include "FileDialogs.h"
#include "Game/Events.h"
#include "Object/Object.h"

#include "AssetBrowserWidget.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGui/ImGui.h"
#include "ImGui/imgui_internal.h"

static bool bInit = false;
static bool bSelected = false;
static bool bClose = false;

static CAssetBrowserWidget* browser;
static SizeType curId = 0;

// fucking terrible hack to make this work
class CThFileDialog : public CObject
{
public:
	CThFileDialog()
	{
		name = "ThFileDialogHandler";
		browser = new CAssetBrowserWidget();
		Events::OnRender.Bind(this, &CThFileDialog::Update);
	}

	void Update()
	{
		ImGui::SetNextWindowSize(ImVec2(900, 600), ImGuiCond_FirstUseEver);
		if (ImGui::BeginPopupModal("Open File##thFileDialogOpenFile", nullptr, 0))
		{
			ImVec2 size = ImGui::GetContentRegionAvail();

			browser->RenderUI(0, size.y - 32);

			ImGui::SetCursorPosX(size.x - 90);

			if (ImGui::Button("Cancel"))
			{
				curId = 0;
				bSelected = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();

			if (!browser->GetSelectedFile())
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.1f, 0.1f, 1.f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.1f, 0.1f, 1.f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.1f, 0.1f, 1.f));
			}

			if ((ImGui::Button("Open") || browser->bDoubleClickedFile) && browser->GetSelectedFile())
				bSelected = true;

			if (!browser->GetSelectedFile())
				ImGui::PopStyleColor(3);

			if (bClose)
			{
				ImGui::CloseCurrentPopup();
				bClose = false;
			}

			ImGui::EndPopup();
		}

		ImGui::SetNextWindowSize(ImVec2(900, 600), ImGuiCond_FirstUseEver);
		if (ImGui::BeginPopupModal("Save File##thFileDialogSaveFile", nullptr, 0))
		{
			ImVec2 size = ImGui::GetContentRegionAvail();

			browser->RenderUI(0, size.y - 32);

			ImGui::SetCursorPosX(size.x - 90);

			if (ImGui::Button("Cancel"))
			{
				curId = 0;
				bSelected = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();

			if (!browser->GetSelectedFile())
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.1f, 0.1f, 1.f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.1f, 0.1f, 1.f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.1f, 0.1f, 0.1f, 1.f));
			}

			if (ImGui::Button("Save") && browser->GetSelectedFile())
				bSelected = true;

			if (!browser->GetSelectedFile())
				ImGui::PopStyleColor(3);

			if (bClose)
			{
				ImGui::CloseCurrentPopup();
				bClose = false;
			}

			ImGui::EndPopup();
		}
	}
};

static TObjectPtr<CThFileDialog> dialogHandler;

void ThFileDialog_Init()
{
	if (dialogHandler || bInit)
		return;

	dialogHandler = new CThFileDialog();
	// Call update so that the popups work.
	dialogHandler->Update();
	bInit = true;
}

bool ThFileDialog::OpenFile(const FString& id, FAssetClass* type, const WString& dir /*= WString()*/)
{
	if (!bInit)
		ThFileDialog_Init();

	if (curId != 0)
		return false;

	if (!dir.IsEmpty())
	{
		WString m;
		WString d;
		browser->ExtractPath(dir, m, d);
		browser->SetDir(m, d);
	}
	else
		browser->SetDir(gEditorEngine()->ActiveGame().mod->Name(), WString());

	browser->viewFilter = type;
	browser->bDoubleClickedFile = false;
	curId = id.Hash();
	bSelected = false;

	ImGui::OpenPopup("Open File##thFileDialogOpenFile");
	return true;
}

bool ThFileDialog::SaveFile(const FString& id, FAssetClass* type, const WString& dir /*= WString()*/)
{
	if (!bInit)
		ThFileDialog_Init();

	if (curId != 0)
		return false;

	return true;
}

bool ThFileDialog::AcceptFile(const FString& id, WString* outFile, WString* outMod)
{
	if (curId != id.Hash())
		return false;

	if (bSelected && browser->GetSelectedFile())
	{
		*outFile = browser->GetSelectedFile()->Path();
		if (outMod)
			*outMod = browser->GetSelectedFile()->Mod()->Name();
		curId = 0;
		bSelected = false;
		bClose = true;
	}
	return true;
}

void ThFileDialog::Cancel(const FString& id)
{
	if (curId == 0)
		return;

	bSelected = false;
	curId = 0;
	bClose = true;
}
