#pragma once

#include "ToolsCore.h"
#include <QObject>
#include <functional>

class SDK_API IHistoryEvent
{
public:
	virtual void Undo() = 0;
	virtual void Redo() = 0;

	inline const FString& Name() const { return name; }
	inline const FString& Description() const { return description; }

protected:
	FString name;
	FString description;

};

class SDK_API FHistoryEvent : IHistoryEvent
{
public:
	FHistoryEvent(const FString& name, std::function<void()> funUndo, std::function<void()> funRedo);
	FHistoryEvent(const FString& name, const FString& description, std::function<void()> funUndo, std::function<void()> funRedo);

	void Undo() override;
	void Redo() override;

protected:
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

	void AddEvent(IHistoryEvent* event);
	void ClearHistory();

	void Undo();
	void Redo();

Q_SIGNALS:
	void onEventAdded();
	void onUndo(SizeType cursor);
	void onRedo(SizeType cursor);

public:
	TArray<IHistoryEvent*> events;
	SizeType cursor = -1;
};
