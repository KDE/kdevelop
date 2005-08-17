/***************************************************************************
 *   Copyright (C) 2003 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef __KDEVPART_BOOKMARKS_H__
#define __KDEVPART_BOOKMARKS_H__


#include <qpointer.h>
#include <q3valuelist.h>
#include <q3dict.h>
#include <qpair.h>
//Added by qt3to4:
#include <QTextStream>

#include <kparts/part.h>
#include <kurl.h>

#include <kdevplugin.h>

class QTimer;
class KDialogBase;
class BookmarksConfig;
class ConfigWidgetProxy;

struct EditorData
{
	KURL url;
	Q3ValueList< QPair<int,QString> > marks;
};

class BookmarksWidget;
class QDomElement;

class BookmarksPart : public KDevPlugin
{
	Q_OBJECT

public:

	BookmarksPart(QObject *parent, const char *name, const QStringList &);
	~BookmarksPart();

	// reimplemented from KDevPlugin
	void restorePartialProjectSession( const QDomElement * el );
	void savePartialProjectSession( QDomElement * el );
	
	BookmarksConfig * config();

	QStringList getContext( KURL const & url, unsigned int line, unsigned int context );
	
private slots:
	// connected to partcontroller
	void partAdded( KParts::Part * part );

	// connected to KTextEditor::MarkInterface
	void marksEvent();

	// connected to _marksChangeTimer	
	void marksChanged();
	
	// connected to KParts::ReadOnlyPart
	void reload();

	// connected to BookmarksWidget
	void removeAllBookmarksForURL( const KURL & );
	void removeBookmarkForURL( const KURL &, int );

	void insertConfigWidget( const KDialogBase * dlg, QWidget * page, unsigned int );
	
private:
	bool setBookmarksForURL( KParts::ReadOnlyPart * );
	bool clearBookmarksForURL( KParts::ReadOnlyPart * );
	void setBookmarksForAllURLs();

	EditorData * storeBookmarksForURL( KParts::ReadOnlyPart * );
	void storeBookmarksForAllURLs();

	void updateContextStringForURL( KParts::ReadOnlyPart * );
	void updateContextStringForURL( KURL const & url );
	void updateContextStringForAll();

	QStringList getContextFromStream( QTextStream & istream, unsigned int line, unsigned int context );
	
	KParts::ReadOnlyPart * partForURL( KURL const & url );
	bool partIsSane( KParts::ReadOnlyPart * );

	QPointer<BookmarksWidget> _widget;
	Q3Dict<EditorData> _editorMap;
	bool _settingMarks;	//	are we currently in the process of setting bookmarks?
	
	BookmarksConfig * _config;
	
	ConfigWidgetProxy * _configProxy;
	
	QTimer * _marksChangeTimer;
	Q3ValueList<KParts::ReadOnlyPart*> _dirtyParts;
};


#endif

// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
