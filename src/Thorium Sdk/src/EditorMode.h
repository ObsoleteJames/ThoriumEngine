#pragma once

#include "ToolsCore.h"
#include <QIcon>

class SDK_API CEditorMode
{
public:
	CEditorMode();
	~CEditorMode();

	virtual void Init() = 0;

	// Called when the editor mode is changed to this mode.
	virtual void Open() {}
	// Called when the editor mode is changed from this to another.
	virtual void Close() {}

	virtual void OnLevelChange() {}

	// Called every frame in the editor
	virtual void EditorUpdate(double dt) {}

	// Called every frame while in play mode.
	virtual void GameUpdate(double dt) {}

	virtual void Render() {}

	inline const FString& Name() const { return name; }
	inline const QIcon& Icon() const { return icon; }

protected:
	QIcon icon;
	FString name;

};
