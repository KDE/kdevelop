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

#include <qwhatsthis.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <ktexteditor/markinterface.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/document.h>
#include <kaction.h>

#include <kdevpartcontroller.h>
#include <kdevcore.h>
#include <kdevmainwindow.h>

#include "bookmarks_widget.h"
#include "bookmarks_part.h"


typedef KGenericFactory<BookmarksPart> BookmarksFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevbookmarks, BookmarksFactory( "kdevbookmarks" ) );

BookmarksPart::BookmarksPart(QObject *parent, const char *name, const QStringList& )
	: KDevPlugin("bookmarks", "bookmarks", parent, name ? name : "BookmarksPart" )
{
	setInstance(BookmarksFactory::instance());

	_widget = new BookmarksWidget(this);

	_widget->setCaption(i18n("Bookmarks"));
	_widget->setIcon(SmallIcon("bookmark"));

	QWhatsThis::add(_widget, i18n("Bookmarks\n\n"
			"The bookmark viewer shows all the source bookmarks in the project."));

	mainWindow()->embedSelectView(_widget, i18n("Bookmarks"), i18n("source bookmarks"));

	// ===================

	_editorMap.setAutoDelete( true );

	_settingMarks = false;

	connect( core(), SIGNAL( projectOpened() ), this, SLOT( projectOpened() ) );
	connect( core(), SIGNAL( projectClosed() ), this, SLOT( projectClosed() ) );

	connect( partController(), SIGNAL( partAdded( KParts::Part * ) ), this, SLOT( partAdded( KParts::Part * ) ) );

	connect( _widget, SIGNAL( removeAllBookmarksForURL( const KURL & ) ),
		this, SLOT( removeAllBookmarksForURL( const KURL & ) ) );
	connect( _widget, SIGNAL( removeBookmarkForURL( const KURL &, int ) ),
		this, SLOT( removeBookmarkForURL( const KURL &, int ) ) );

	// if there is a project loaded, read bookmarks from project file

	// then ... setBookmarksForAllURLs();

	// and ... storeBookmarksForAllURLs();


}

BookmarksPart::~BookmarksPart()
{
	delete _widget;
}

void BookmarksPart::partAdded( KParts::Part * part )
{
	kdDebug(0) << "BookmarksPart::partAdded()" << endl;

	if ( KParts::ReadOnlyPart * ro_part = dynamic_cast<KParts::ReadOnlyPart *>( part ) )
	{
		if ( setBookmarksForURL( ro_part ) )
		{
			// connect to this editor
			KTextEditor::Document * doc = static_cast<KTextEditor::Document*>( ro_part );
			connect( doc, SIGNAL( marksChanged() ), this, SLOT( marksChanged() ) );

			// workaround for a katepart oddity where it drops all bookmarks on 'reload'
			connect( doc, SIGNAL( completed() ), this, SLOT( reload() ) );
		}
	}
}

void BookmarksPart::reload()
{
	kdDebug(0) << "BookmarksPart::reload()" << endl;

    QObject * senderobj = const_cast<QObject*>( sender() );
	if ( KParts::ReadOnlyPart * ro_part = dynamic_cast<KParts::ReadOnlyPart *>( senderobj ) )
	{
		setBookmarksForURL( ro_part );
	}
}

void BookmarksPart::marksChanged()
{
	kdDebug(0) << "BookmarksPart::marksChanged()" << endl;

    QObject * senderobj = const_cast<QObject*>( sender() );
	KParts::ReadOnlyPart * ro_part = dynamic_cast<KParts::ReadOnlyPart *>( senderobj );
	KTextEditor::MarkInterface * mi = dynamic_cast<KTextEditor::MarkInterface*>( senderobj );

	// don't react if we're in the middle of setting marks
	if ( ! _settingMarks )
	{
		if ( ro_part && mi )
		{
			kdDebug(0) << "found a MarkInterface" << endl;

			if ( EditorData * data = storeBookmarksForURL( ro_part ) )
			{
				_widget->updateURL( data );
			}
			else
			{
				_widget->removeURL( ro_part->url() );
			}
		}
		else
		{
			kdDebug(0) << "ReadOnlyPart == " << ro_part << endl
				<< "MarkInterface == " << mi << endl
				<< "examining all loaded parts instead" << endl;

			storeBookmarksForAllURLs();
			_widget->update( _editorMap );

			return;
		}
	}
	else
	{
		kdDebug(0) << "currently setting marks, bailing out" << endl;
	}
}

void BookmarksPart::projectOpened()
{
	kdDebug(0) << "BookmarksPart::projectOpened()" << endl;

	// here we need to retrieve saved bookmarks
}

void BookmarksPart::projectClosed()
{
	kdDebug(0) << "BookmarksPart::projectClosed()" << endl;

	// here we need to save bookmarks
}

void BookmarksPart::removeAllBookmarksForURL( KURL const & url )
{
	kdDebug(0) << "BookmarksPart::removeAllBookmarksForURL()" << endl;

	_editorMap.remove( url.path() );

	setBookmarksForURL( partForURL( url ) );
	_widget->removeURL( url );
}

void BookmarksPart::removeBookmarkForURL( KURL const & url, int line )
{
	kdDebug(0) << "BookmarksPart::removeBookmarkForURL()" << endl;

	if ( EditorData * data = _editorMap.find( url.path() ) )
	{
		QValueListIterator< QPair<int,QString> > it = data->marks.begin();
		while ( it != data->marks.end() )
		{
			if ( (*it).first == line )
			{
				kdDebug(0) << "removing bookmark. Line: " << line << endl;
				data->marks.remove( it );
				break;
			}
			++it;
		}
		setBookmarksForURL( partForURL( url ) );
		_widget->updateURL( data );
	}
}

bool BookmarksPart::setBookmarksForURL( KParts::ReadOnlyPart * ro_part )
{
	if ( KTextEditor::MarkInterface * mi = dynamic_cast<KTextEditor::MarkInterface *>(ro_part) )
	{
		_settingMarks = true;

		mi->clearMarks();

		if ( EditorData * data = _editorMap.find( ro_part->url().path() ) )
		{
			// we've seen this one before, apply stored bookmarks

			QValueListIterator< QPair<int,QString> > it = data->marks.begin();
			while ( it != data->marks.end() )
			{
				kdDebug(0) << "Setting bookmark. Line: " << (*it).first << endl;
				mi->addMark( (*it).first, KTextEditor::MarkInterface::markType01 );
				++it;
			}
		}
		_settingMarks = false;

		// true == this is a MarkInterface
		return true;
	}
	return false;
}

EditorData * BookmarksPart::storeBookmarksForURL( KParts::ReadOnlyPart * ro_part )
{
	KTextEditor::EditInterface * ed = dynamic_cast<KTextEditor::EditInterface *>( ro_part );

	if ( KTextEditor::MarkInterface * mi = dynamic_cast<KTextEditor::MarkInterface *>( ro_part ) )
	{
		EditorData * data = new EditorData;
		data->url = ro_part->url();

		// removing previous data for this url, if any
		if ( _editorMap.remove( data->url.path() ) )
		{
			kdDebug(0) << "removed previous data" << endl;
		}

		QPtrList<KTextEditor::Mark> marks = mi->marks();
		QPtrListIterator<KTextEditor::Mark> it( marks );
		while ( it.current() )
		{
			if ( it.current()->type == KTextEditor::MarkInterface::markType01 )
			{
				int line = it.current()->line;
				QString textLine;
				if ( ed )
				{
					textLine = ed->textLine( line );
				}

				kdDebug(0) << "Found bookmark. Line: " << line << endl;

				data->marks.append( qMakePair( line, textLine) );
			}
			++it;
		}

		if ( ! data->marks.isEmpty() )
		{
			kdDebug(0) << data->marks.count() << " bookmarks in " << data->url.path() << " - Keeping" << endl;

			_editorMap.insert( data->url.path(), data );
		}
		else
		{
			kdDebug(0) << "No bookmarks in " << data->url.path() << " - Deleting" << endl;
			delete data;
			data = 0;
		}
		kdDebug(0) << "_editorMap.count(): " << _editorMap.count() << endl;
		return data;
	}
	return 0;
}

void BookmarksPart::setBookmarksForAllURLs()
{
	if( const QPtrList<KParts::Part> * partlist = partController()->parts() )
	{
		QPtrListIterator<KParts::Part> it( *partlist );
		while ( KParts::Part* part = it.current() )
		{
			if ( KParts::ReadOnlyPart * ro_part = dynamic_cast<KParts::ReadOnlyPart *>( part ) )
			{
				setBookmarksForURL( ro_part );
			}
			++it;
		}
	}
}

void BookmarksPart::storeBookmarksForAllURLs()
{
	if( const QPtrList<KParts::Part> * partlist = partController()->parts() )
	{
		QPtrListIterator<KParts::Part> it( *partlist );
		while ( KParts::Part* part = it.current() )
		{
			if ( KParts::ReadOnlyPart * ro_part = dynamic_cast<KParts::ReadOnlyPart *>( part ) )
			{
				storeBookmarksForURL( ro_part );
			}
			++it;
		}
	}
}

// reimplemented from PartController::partForURL to avoid linking
KParts::ReadOnlyPart * BookmarksPart::partForURL( KURL const & url )
{
    QPtrListIterator<KParts::Part> it( *partController()->parts() );
    while( it.current() )
    {
        KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(it.current());
        if (ro_part && url == ro_part->url())
        {
            return ro_part;
        }
        ++it;
    }
    return 0;
}


#include "bookmarks_part.moc"
