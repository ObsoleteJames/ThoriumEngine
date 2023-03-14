#pragma once

#include <Util/Core.h>
#include <QObject>
#include <QSplitter>

class CSplitterLink : QObject
{
	Q_OBJECT

public:
	CSplitterLink() = default;

	void AddSplitter(QSplitter* splitter);
	void RemoveSplitter(QSplitter* splitter);
	void Clear();

private:
	void OnSplitterMove(QSplitter* splitter);

private:
	TArray<QSplitter*> splitters;

};
