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


#include <q3vbox.h>
#include <QTimer>
#include <qtextstream.h>
#include <QFile>
//Added by qt3to4:
#include <Q3PtrList>

#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdevgenericfactory.h>
#include <ktexteditor/markinterface.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/document.h>
#include <kaction.h>
#include <kdialogbase.h>

#include <kdevdocumentcontroller.h>
#include <kdevcore.h>
#include <kdevmainwindow.h>
#include "domutil.h"

#include "bookmarks_widget.h"
#include "bookmarks_part.h"
#include "bookmarks_settings.h"
#include "bookmarks_config.h"

#include <configwidgetproxy.h>
#include <kdevplugininfo.h>

#define BOOKMARKSETTINGSPAGE 1

typedef KDevGenericFactory<BookmarksPart> BookmarksFactory;
static const KDevPluginInfo data("kdevbookmarks");
K_EXPORT_COMPONENT_FACTORY( libkdevbookmarks, BookmarksFactory( data ) )

BookmarksPart::BookmarksPart(QObject *parent, const char *name, const QStringList& )
    : KDevPlugin(&data, parent, name ? name : "BookmarksPart" )
{
    setInstance(BookmarksFactory::instance());

    _widget = new BookmarksWidget(this);

    _widget->setCaption(i18n("Bookmarks"));
    _widget->setIcon(SmallIcon( info()->icon() ));

    _marksChangeTimer = new QTimer( this );

    _widget->setWhatsThis( i18n("<b>Bookmarks</b><p>"
            "The bookmark viewer shows all the source bookmarks in the project."));

    mainWindow()->embedSelectView(_widget, i18n("Bookmarks"), i18n("Source bookmarks"));

    _editorMap.setAutoDelete( true );
    _settingMarks = false;

    connect( documentController(), SIGNAL( partAdded( KParts::Part * ) ), this, SLOT( partAdded( KParts::Part * ) ) );

    _configProxy = new ConfigWidgetProxy( core() );
    _configProxy->createProjectConfigPage( i18n("Bookmarks"), BOOKMARKSETTINGSPAGE, info()->icon() );
    connect( _configProxy, SIGNAL(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int )),
        this, SLOT(insertConfigWidget(const KDialogBase*, QWidget*, unsigned int )) );

    connect( _widget, SIGNAL( removeAllBookmarksForURL( const KUrl & ) ),
        this, SLOT( removeAllBookmarksForURL( const KUrl & ) ) );
    connect( _widget, SIGNAL( removeBookmarkForURL( const KUrl &, int ) ),
        this, SLOT( removeBookmarkForURL( const KUrl &, int ) ) );

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
    delete _configProxy;
}

void BookmarksPart::partAdded( KParts::Part * part )
{
    //kDebug(0) << "BookmarksPart::partAdded()" << endl;

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
            connect( doc, SIGNAL( marksChanged(KTextEditor::Document*) ), this, SLOT( marksEvent() ) );

            // workaround for a katepart oddity where it drops all bookmarks on 'reload'
            connect( doc, SIGNAL( completed() ), this, SLOT( reload() ) );
        }
    }
}

void BookmarksPart::reload()
{
    //kDebug(0) << "BookmarksPart::reload()" << endl;

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
    //kDebug(0) << "BookmarksPart::marksEvent()" << endl;

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
    //kDebug(0) << "BookmarksPart::marksChanged()" << endl;

    Q3ValueListIterator<KParts::ReadOnlyPart*> it = _dirtyParts.begin();
    while ( it != _dirtyParts.end() )
    {
        KParts::ReadOnlyPart * ro_part = *it;
        if ( partIsSane( ro_part ) )
        {
            if ( dynamic_cast<KTextEditor::MarkInterface*>( ro_part ) )
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
        }
        ++it;
    }
    _dirtyParts.clear();
}

void BookmarksPart::restorePartialProjectSession( const QDomElement * el )
{
    //kDebug(0) << "BookmarksPart::restorePartialProjectSession()" << endl;

    if ( ! el ) return;

    QDomElement bookmarksList = el->namedItem( "bookmarks" ).toElement();
    if ( bookmarksList.isNull() ) return;

    QDomElement bookmark = bookmarksList.firstChild().toElement();
    while ( ! bookmark.isNull() )
    {
        QString path = bookmark.attribute( "url" );
        if ( path != QString() )
        {
            EditorData * data = new EditorData;
            data->url.setPath( path );

            QDomElement mark = bookmark.firstChild().toElement();
            while ( ! mark.isNull() )
            {
                QString line = mark.attribute( "line" );
                if ( line != QString() )
                {
                    data->marks.append( qMakePair( line.toInt(), QString() ) );
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
    //kDebug(0) << "BookmarksPart::savePartialProjectSession()" << endl;

    if ( ! el ) return;

    QDomDocument domDoc = el->ownerDocument();
    if ( domDoc.isNull() ) return;

    QDomElement bookmarksList = domDoc.createElement( "bookmarks" );

    Q3DictIterator<EditorData> it( _editorMap );
    while ( it.current() )
    {
        QDomElement bookmark = domDoc.createElement( "bookmark" );
        bookmark.setAttribute( "url", it.current()->url.path() );
        bookmarksList.appendChild( bookmark );

        Q3ValueListIterator< QPair<int,QString> > it2 = it.current()->marks.begin();
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

void BookmarksPart::removeAllBookmarksForURL( KUrl const & url )
{
    //kDebug(0) << "BookmarksPart::removeAllBookmarksForURL()" << endl;

    _editorMap.remove( url.path() );

    setBookmarksForURL( partForURL( url ) );
    _widget->removeURL( url );
}

void BookmarksPart::removeBookmarkForURL( KUrl const & url, int line )
{
    //kDebug(0) << "BookmarksPart::removeBookmarkForURL()" << endl;

    if ( EditorData * data = _editorMap.find( url.path() ) )
    {
        Q3ValueListIterator< QPair<int,QString> > it = data->marks.begin();
        while ( it != data->marks.end() )
        {
            if ( (*it).first == line )
            {
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

    Q3ValueListIterator< QPair<int,QString> > it = data->marks.begin();
    while ( it != data->marks.end() )
    {
        (*it).second = ed->textLine( (*it).first );
        ++it;
    }
}

void BookmarksPart::updateContextStringForURL( KUrl const & url )
{
    updateContextStringForURL( partForURL( url ) );
}

void BookmarksPart::updateContextStringForAll()
{
    Q3DictIterator<EditorData> it( _editorMap );
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

            Q3ValueListIterator< QPair<int,QString> > it = data->marks.begin();
            while ( it != data->marks.end() )
            {
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

        Q3PtrList<KTextEditor::Mark> marks = mi->marks();
        Q3PtrListIterator<KTextEditor::Mark> it( marks );
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
    //kDebug(0) << "BookmarksPart::storeBookmarksForURL()" << endl;

    if ( KTextEditor::MarkInterface * mi = dynamic_cast<KTextEditor::MarkInterface *>( ro_part ) )
    {
        EditorData * data = new EditorData;
        data->url = ro_part->url();

        // removing previous data for this url, if any
        _editorMap.remove( data->url.path() );

        Q3PtrList<KTextEditor::Mark> marks = mi->marks();
        Q3PtrListIterator<KTextEditor::Mark> it( marks );
        while ( it.current() )
        {
            if ( it.current()->type & KTextEditor::MarkInterface::markType01 )
            {
                int line = it.current()->line;
                data->marks.append( qMakePair( line, QString() ) );
            }
            ++it;
        }

        if ( ! data->marks.isEmpty() )
        {
            _editorMap.insert( data->url.path(), data );
        }
        else
        {
            delete data;
            data = 0;
        }
        return data;
    }
    return 0;
}

void BookmarksPart::setBookmarksForAllURLs()
{
    if( const Q3PtrList<KParts::Part> * partlist = documentController()->parts() )
    {
        Q3PtrListIterator<KParts::Part> it( *partlist );
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
    if( const Q3PtrList<KParts::Part> * partlist = documentController()->parts() )
    {
        Q3PtrListIterator<KParts::Part> it( *partlist );
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
KParts::ReadOnlyPart * BookmarksPart::partForURL( KUrl const & url )
{
    Q3PtrListIterator<KParts::Part> it( *documentController()->parts() );
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
    return ( ro_part != 0 ) &&
            documentController()->parts()->contains( ro_part) &&
            !ro_part->url().path().isEmpty();
}

void BookmarksPart::insertConfigWidget( const KDialogBase * dlg, QWidget * page, unsigned int pagenumber )
{
    kDebug() << k_funcinfo << endl;

    if ( pagenumber == BOOKMARKSETTINGSPAGE )
    {
        BookmarkSettings * w = new BookmarkSettings( this, page );
        connect( dlg, SIGNAL(okClicked()), w, SLOT(slotAccept()) );
    }
}

////////////////////////////////////////////

QStringList BookmarksPart::getContextFromStream( QTextStream & istream, unsigned int line, unsigned int context )
{
    kDebug() << k_funcinfo << endl;

    int startline = context > line ? 0 : line - context;
    int endline = line + context;

    int n = 0;
    QStringList list;
    while ( !istream.atEnd() )
    {
        QString templine = istream.readLine();
        if ( (n >= startline) && ( n <= endline ) )
        {
            list << templine;
        }
        n++;
    }

    // maybe pad empty lines to the tail
    while( n < endline )
    {
        list.append( " " );
        n++;
    }

    // maybe pad empty lines to the head
    while( list.count() < ( context * 2 + 1) )
    {
        list.prepend( " " );
    }

    return list;
}

QStringList BookmarksPart::getContext( KUrl const & url, unsigned int line, unsigned int context )
{
    // if the file is open - get the line from the editor buffer
    if ( KTextEditor::EditInterface * ei = dynamic_cast<KTextEditor::EditInterface*>( partForURL( url ) ) )
    {
        kDebug() << "the file is open - get the line from the editor buffer" << endl;

        QString ibuffer = ei->text();
        QTextStream istream( &ibuffer, QIODevice::ReadOnly );
        return getContextFromStream( istream, line, context );
    }
    else if ( url.isLocalFile() ) // else the file is not open - get the line from the file on disk
    {
        kDebug() << "the file is not open - get the line from the file on disk" << endl;

        QFile file( url.path() );
        QString buffer;

        if ( file.open( QIODevice::ReadOnly ) )
        {
            QTextStream istream( &file );
            return getContextFromStream( istream, line, context );
        }
    }
    return QStringList( i18n("Could not find file") );
}

BookmarksConfig * BookmarksPart::config( )
{
    return _config;
}

#include "bookmarks_part.moc"

// kate: space-indent off; indent-width 4; tab-width 4; show-tabs off;
