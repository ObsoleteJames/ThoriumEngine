
#include "SplitterLink.h"

void CSplitterLink::AddSplitter(QSplitter* splitter)
{
	splitters.Add(splitter);
	connect(splitter, &QSplitter::splitterMoved, this, [=](int pos, int a) { this->OnSplitterMove(splitter); });
}

void CSplitterLink::RemoveSplitter(QSplitter* splitter)
{
	splitters.Erase(splitters.Find(splitter));
}

void CSplitterLink::Clear()
{
	splitters.Clear();
}

void CSplitterLink::OnSplitterMove(QSplitter* splitter)
{
	auto sizes = splitter->sizes();
	for (auto* p : splitters)
	{
		if (p == splitter)
			continue;

		p->setSizes(sizes);
	}
}
