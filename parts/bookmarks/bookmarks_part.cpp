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
#include <qvbox.h>
#include <qtimer.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <ktexteditor/markinterface.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/document.h>
#include <kaction.h>
#include <kdialogbase.h>

#include <kdevpartcontroller.h>
#include <kdevcore.h>
#include <kdevmainwindow.h>
#include "domutil.h"

#include "bookmarks_widget.h"
#include "bookmarks_part.h"
#include "bookmarks_settings.h"
#include "bookmarks_config.h"

typedef KGenericFactory<BookmarksPart> BookmarksFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevbookmarks, BookmarksFactory( "kdevbookmarks" ) )

BookmarksPart::BookmarksPart(QObject *parent, const char *name, const QStringList& )
	: KDevPlugin("bookmarks", "bookmarks", parent, name ? name : "BookmarksPart" )
{
	setInstance(BookmarksFactory::instance());

	_widget = new BookmarksWidget(this);

	_widget->setCaption(i18n("Bookmarks"));
	_widget->setIcon(SmallIcon("bookmark"));
	
	_marksChangeTimer = new QTimer( this );

	QWhatsThis::add(_widget, i18n("<b>Bookmarks</b><p>"
			"The bookmark viewer shows all the source bookmarks in the project."));

	mainWindow()->embedSelectView(_widget, i18n("Bookmarks"), i18n("Source bookmarks"));

	// ===================

	_editorMap.setAutoDelete( true );

	_settingMarks = false;

	connect( partController(), SIGNAL( partAdded( KParts::Part * ) ), this, SLOT( partAdded( KParts::Part * ) ) );
	connect( core(), SIGNAL( projectConfigWidget( KDialogBase * ) ), this, SLOT( projectConfigWidget( KDialogBase * ) ) );
	
	connect( _widget, SIGNAL( removeAllBookmarksForURL( const KURL & ) ),
		this, SLOT( removeAllBookmarksForURL( const KURL & ) ) );
	connect( _widget, SIGNAL( removeBookmarkForURL( const KURL &, int ) ),
		this, SLOT( removeBookmarkForURL( const KURL &, int ) ) );
		
	connect( _marksChangeTimer, SIGNAL( timeout() ), this, SLOT( marksChanged() ) );

	_config = new BookmarksConfig;
	_config->readConfig();

	storeBookmarksForAllURLs();
	updateContextStringForAll();
	_widget->update( _editorMap );
}

BookmarksPart::~BookmarksPart()
{
	if( _widget ) {
		mainWindow()->removeView( _widget );
		delete _widget;
	}
	delete _config;
}

void BookmarksPart::partAdded( KParts::Part * part )
{
	//kdDebug(0) << "BookmarksPart::partAdded()" << endl;

	if ( KParts::ReadOnlyPart * ro_part = dynamic_cast<KParts::ReadOnlyPart *>( part ) )
	{
		if ( setBookmarksForURL( ro_part ) )
		{
			updateContextStringForURL( ro_part );
			if ( EditorData * data = _editorMap.find( ro_part->url().path() ) )
			{
				_widget->updateURL( data );
			}
			
			// connect to this editor
			KTextEditor::Document * doc = static_cast<KTextEditor::Document*>( ro_part );
			connect( doc, SIGNAL( marksChanged() ), this, SLOT( marksEvent() ) );

			// workaround for a katepart oddity where it drops all bookmarks on 'reload'
			connect( doc, SIGNAL( completed() ), this, SLOT( reload() ) );
		}
	}
}

void BookmarksPart::reload()
{
	//kdDebug(0) << "BookmarksPart::reload()" << endl;

	QObject * senderobj = const_cast<QObject*>( sender() );
	if ( KParts::ReadOnlyPart * ro_part = dynamic_cast<KParts::ReadOnlyPart *>( senderobj ) )
	{
		if ( partIsSane( ro_part ) )
		{
			setBookmarksForURL( ro_part );
		}
	}
}

void BookmarksPart::marksEvent()
{
	//kdDebug(0) << "BookmarksPart::marksEvent()" << endl;
	
	if ( ! _settingMarks )
	{
		QObject * senderobj = const_cast<QObject*>( sender() );
		KParts::ReadOnlyPart * ro_part = dynamic_cast<KParts::ReadOnlyPart *>( senderobj );
		
		if ( partIsSane( ro_part ) && !_dirtyParts.contains( ro_part ) )
		{
			_dirtyParts.push_back( ro_part );
			_marksChangeTimer->start( 1000, true );
		}
	}
}

void BookmarksPart::marksChanged()
{
	//kdDebug(0) << "BookmarksPart::marksChanged()" << endl;

	QValueListIterator<KParts::ReadOnlyPart*> it = _dirtyParts.begin();
	while ( it != _dirtyParts.end() )
	{
		KParts::ReadOnlyPart * ro_part = *it;
		KTextEditor::MarkInterface * mi = dynamic_cast<KTextEditor::MarkInterface*>( ro_part );
	
		if ( ro_part && mi && partIsSane( ro_part ) )
		{
			if ( EditorData * data = storeBookmarksForURL( ro_part ) )
			{
				updateContextStringForURL( ro_part );
				_widget->updateURL( data );
			}
			else
			{
				_widget->removeURL( ro_part->url() );
			}
		}
		++it;
	}
	_dirtyParts.clear();	
}

void BookmarksPart::projectConfigWidget( KDialogBase *dlg )
{
    QVBox *vbox = dlg->addVBoxPage( i18n("Bookmarks") );
	BookmarkSettings * w = new BookmarkSettings( this, vbox );
    connect( dlg, SIGNAL(okClicked()), w, SLOT(slotAccept()) );
}

void BookmarksPart::restorePartialProjectSession( const QDomElement * el )
{
	//kdDebug(0) << "BookmarksPart::restorePartialProjectSession()" << endl;

	if ( ! el ) return;

	QDomElement bookmarksList = el->namedItem( "bookmarks" ).toElement();
	if ( bookmarksList.isNull() ) return;

	QDomElement bookmark = bookmarksList.firstChild().toElement();
	while ( ! bookmark.isNull() )
	{
		QString path = bookmark.attribute( "url" );
		if ( path != QString::null )
		{
			EditorData * data = new EditorData;
			data->url.setPath( path );

			QDomElement mark = bookmark.firstChild().toElement();
			while ( ! mark.isNull() )
			{
				QString line = mark.attribute( "line" );
				if ( line != QString::null )
				{
					data->marks.append( qMakePair( line.toInt(), QStringList() ) );
				}
				mark = mark.nextSibling().toElement();
			}

			if ( ! data->marks.isEmpty() )
			{
				_editorMap.insert( data->url.path(), data );
			}
			else
			{
				delete data;
			}
		}
		bookmark = bookmark.nextSibling().toElement();
	}
	setBookmarksForAllURLs();
	updateContextStringForAll();
	_widget->update( _editorMap );
}

void BookmarksPart::savePartialProjectSession( QDomElement * el )
{
	//kdDebug(0) << "BookmarksPart::savePartialProjectSession()" << endl;

	if ( ! el ) return;

    QDomDocument domDoc = el->ownerDocument();
    if ( domDoc.isNull() ) return;

	QDomElement bookmarksList = domDoc.createElement( "bookmarks" );

	QDictIterator<EditorData> it( _editorMap );
	while ( it.current() )
	{
		QDomElement bookmark = domDoc.createElement( "bookmark" );
		bookmark.setAttribute( "url", it.current()->url.path() );
		bookmarksList.appendChild( bookmark );

		QValueListIterator< QPair<int,QStringList> > it2 = it.current()->marks.begin();
		while ( it2 != it.current()->marks.end() )
		{
			QDomElement line = domDoc.createElement( "mark" );
			line.setAttribute( "line", (*it2).first );
			bookmark.appendChild( line );
			++it2;
		}
		++it;
	}

	if ( ! bookmarksList.isNull() )
	{
		el->appendChild( bookmarksList );
	}
}

void BookmarksPart::removeAllBookmarksForURL( KURL const & url )
{
	//kdDebug(0) << "BookmarksPart::removeAllBookmarksForURL()" << endl;

	_editorMap.remove( url.path() );

	setBookmarksForURL( partForURL( url ) );
	_widget->removeURL( url );
}

void BookmarksPart::removeBookmarkForURL( KURL const & url, int line )
{
	//kdDebug(0) << "BookmarksPart::removeBookmarkForURL()" << endl;

	if ( EditorData * data = _editorMap.find( url.path() ) )
	{
		QValueListIterator< QPair<int,QStringList> > it = data->marks.begin();
		while ( it != data->marks.end() )
		{
			if ( (*it).first == line )
			{
				//kdDebug(0) << "removing bookmark. Line: " << line << endl;
				data->marks.remove( it );
				break;
			}
			++it;
		}

		if ( data->marks.isEmpty() )
		{
			removeAllBookmarksForURL( url );
		}
		else
		{
			setBookmarksForURL( partForURL( url ) );
			_widget->updateURL( data );
		}
	}
}

void BookmarksPart::updateContextStringForURL( KParts::ReadOnlyPart * ro_part )
{
	if ( ! ro_part ) return;

	KTextEditor::EditInterface * ed =
		dynamic_cast<KTextEditor::EditInterface *>( ro_part );

	EditorData * data = _editorMap.find( ro_part->url().path() );

	if ( ! ( data && ed ) ) return;

	QValueListIterator< QPair<int,QStringList> > it = data->marks.begin();
	while ( it != data->marks.end() )
	{
		int line = (*it).first;
		QString textLine;

		int context = config()->context();
 		int startline = context > line ? 0 : line - context;
 		int endline = ( line + context > ed->numLines() ) ? ed->numLines() : line + context;

		(*it).second.clear();
		
		for ( int i = line - context; i<= line + context; i++)
		{
			if ( i >= startline && i <= endline )
			{
				(*it).second << ed->textLine( i );
			}
			else
			{
				(*it).second << " ";	// creating empty lines for lines outside the editor
			}
		}
		
		++it;
	}
}

void BookmarksPart::updateContextStringForURL( KURL const & url )
{
	updateContextStringForURL( partForURL( url ) );
}

void BookmarksPart::updateContextStringForAll()
{
	QDictIterator<EditorData> it( _editorMap );
	while ( it.current() )
	{
		if ( ! it.current()->marks.isEmpty() )
		{
			updateContextStringForURL( it.current()->url );
		}
		++it;
	}
}

bool BookmarksPart::setBookmarksForURL( KParts::ReadOnlyPart * ro_part )
{
	if ( KTextEditor::MarkInterface * mi = dynamic_cast<KTextEditor::MarkInterface *>(ro_part) )
	{
		clearBookmarksForURL( ro_part );

		_settingMarks = true;

		if ( EditorData * data = _editorMap.find( ro_part->url().path() ) )
		{
			// we've seen this one before, apply stored bookmarks

			QValueListIterator< QPair<int,QStringList> > it = data->marks.begin();
			while ( it != data->marks.end() )
			{
				//kdDebug(0) << "Setting bookmark. Line: " << (*it).first << endl;
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

// Note: This method is only a convenience method to clear the bookmark marks,
// the way a hypothetical KTextEditor::MarkInterface::clearMarks( uint markType )
// would work.
bool BookmarksPart::clearBookmarksForURL( KParts::ReadOnlyPart * ro_part )
{
	if ( KTextEditor::MarkInterface * mi = dynamic_cast<KTextEditor::MarkInterface *>(ro_part) )
	{
		_settingMarks = true;

		QPtrList<KTextEditor::Mark> marks = mi->marks();
		QPtrListIterator<KTextEditor::Mark> it( marks );
		while ( it.current() )
		{
			if ( it.current()->type & KTextEditor::MarkInterface::markType01 )
			{
				mi->removeMark( it.current()->line, KTextEditor::MarkInterface::markType01 );
			}
			++it;
		}

		_settingMarks = false;

		// true == this is a MarkInterface
		return true;
	}
	return false;
}

EditorData * BookmarksPart::storeBookmarksForURL( KParts::ReadOnlyPart * ro_part )
{
	//kdDebug(0) << "BookmarksPart::storeBookmarksForURL()" << endl;
	
	if ( KTextEditor::MarkInterface * mi = dynamic_cast<KTextEditor::MarkInterface *>( ro_part ) )
	{
		EditorData * data = new EditorData;
		data->url = ro_part->url();

		// removing previous data for this url, if any
		if ( _editorMap.remove( data->url.path() ) )
		{
			//kdDebug(0) << "removed previous data" << endl;
		}

		QPtrList<KTextEditor::Mark> marks = mi->marks();
		QPtrListIterator<KTextEditor::Mark> it( marks );
		while ( it.current() )
		{
			if ( it.current()->type & KTextEditor::MarkInterface::markType01 )
			{
			    int line = it.current()->line;
				//kdDebug(0) << "Found bookmark. Line: " << line << endl;
				data->marks.append( qMakePair( line, QStringList() ) );
			}
			++it;
		}

		if ( ! data->marks.isEmpty() )
		{
			//kdDebug(0) << data->marks.count() << " bookmarks in " << data->url.path() << " - Keeping" << endl;

			_editorMap.insert( data->url.path(), data );
		}
		else
		{
			//kdDebug(0) << "No bookmarks in " << data->url.path() << " - Deleting" << endl;
			delete data;
			data = 0;
		}
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

bool BookmarksPart::partIsSane( KParts::ReadOnlyPart * ro_part )
{
	//kdDebug(0) << "BookmarksPart::partIsSane()" << endl;
	
	bool isSane = true;
	
	isSane = isSane && partController()->parts()->contains( ro_part);
	isSane = isSane && !ro_part->url().path().isEmpty();
	
	if ( ! isSane )
	{
		//kdDebug(0) << " ** Non-sane part encountered! ** " << endl;
	}
	
	return isSane;
}

#include "bookmarks_part.moc"
