#pragma once

#include "ToolsCore.h"
#include <QObject>
#include <functional>

class SDK_API FHistoryEvent
{
public:
	FHistoryEvent(const FString& name, std::function<void()> funUndo, std::function<void()> funRedo);
	FHistoryEvent(const FString& name, const FString& description, std::function<void()> funUndo, std::function<void()> funRedo);

	inline void Undo() { if (funUndo) funUndo(); }
	inline void Redo() { if (funRedo) funRedo(); }

	inline const FString& Name() const { return name; }
	inline const FString& Description() const { return description; }

protected:
	FString name;
	FString description;

	std::function<void()> funUndo; 
	std::function<void()> funRedo;
};

class SDK_API CHistoryBuffer : public QObject
{
	Q_OBJECT

public:
	CHistoryBuffer() = default;
	CHistoryBuffer(const CHistoryBuffer& other) = delete;
	~CHistoryBuffer() = default;

	void AddEvent(const FHistoryEvent& event);
	void ClearHistory();

	void Undo();
	void Redo();

Q_SIGNALS:
	void onEventAdded();
	void onUndo(SizeType cursor);
	void onRedo(SizeType cursor);

public:
	TArray<FHistoryEvent> events;
	SizeType cursor = -1;
};
