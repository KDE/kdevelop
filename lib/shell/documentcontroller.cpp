/* This file is part of the KDE project
Copyright (C) 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
Copyright (C) 2002 Bernd Gehrmann <bernd@kdevelop.org>
Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>
Copyright (C) 2003 Hamish Rodda <rodda@kde.org>
Copyright (C) 2003 Harald Fernengel <harry@kdevelop.org>
Copyright (C) 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>
Copyright (C) 2005 Adam Treat <treat@kde.org>
Copyright (C) 2004-2007 Alexander Dymo <adymo@kdevelop.org>
Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/
#include "documentcontroller.h"

#include <QFileInfo>
#include <QtDBus/QtDBus>

#include <kio/netaccess.h>

#include <sublime/area.h>
#include <sublime/view.h>

#include "core.h"
#include "mainwindow.h"
#include "textdocument.h"
#include "uicontroller.h"
#include "partcontroller.h"

/*

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <QMap>
#include <QFile>
#include <QDebug>
#include <QLabel>
#include <QLayout>
#include <QCheckBox>
#include <QRadioButton>

#include <krun.h>
#include <kmenu.h>
#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kmimetype.h>
#include <kmimetypetrader.h>
#include <klineedit.h>
#include <kshortcut.h>
#include <kdirwatch.h>
#include <khtml_part.h>
#include <kdeversion.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kxmlguiwindow.h>
#include <kdialog.h>
#include <kcompletion.h>
#include <kiconloader.h>
#include <kxmlguifactory.h>
#include <kservicetypetrader.h>
#include <ksqueezedtextlabel.h>
#include <kencodingfiledialog.h>
#include <krecentfilesaction.h>
#include <ktoolbarpopupaction.h>
#include <kstandardaction.h>

#include <kio/netaccess.h>

#include <kparts/part.h>
#include <kparts/factory.h>
#include <kparts/partmanager.h>
#include <kparts/browserextension.h>

#include <ktexteditor/view.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/document.h>

#include "kdevconfig.h"
#include "kdevcontext.h"
#include "kdevproject.h"
#include "kdevmainwindow.h"
#include "ui_mimewarningdialog.h"
#include "kdevpartcontroller.h"
#include "kdevlanguagesupport.h"
#include "kdevbackgroundparser.h"
*/

namespace KDevelop
{

struct DocumentControllerPrivate {
    QString presetEncoding;

    // used to map urls to open docs
    QHash< KUrl, IDocument* > documents;

    QHash< QString, IDocumentFactory* > factories;

    struct HistoryEntry
    {
        HistoryEntry() {}
        HistoryEntry( const KUrl & u, const KTextEditor::Cursor& cursor );

        KUrl url;
        KTextEditor::Cursor cursor;
        int id;
    };

    QList<HistoryEntry> backHistory;
    QList<HistoryEntry> forwardHistory;
    bool isJumping;

/*    HistoryEntry createHistoryEntry();
    void addHistoryEntry();
    void jumpTo( const HistoryEntry & );*/
};


DocumentController::DocumentController( QObject *parent )
        : IDocumentController( parent )
{
    d = new DocumentControllerPrivate();
    QDBusConnection::sessionBus().registerObject( "/org/kdevelop/DocumentController",
        this, QDBusConnection::ExportScriptableSlots );
}

DocumentController::~DocumentController()
{
    delete d;
}

void DocumentController::setEncoding( const QString &encoding )
{
    d->presetEncoding = encoding;
}

QString KDevelop::DocumentController::encoding() const
{
    return d->presetEncoding;
}

IDocument* DocumentController::openDocument( const KUrl & inputUrl,
        const KTextEditor::Cursor& /*cursor*/,
        DocumentActivation activate )
{
    UiController *uiController = Core::self()->uiControllerInternal();
    Sublime::Area *area = uiController->activeArea();
    if (!area)
        return 0;

    KUrl url = inputUrl;

    //get a part document
    if (!d->documents.contains(url))
    {
        //make sure the URL exists
        if ( !url.isValid() || !KIO::NetAccess::exists( url, false, 0 ) )
        {
            kDebug( 9000 ) << "cannot find URL: " << url.url() << endl;
            return 0;
        }

        // clean it and resolve possible symlink
        url.cleanPath( KUrl::SimplifyDirSeparators );
        if ( url.isLocalFile() )
        {
            QString path = QFileInfo( url.path() ).canonicalFilePath();
            if ( !path.isEmpty() )
                url.setPath( path );
        }

        KMimeType::Ptr mimeType = KMimeType::findByUrl( url );

        // is the URL pointing to a directory?
        if ( mimeType->is( "inode/directory" ) )
        {
            kDebug( 9000 ) << "cannot open directory: " << url.url() << endl;
            return 0;
        }

        if( d->factories.contains( mimeType->name() ) )
        {
            IDocument* idoc = d->factories[mimeType->name()]->create(url, Core::self());
            if( idoc )
            {
                 d->documents[url] = idoc;
            }
        }
        if ( !d->documents.contains(url) && Core::self()->partController()->isTextType(mimeType))
            d->documents[url] = new TextDocument(url, Core::self());
        else if( !d->documents.contains(url) )
            d->documents[url] = new PartDocument(url, Core::self());
        if( d->documents.contains(url) )
            emit documentLoaded( d->documents[url] );
    }
    IDocument *doc = d->documents[url];

    Sublime::Document *sdoc = dynamic_cast<Sublime::Document*>(doc);
    if( !sdoc )
    {
        d->documents.remove(url);
        delete doc;
        return 0;
    }
    //find a view if there's one already opened in this area
    Sublime::View *partView = 0;
    foreach (Sublime::View *view, sdoc->views())
    {
        if (area->views().contains(view))
        {
            partView = view;
            break;
        }
    }
    if (!partView)
    {
        //no view currently shown for this url
        partView = sdoc->createView();

        //add view to the area
        area->addView(partView, uiController->activeSublimeWindow()->activeView());
    }
    if (activate == IDocumentController::ActivateOnOpen)
    {
        uiController->activeSublimeWindow()->activateView(partView);
        Core::self()->partController()->setActivePart(doc->partForView(partView->widget()), partView->widget());
        emit documentActivated( doc );
    }

    return doc;
}

IDocument * DocumentController::documentForUrl( const KUrl & url ) const
{
    if ( d->documents.contains( url ) )
        return d->documents.value( url );

    return 0;
}

QList<IDocument*> DocumentController::openDocuments() const
{
    QList<IDocument*> opened;
    foreach (IDocument *doc, d->documents.values())
    {
        Sublime::Document *sdoc = dynamic_cast<Sublime::Document*>(doc);
        if( !sdoc )
        {
            continue;
        }
        if (!sdoc->views().isEmpty())
            opened << doc;
    }
    return opened;
}

void DocumentController::activateDocument( IDocument * document )
{
    openDocument(document->url());
}

IDocument* DocumentController::activeDocument() const
{
    UiController *uiController = Core::self()->uiControllerInternal();
    if( !uiController->activeSublimeWindow() || !uiController->activeSublimeWindow()->activeView() ) return 0;
    return dynamic_cast<IDocument*>(uiController->activeSublimeWindow()->activeView()->document());
}

void DocumentController::registerDocumentForMimetype( const QString& mimetype,
                                        KDevelop::IDocumentFactory* factory )
{
    if( !d->factories.contains( mimetype ) )
        d->factories[mimetype] = factory;
}

}

#include "documentcontroller.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
