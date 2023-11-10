
#include <string>
#include "PropertyEditor.h"
#include "Game/Entity.h"
#include "EditorEngine.h"

#include "Math/Color.h"

#include <map>
#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_thorium.h"
#include "EditorWidgets.h"

void CPropertyEditor::OnUIRender()
{
	auto& selectedEntities = gEditorEngine()->selectedEntities;

	// Property Editor
	if (ImGui::Begin("Properties##_editorPropertyEditor", &bEnabled))
	{
		char editName[65];
		if (selectedEntities.Size() == 1)
			memcpy(editName, selectedEntities[0]->Name().c_str(), FMath::Min(selectedEntities[0]->Name().Size() + 1, 64ull));
		else if (selectedEntities.Size() > 1)
			memcpy(editName, "Multiple Selected", 18);
		else
			editName[0] = '\0';

		if (ImGui::InputText("Name##_editorPropertyEditor", editName, 64, selectedEntities.Size() == 1 ? 0 : ImGuiInputTextFlags_ReadOnly))
		{
			selectedEntities[0]->SetName(editName);
		}

		if (selectedEntities.Size() == 1)
		{
			if (prevEnt == nullptr)
				prevEnt = selectedEntities[0];
			else if (selectedEntities[0] != prevEnt)
			{
				prevEnt = selectedEntities[0];
				rotCache = prevEnt->RootComponent()->GetRotation().ToEuler().Degrees();
				selectedComp = nullptr;
			}

			if (ImGui::BeginTable("entityComponentsEdit", 1, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg, ImVec2(0, 48)))
			{
				// TODO: this idfk
				struct FComponentTree {
					TArray<FComponentTree> childTrees;
					FComponentTree* parent;
					TObjectPtr<CEntityComponent> comp;

					FComponentTree* AddChild(CEntityComponent* comp) {
						childTrees.Add();
						auto* t = &*childTrees.last();
						t->parent = this;
						t->comp = comp;
						return t;
					}
					FComponentTree* AddChild(CSceneComponent* comp) {
						childTrees.Add();
						auto* t = &*childTrees.last();
						t->parent = this;
						t->comp = comp;

						for (TObjectPtr<CSceneComponent>& c : comp->GetChildren())
						{
							if (c)
								t->AddChild(c);
						}

						return t;
					}
					FComponentTree* FindTree(CEntityComponent* c) 
					{
						if (c == comp)
							return this;

						for (auto& child : childTrees)
						{
							if (auto* t = child.FindTree(c); t != nullptr)
								return t;
						}

						return nullptr;
					}

					void DrawUI(CEntityComponent*& selectedComp)
					{
						ImGui::TableNextRow();
						ImGui::TableNextColumn();

						bool bSelected = selectedComp == comp;

						if (!bSelected)
							ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
						else
							ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.21f, 0.26f, 0.38f, 1.00f));

						if (ImGui::Selectable(("##_compSelect" + comp->Name()).c_str(), bSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap))
						{
							selectedComp = comp;
						}
						ImGui::PopStyleColor();
						ImGui::SameLine();

						ImVec2 cursor = ImGui::GetCursorScreenPos();
						if (childTrees.Size() > 0)
						{
							ImGui::SetNextItemWidth(10);
							ImGui::SetCursorScreenPos(cursor - ImVec2(14, 0));
							bool bOpen = ImGui::TreeNodeEx(("##_treeComp" + comp->Name()).c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen);
							ImGui::SameLine();

							ImGui::SetCursorScreenPos(cursor + ImVec2(6, 0));
							ImGui::Text(comp->Name().c_str());
							
							if (bOpen)
							{
								for (auto& c : childTrees)
								{
									c.DrawUI(selectedComp);
								}
								ImGui::TreePop();
							}
						}
						else
						{
							ImGui::SetCursorScreenPos(cursor + ImVec2(6, 0));
							ImGui::Text(comp->Name().c_str());
						}
					}
				};

				static FComponentTree compTree;
				compTree.childTrees.Clear();
				
				static TArray<CEntityComponent*> nonSceneComps;
				nonSceneComps.Clear();

				for (auto& comp : selectedEntities[0]->GetAllComponents())
				{
					if (auto scene = CastChecked<CSceneComponent>(comp); scene != nullptr)
					{
						if (scene->GetParent() == nullptr && scene != selectedEntities[0]->RootComponent())
						{
							nonSceneComps.Add(comp);
						}
					}
					else
					{
						nonSceneComps.Add(comp);
					}
				}

				if (selectedEntities[0]->RootComponent())
				{
					for (TObjectPtr<CSceneComponent>& c : selectedEntities[0]->RootComponent()->GetChildren())
						if (c)
							compTree.AddChild(c);
				}

				ImGui::TableNextRow();
				ImGui::TableNextColumn();

				bool bSelected = selectedComp == nullptr;

				if (!bSelected)
					ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
				else
					ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.21f, 0.26f, 0.38f, 1.00f));

				if (ImGui::Selectable(("##_compSelect" + selectedEntities[0]->Name()).c_str(), bSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap))
				{
					selectedComp = nullptr;
				}
				ImGui::PopStyleColor();
				ImGui::SameLine();

				ImVec2 cursor = ImGui::GetCursorScreenPos();

				ImGui::SetNextItemWidth(10);
				ImGui::SetCursorScreenPos(cursor - ImVec2(14, 0));
				bool bOpen = ImGui::TreeNodeEx(("##_treeEnt" + selectedEntities[0]->Name()).c_str(), ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen);
				ImGui::SameLine();

				ImGui::SetCursorScreenPos(cursor + ImVec2(6, 0));
				ImGui::Text(selectedEntities[0]->Name().c_str());

				if (bOpen)
				{
					for (auto& c : compTree.childTrees)
						c.DrawUI(selectedComp);

					ImGui::TreePop();
				}

				if (nonSceneComps.Size() > 0)
					ImGui::Separator();

				for (auto& c : nonSceneComps)
				{
					ImGui::TableNextRow();
					ImGui::TableNextColumn();

					bool bSelected = selectedComp == c;

					if (!bSelected)
						ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
					else
						ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.21f, 0.26f, 0.38f, 1.00f));

					if (ImGui::Selectable(("##_compSelect" + c->Name() + FString::ToString((SizeType)c)).c_str(), bSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap))
					{
						selectedComp = c;

						if (auto* sc = Cast<CSceneComponent>(c); sc)
							rotCache = sc->GetRotation().ToEuler().Degrees();
					}
					ImGui::PopStyleColor();
					ImGui::SameLine();

					ImVec2 cursor = ImGui::GetCursorScreenPos();
					ImGui::SetCursorScreenPos(cursor + ImVec2(6, 0));
					ImGui::Text(c->Name().c_str());
				}

				ImGui::EndTable();
			}
		}
		else
		{
			prevEnt = nullptr;
			selectedComp = nullptr;
		}

		if (selectedEntities.Size() > 0)
		{
			static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY;
			if (ImGui::BeginTable("propertyEditorTable", 2, flags))
			{
				FClass* _class = selectedEntities[0]->GetClass();
				for (int i = 1; i < selectedEntities.Size(); i++)
				{
					if (selectedEntities[i]->GetClass() != _class)
					{
						// Fallback to entity class if all selected objects aren't the same type
						_class = CEntity::StaticClass();
						break;
					}
				}

				ImGui::TableSetupColumn("Name");
				ImGui::TableSetupColumn("Value");
				ImGui::TableHeadersRow();

				_class = selectedComp ? selectedComp->GetClass() : _class;

				if (_class->CanCast(CEntity::StaticClass()) || _class->CanCast(CSceneComponent::StaticClass()))
					RenderTransformEdit();

				RenderClassProperties(_class, 0);
				ImGui::EndTable();
			}
		}
	}
	ImGui::End();
}

void CPropertyEditor::RenderClassProperties(FStruct* type, SizeType offset)
{
	auto& selectedEntities = gEditorEngine()->selectedEntities;
	std::multimap<std::string, const FProperty*> propertyPerCategory;

	FClass* _class = type->IsClass() ? (FClass*)type : nullptr;
	for (FClass* c = _class; c != nullptr; c = c->GetBaseClass())
	{
		for (const FProperty* p = c->GetPropertyList(); p != nullptr; p = p->next)
		{
			if ((p->flags & VTAG_EDITOR_VISIBLE) == 0 && (p->flags & VTAG_EDITOR_EDITABLE) == 0)
				continue;

			if (p->meta && !p->meta->category.IsEmpty())
				propertyPerCategory.insert(std::make_pair(p->meta->category.c_str(), p));
			else
				propertyPerCategory.insert(std::make_pair(c->GetName().c_str(), p));
		}
	}
	if (!_class)
	{
		for (const FProperty* p = type->GetPropertyList(); p != nullptr; p = p->next)
		{
			if ((p->flags & VTAG_EDITOR_VISIBLE) == 0 && (p->flags & VTAG_EDITOR_EDITABLE) == 0)
				continue;

			if (p->meta && !p->meta->category.IsEmpty())
				propertyPerCategory.insert(std::make_pair(p->meta->category.c_str(), p));
			else
				propertyPerCategory.insert(std::make_pair(type->GetName().c_str(), p));
		}
	}

	ImVec4* colors = ImGui::GetStyle().Colors;

	FString curCat = propertyPerCategory.size() > 0 ? propertyPerCategory.begin()->first : "NULL";
	ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FrameDontExpand;

	//ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5, 0));
	//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 7));
	//ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0);
	//ImGui::TableNextRow();
	//ImGui::TableNextColumn();

	//bool bOpen = ImGui::TreeNodeEx(curCat.c_str(), treeFlags);
	//ImGui::PopStyleVar(3);
	bool bOpen = ImGui::TableTreeHeader(curCat.c_str(), treeFlags);
	
	for (auto& it : propertyPerCategory)
	{
		bool bReadOnly = (it.second->flags & VTAG_EDITOR_EDITABLE) == 0;
		if (curCat != it.first)
		{
			curCat = it.first;
			if (bOpen)
				ImGui::TreePop();

			bOpen = ImGui::TableTreeHeader(curCat.c_str(), treeFlags);
		}

		if (!bOpen)
			continue;

		const FProperty* prop = it.second;
		if (prop->type != EVT_ARRAY && prop->type != EVT_STRUCT)
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::SetCursorScreenPos(ImGui::GetCursorScreenPos() + ImVec2(0, 4));
			ImGui::Text(prop->name.c_str());
			ImGui::TableNextColumn();
		}

		FString propId = FString::ToString((SizeType)prop);

		void* obj = selectedComp != nullptr ? (void*)selectedComp : (void*)&*selectedEntities[0];

		static TArray<void*> objects;
		objects.Clear();

		if (selectedEntities.Size() > 1) 
		{
			for (auto& t : selectedEntities)
				objects.Add((void*)&*t);
		}
		objects.Add(obj);

		RenderProperty(prop->type, prop, objects.Data(), objects.Size(), prop->offset + offset);
	}
	if (bOpen)
		ImGui::TreePop();
}

void CPropertyEditor::RenderProperty(uint type, const FProperty* prop, void** objects, int objCount, SizeType offset)
{
	FString propId = FString::ToString((SizeType)prop + offset);
	bool bReadOnly = (prop->flags & VTAG_EDITOR_EDITABLE) == 0;
	switch (type)
	{
	case EVT_ENUM:
	{
		FEnum* _enum = CModuleManager::FindEnum(prop->typeName);
		if (!_enum)
			break;

		void* value = (void*)(((SizeType)objects[0]) + offset);
		int64 v;
		switch (_enum->Size())
		{
		case 1:
			v = *(int8*)value;
			break;
		case 2:
			v = *(int16*)value;
			break;
		case 4:
			v = *(int32*)value;
			break;
		case 8:
			v = *(int64*)value;
			break;
		}

		FString name = _enum->GetNameByValue(v);

		if (objCount > 1)
		{
			bool bDifferent = false;

			for (int i = 1; i < objCount; i++)
			{
				void* _v = (void*)(((SizeType)objects[i]) + offset);

				if (_enum->Size() == 1 && *(int8*)_v != v)
				{
					bDifferent = true;
					break;
				}
				else if (_enum->Size() == 2 && *(int16*)_v != v)
				{
					bDifferent = true;
					break;
				}
				else if (_enum->Size() == 4 && *(int32*)_v != v)
				{
					bDifferent = true;
					break;
				}
				else if (_enum->Size() == 8 && *(int64*)_v != v)
				{
					bDifferent = true;
					break;
				}
			}

			if (ImGui::BeginCombo(("##_combo" + propId).c_str(), bDifferent ? "Multiple Values" : name.c_str()))
			{
				for (auto& val : _enum->GetValues())
				{
					bool bSelected = bDifferent ? false : val.Value == v;
					if (ImGui::Selectable(val.Key.c_str(), bSelected))
					{
						for (int i = 0; i < objCount; i++)
						{
							void* _v = (void*)(((SizeType)objects[i]) + offset);
							v = val.Value;
							switch (_enum->Size())
							{
							case 1:
								*(int8*)_v = v;
								break;
							case 2:
								*(int16*)_v = v;
								break;
							case 4:
								*(int32*)_v = v;
								break;
							case 8:
								*(int64*)_v = v;
								break;
							}
						}
					}
					if (bSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
		}
		else
		{
			if (ImGui::BeginCombo(("##_combo" + propId).c_str(), name.c_str()))
			{
				for (auto& val : _enum->GetValues())
				{
					bool bSelected = val.Value == v;
					if (ImGui::Selectable(val.Key.c_str(), bSelected))
					{
						v = val.Value;
						switch (_enum->Size())
						{
						case 1:
							*(int8*)value = v;
							break;
						case 2:
							*(int16*)value = v;
							break;
						case 4:
							*(int32*)value = v;
							break;
						case 8:
							*(int64*)value = v;
							break;
						}
					}
					if (bSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
		}
	}
	break;
	case EVT_ARRAY:
	{
		if (objCount == 1 && ImGui::TableTreeHeader(prop->name.c_str(), 0, true))
		{
			FArrayHelper* helper = (FArrayHelper*)prop->typeHelper;
			SizeType numElements = helper->Size((void*)((SizeType)objects[0] + offset));

			for (int i = 0; i < numElements; i++)
			{
				void* obj = helper->Data((void*)((SizeType)objects[0] + offset));
				
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text((FString::ToString(i) + ":").c_str());
				ImGui::TableNextColumn();

				RenderProperty(helper->objType, prop, &obj, 1, i * helper->objSize);
			}

			ImGui::TreePop();
		}
	}
	break;
	case EVT_OBJECT_PTR:
	{
		TObjectPtr<CObject>* _obj = objCount == 1 ? (TObjectPtr<CObject>*)((SizeType)objects[0] + offset) : nullptr;
		static TArray<TObjectPtr<CObject>*> objects;
		objects.Clear();
		if (_obj == nullptr)
		{
			for (int i = 0; i < objCount; i++)
			{
				objects.Add((TObjectPtr<CObject>*)((SizeType)objects[i] + offset));
			}
		}
		else
			objects.Add((TObjectPtr<CObject>*)_obj);

		FClass* type = CModuleManager::FindClass(prop->typeName);

		if (!type->CanCast(CAsset::StaticClass()))
			ImGui::ObjectPtrWidget(("##_objectPtr" + propId).c_str(), objects.Data(), objects.Size(), type);
		else
			ImGui::AssetPtrWidget(("##_assetPtr" + propId).c_str(), (TObjectPtr<CAsset>**)objects.Data(), objects.Size(), (FAssetClass*)type);
	}
	break;
	case EVT_STRING:
	{
		FString* str = (FString*)(((SizeType)objects[0]) + offset);
		/*char buff[64];
		memcpy(buff, str->Data(), str->Size() + 1);

		if (ImGui::InputText(("##_inputText" + propId).c_str(), buff, 64, bReadOnly ? ImGuiInputTextFlags_ReadOnly : 0))
			*str = buff;*/
		ImGui::InputText(("##_inputText" + propId).c_str(), str);
	}
	break;
	case EVT_FLOAT:
	{
		float step = (prop->meta && prop->meta->HasFlag("UIStepSize")) ? std::stof(prop->meta->FlagValue("UIStepSize").c_str()) : 0.1f;
		FString format = (prop->meta && prop->meta->HasFlag("UIFormat")) ? prop->meta->FlagValue("UIFormat") : "%.3f";

		if (objCount > 1)
		{
			float* value = (float*)(((SizeType)objects[0]) + offset);
			bool bDifferent = false;

			for (int i = 1; i < objCount; i++)
			{
				float v = *(float*)(((SizeType)objects[i]) + offset);
				if (v != *value)
				{
					bDifferent = true;
					break;
				}
			}

			if (ImGui::DragFloat(("##_floatInput" + propId).c_str(), value, step, 0, 0, bDifferent ? "Multiple Values" : format.c_str(), bReadOnly ? ImGuiSliderFlags_ReadOnly : 0))
			{
				for (int i = 0; i < objCount; i++)
				{
					float& _v = *(float*)(((SizeType)objects[i]) + offset);
					_v = *value;
				}
			}
		}
		else
		{
			float* v = (float*)(((SizeType)objects[0]) + offset);
			ImGui::DragFloat(("##_floatInput" + propId).c_str(), v, step, 0, 0, format.c_str(), bReadOnly ? ImGuiSliderFlags_ReadOnly : 0);
		}
	}
	break;
	case EVT_INT:
	{
		if (objCount > 1)
		{

		}
		else
		{
			switch (prop->size)
			{
			case 1:
			{
				int8* v = (int8*)(((SizeType)objects[0]) + offset);
				int _v = *v;

				if (ImGui::DragInt(("##_dragInt" + propId).c_str(), &_v, 1, 0, 0, "%d", bReadOnly ? ImGuiSliderFlags_ReadOnly : 0))
					*v = _v;

			}
			break;
			case 2:
			{
				int16* v = (int16*)(((SizeType)objects[0]) + offset);
				int _v = *v;

				if (ImGui::DragInt(("##_dragInt" + propId).c_str(), &_v, 1, 0, 0, "%d", bReadOnly ? ImGuiSliderFlags_ReadOnly : 0))
					*v = _v;

			}
			break;
			case 4:
			{
				int32* v = (int32*)(((SizeType)objects[0]) + offset);
				int _v = *v;

				if (ImGui::DragInt(("##_dragInt" + propId).c_str(), &_v, 1, 0, 0, "%d", bReadOnly ? ImGuiSliderFlags_ReadOnly : 0))
					*v = _v;

			}
			break;
			case 8:
			{
				int64* v = (int64*)(((SizeType)objects[0]) + offset);
				int _v = *v;

				if (ImGui::DragInt(("##_dragInt" + propId).c_str(), &_v, 1, 0, 0, "%d", bReadOnly ? ImGuiSliderFlags_ReadOnly : 0))
					*v = _v;

			}
			break;
			}
		}
	}
	break;
	case EVT_BOOL:
	{
		if (objCount > 1)
		{
			bool* value = (bool*)(((SizeType)objects[0]) + offset);
			bool bDifferent = false;

			for (int i = 1; i < objCount; i++)
			{
				bool v = *(bool*)(((SizeType)objects[i]) + offset);
				if (v != *value)
				{
					bDifferent = true;
					break;
				}
			}

			if (bDifferent)
				ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, true);

			if (ImGui::Checkbox(("##_checkbox" + propId).c_str(), value))
			{
				for (int i = 0; i < objCount; i++)
				{
					bool& _v = *(bool*)(((SizeType)objects[i]) + offset);
					_v = *value;
				}
			}

			if (bDifferent)
				ImGui::PopItemFlag();
		}
		else
		{
			bool* v = (bool*)(((SizeType)objects[0]) + offset);
			ImGui::Checkbox(("##_checkbox" + propId).c_str(), v);
		}
	}
	break;
	}
}

void CPropertyEditor::RenderTransformEdit()
{
	auto& selectedEntities = gEditorEngine()->selectedEntities;
	ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAllColumns | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_FrameDontExpand;

	bool bOpen = ImGui::TableTreeHeader("Transform", treeFlags);

	if (bOpen)
	{
		if (selectedEntities.Size() > 1)
		{
			CSceneComponent* root = selectedEntities[0]->RootComponent();


		}
		else
		{
			CSceneComponent* scene = selectedComp ? (CSceneComponent*)selectedComp : selectedEntities[0]->RootComponent();

			FVector* p = (FVector*)&scene->GetPosition();
			FVector* s = (FVector*)&scene->GetScale();
			FQuaternion* r = (FQuaternion*)&scene->GetRotation();

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::SetCursorScreenPos(ImGui::GetCursorScreenPos() + ImVec2(0, 4));
			ImGui::Text("Position");
			ImGui::TableNextColumn();

			CEntityComponent* _c = selectedComp;
			selectedComp = scene;
			RenderVectorProperty((SizeType)p - (SizeType)scene, false);

			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::SetCursorScreenPos(ImGui::GetCursorScreenPos() + ImVec2(0, 4));
			ImGui::Text("Rotation");
			ImGui::TableNextColumn();
			RenderQuatProperty((SizeType)r - (SizeType)scene, false);
			
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::SetCursorScreenPos(ImGui::GetCursorScreenPos() + ImVec2(0, 4));
			ImGui::Text("Scale");
			ImGui::TableNextColumn();
			RenderVectorProperty((SizeType)s - (SizeType)scene, false);
			selectedComp = _c;
		}

		ImGui::TreePop();
	}
}

void CPropertyEditor::RenderVectorProperty(SizeType offset, bool bReadOnly)
{
	auto& selectedEntities = gEditorEngine()->selectedEntities;
	void* obj = selectedComp != nullptr ? (void*)selectedComp : (void*)&*selectedEntities[0];

	auto areaSize = ImGui::GetContentRegionAvail();
	float tWidth = areaSize.x / 3 - 5.f;

	if (selectedEntities.Size() > 1)
	{
		FVector* v = (FVector*)(((SizeType)&*selectedEntities[0]) + offset);
		bool bXDiff = false;
		bool bYDiff = false;
		bool bZDiff = false;

		for (auto& t : selectedEntities)
		{
			FVector& _v = *(FVector*)(((SizeType)&*t) + offset);
			if (v->x != _v.x)
			{
				bXDiff = true;
			}
			if (v->y != _v.y)
			{
				bYDiff = true;
			}
			if (v->z != _v.z)
			{
				bZDiff = true;
			}
		}

		ImGui::SetNextItemWidth(tWidth);
		if (ImGui::DragFloat(("##_vectorInputX" + FString::ToString(offset + (SizeType)v)).c_str(), &v->x, 0.1f, 0, 0, bXDiff ? "Multiple Values" : "X:%.3f", bReadOnly ? ImGuiSliderFlags_ReadOnly : 0))
		{
			for (auto& t : selectedEntities)
			{
				FVector& _v = *(FVector*)(((SizeType) & *t) + offset);
				_v.x = v->x;
			}
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(tWidth);
		if (ImGui::DragFloat(("##_vectorInputY" + FString::ToString(offset + (SizeType)v)).c_str(), &v->y, 0.1f, 0, 0, bXDiff ? "Multiple Values" : "Y:%.3f", bReadOnly ? ImGuiSliderFlags_ReadOnly : 0))
		{
			for (auto& t : selectedEntities)
			{
				FVector& _v = *(FVector*)(((SizeType) & *t) + offset);
				_v.y = v->y;
			}
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(tWidth);
		if (ImGui::DragFloat(("##_vectorInputZ" + FString::ToString(offset + (SizeType)v)).c_str(), &v->z, 0.1f, 0, 0, bXDiff ? "Multiple Values" : "Z:%.3f", bReadOnly ? ImGuiSliderFlags_ReadOnly : 0))
		{
			for (auto& t : selectedEntities)
			{
				FVector& _v = *(FVector*)(((SizeType) & *t) + offset);
				_v.z = v->z;
			}
		}
	}
	else
	{
		FVector* v = (FVector*)(((SizeType)obj) + offset);
		ImGui::SetNextItemWidth(tWidth);
		ImGui::DragFloat(("##_vectorInputX" + FString::ToString(offset + (SizeType)v)).c_str(), &v->x, 0.1f, 0, 0, "X:%.3f", bReadOnly ? ImGuiSliderFlags_ReadOnly : 0);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(tWidth);
		ImGui::DragFloat(("##_vectorInputY" + FString::ToString(offset + (SizeType)v)).c_str(), &v->y, 0.1f, 0, 0, "Y:%.3f", bReadOnly ? ImGuiSliderFlags_ReadOnly : 0);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(tWidth);
		ImGui::DragFloat(("##_vectorInputZ" + FString::ToString(offset + (SizeType)v)).c_str(), &v->z, 0.1f, 0, 0, "Z:%.3f", bReadOnly ? ImGuiSliderFlags_ReadOnly : 0);
	}
}

void CPropertyEditor::RenderColorProperty(SizeType offset, bool bReadOnly)
{
	auto& selectedEntities = gEditorEngine()->selectedEntities;
	void* obj = selectedComp != nullptr ? (void*)selectedComp : (void*)&*selectedEntities[0];

	//auto areaSize = ImGui::GetContentRegionAvail();
	//float tWidth = areaSize.x / 3 - 5.f;

	if (selectedEntities.Size() > 1)
	{
		// TODO
	}
	else
	{
		FColor* v = (FColor*)(((SizeType)obj) + offset);
		ImGui::ColorEdit4(("##_colorEdit" + FString::ToString(offset + (SizeType)v)).c_str(), (float*)v);
	}
}

void CPropertyEditor::RenderQuatProperty(SizeType offset, bool bReadOnly)
{
	auto& selectedEntities = gEditorEngine()->selectedEntities;
	void* obj = selectedComp != nullptr ? (void*)selectedComp : (void*)&*selectedEntities[0];

	auto areaSize = ImGui::GetContentRegionAvail();
	float tWidth = areaSize.x / 3 - 5.f;

	if (selectedEntities.Size() > 1)
	{

	}
	else
	{
		FQuaternion* v = (FQuaternion*)(((SizeType)obj) + offset);
		FVector euler = v->ToEuler().Degrees();

		bool bUpdate = false;

		ImGui::SetNextItemWidth(tWidth);
		bUpdate = ImGui::DragFloat(("##_vectorInputX" + FString::ToString(offset + (SizeType)v)).c_str(), &rotCache.x, 0.1f, 0, 0, "X:%.3f", bReadOnly ? ImGuiSliderFlags_ReadOnly : 0);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(tWidth);
		bUpdate = ImGui::DragFloat(("##_vectorInputY" + FString::ToString(offset + (SizeType)v)).c_str(), &rotCache.y, 0.1f, 0, 0, "Y:%.3f", bReadOnly ? ImGuiSliderFlags_ReadOnly : 0) || bUpdate;
		ImGui::SameLine();
		ImGui::SetNextItemWidth(tWidth);
		bUpdate = ImGui::DragFloat(("##_vectorInputZ" + FString::ToString(offset + (SizeType)v)).c_str(), &rotCache.z, 0.1f, 0, 0, "Z:%.3f", bReadOnly ? ImGuiSliderFlags_ReadOnly : 0) || bUpdate;

		if (bUpdate)
		{
			*v = FQuaternion::EulerAngles(rotCache.Radians());
		}
	}
}
