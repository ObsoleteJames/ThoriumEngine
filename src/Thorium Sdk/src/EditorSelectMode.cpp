
#include "EditorMode.h"

class SDK_API CEditorSelectMode : public CEditorMode
{
public:
	void Init() override
	{
		name = "Select Mode";
	}

};
static CEditorSelectMode selectMode;
