/*
 *  Copyright (C) 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
 */


#ifndef __KDEVPART_BOOKMARKS_H__
#define __KDEVPART_BOOKMARKS_H__


#include <qguardedptr.h>
#include <qvaluelist.h>
#include <qdict.h>

#include <kdevplugin.h>
#include <kparts/part.h>
#include <kurl.h>

struct EditorData
{
	KURL url;
	QValueList<int> marks;
};

class BookmarksWidget;

class BookmarksPart : public KDevPlugin
{
	Q_OBJECT

public:

	BookmarksPart(QObject *parent, const char *name, const QStringList &);
	~BookmarksPart();

private slots:
	// connected to core
	void projectOpened();
	void projectClosed();

	// connected to partcontroller
	void partAdded( KParts::Part * part );

	// connected to KTextEditor::MarkInterface
	void marksChanged();

private:

	EditorData * storeBookmarksForURL( KParts::ReadOnlyPart * );
	void examineLoadedParts();

	QGuardedPtr<BookmarksWidget> _widget;
	QDict<EditorData> _editorMap;
	bool _settingMarks;

};


#endif
