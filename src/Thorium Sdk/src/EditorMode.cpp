
#include "EditorMode.h"
#include "EditorEngine.h"

CEditorMode::CEditorMode()
{
	CEditorModeRegistry::AddToRegistry(this);
}

CEditorMode::~CEditorMode()
{
	CEditorModeRegistry::Remove(this);
}
