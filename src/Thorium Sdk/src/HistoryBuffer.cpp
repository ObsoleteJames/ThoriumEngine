#pragma once

#include "HistoryBuffer.h"

FHistoryEvent::FHistoryEvent(const FString& n, std::function<void()> _funUndo, std::function<void()> _funRedo) : name(n), funUndo(_funUndo), funRedo(_funRedo)
{
}

FHistoryEvent::FHistoryEvent(const FString& n, const FString& d, std::function<void()> _funUndo, std::function<void()> _funRedo) : name(n), description(d), funUndo(_funUndo), funRedo(_funRedo)
{
}

void CHistoryBuffer::AddEvent(const FHistoryEvent& event)
{
	if (cursor + 1 < events.Size())
		events.Erase(events.begin() + cursor + 1, events.end());

	events.Add(event);
	cursor = events.Size() - 1;

	emit(onEventAdded());
}

void CHistoryBuffer::ClearHistory()
{
	events.Clear();
	cursor = -1;
}

void CHistoryBuffer::Undo()
{
	if (cursor == -1)
		return;

	events[cursor].Undo();
	cursor--;

	emit(onUndo(cursor));
}

void CHistoryBuffer::Redo()
{
	if (cursor + 1 == events.Size())
		return;

	cursor++;
	events[cursor].Redo();

	emit(onRedo(cursor));
}
