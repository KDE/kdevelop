/*
    This file is part of the KDevelop Okteta module, part of the KDE project.

    Copyright 2010 Friedrich W. H. Kossebau <kossebau@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#include "oktetadocument.h"

// plugin
#include "oktetaview.h"
// Okteta
#include <kasten/bytearrayrawfilesynchronizerfactory.h>
#include <kasten/bytearraydocument.h>
// Kasten
#include <jobmanager.h>
#include <abstractloadjob.h>
#include <abstractsyncwithremotejob.h>
#include <abstractmodelsynchronizer.h>
// KDevelop
#include <shell/core.h>
#include <shell/uicontroller.h>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>
// Sublime
#include <sublime/mainwindow.h>
#include <sublime/view.h>
#include <sublime/area.h>
#include <sublime/controller.h>
// KDE
#include <KMessageBox>
#include <KLocale>
// Qt
#include <QtGui/QApplication>


namespace KDevelop
{

OktetaDocument::OktetaDocument( const KUrl& url , ICore* core )
  : Sublime::UrlDocument( core->uiController()->controller(), url ),
    IDocument( core ),
    mByteArrayDocument( 0 ),
    mState( IDocument::Clean )
{
}

KUrl OktetaDocument::url() const { return UrlDocument::url(); }

KSharedPtr<KMimeType> OktetaDocument::mimeType() const { return KMimeType::mimeType( "audio/x-wav" ); }

KParts::Part* OktetaDocument::partForView( QWidget* ) const { return 0; }
KTextEditor::Document* OktetaDocument::textDocument() const { return 0; }

IDocument::DocumentState OktetaDocument::state() const { return mState; }


bool OktetaDocument::save( IDocument::DocumentSaveMode mode )
{
    if( mode & Discard )
        return true;

    if( mState == IDocument::Clean )
        return false;

    Kasten::AbstractModelSynchronizer* synchronizer = mByteArrayDocument->synchronizer();

    Kasten::AbstractSyncWithRemoteJob *syncJob =
        synchronizer->startSyncWithRemote( url(), Kasten::AbstractModelSynchronizer::ReplaceRemote );
    const bool syncSucceeded = Kasten::JobManager::executeJob( syncJob, qApp->activeWindow() );

    if( syncSucceeded )
    {
        mState = IDocument::Clean;
        notifySaved();
        notifyStateChanged();
    }

    return syncSucceeded;
}


void OktetaDocument::reload()
{
    mState = IDocument::Clean;
    notifyStateChanged();
}

bool OktetaDocument::close( IDocument::DocumentSaveMode mode )
{
    bool isCanceled = false;
    if( !(mode & Discard) )
    {
        if (mode & Silent)
        {
            if (!save(mode))
                isCanceled = true;

        }
        else
        {
            if (state() == IDocument::Modified)
            {
                int code = KMessageBox::warningYesNoCancel(
                    qApp->activeWindow(),
                    i18n("The document \"%1\" has unsaved changes. Would you like to save them?", url().toLocalFile()),
                    i18n("Close Document"));

                if (code == KMessageBox::Yes) {
                    if (!save(mode))
                isCanceled = true;

                } else if (code == KMessageBox::Cancel)
                    isCanceled = true;

            }
            else if( state() == IDocument::DirtyAndModified )
            {
                if( !save(mode) )
                    isCanceled = true;
            }
        }
    }

    if( isCanceled )
        return false;

    //close all views and then delete ourself
    ///@todo test this
    foreach( Sublime::Area *area,
        ICore::self()->uiController()->controller()->allAreas() )
    {
        QList<Sublime::View*> areaViews = area->views();
        foreach( Sublime::View* view, areaViews )
        {
            if (views().contains(view))
            {
                area->removeView(view);
                delete view;
            }
        }
    }

    ICore::self()->documentController()->notifyDocumentClosed( this );

    // Here we go...
    deleteLater();

    return true;
}

bool OktetaDocument::isActive() const
{
    return Core::self()->uiControllerInternal()->activeSublimeWindow()->activeView()->document() == this;
}


void OktetaDocument::setCursorPosition( const KTextEditor::Cursor& ) {}
void OktetaDocument::setTextSelection( const KTextEditor::Range& ) {}

void OktetaDocument::activate( Sublime::View* view, KParts::MainWindow* mainWindow )
{
//     KParts::Part *part = partForView(activeView->widget());
//     if (Core::self()->partController()->activePart() != part)
//         Core::self()->partController()->setActivePart(part);

    notifyActivated();
}

void OktetaDocument::setPlugin( OktetaPlugin* plugin )
{
    mPlugin = plugin;
}

Sublime::View* OktetaDocument::newView( Sublime::Document* document )
{
    kDebug()<<document;
    if( mByteArrayDocument == 0 )
    {
        Kasten::ByteArrayRawFileSynchronizerFactory* synchronizerFactory =
            new Kasten::ByteArrayRawFileSynchronizerFactory();
        Kasten::AbstractModelSynchronizer* synchronizer = synchronizerFactory->createSynchronizer();

        Kasten::AbstractLoadJob* loadJob = synchronizer->startLoad( url() );
        connect( loadJob, SIGNAL(documentLoaded( Kasten::AbstractDocument* )),
                 SLOT(onByteArrayDocumentLoaded( Kasten::AbstractDocument* )) );
        Kasten::JobManager::executeJob( loadJob, qApp->activeWindow() );

        delete synchronizerFactory;
    }

    return new OktetaView( this );
}

KTextEditor::Cursor OktetaDocument::cursorPosition() const
{
    return KTextEditor::Cursor();
}

bool OktetaDocument::closeDocument()
{
    return close();
}

void OktetaDocument::onByteArrayDocumentLoaded( Kasten::AbstractDocument* document )
{
kDebug()<<document;
    if( document )
    {
        mByteArrayDocument = static_cast<Kasten::ByteArrayDocument*>( document );
        connect( mByteArrayDocument, SIGNAL(localSyncStateChanged( Kasten::LocalSyncState )),
                 SLOT(onByteArrayDocumentChanged( Kasten::LocalSyncState )) );
    }
}

void OktetaDocument::onByteArrayDocumentChanged( Kasten::LocalSyncState newState )
{
    mState = ( newState == Kasten::LocalHasChanges ) ?
        IDocument::Modified :
        IDocument::Clean;
    notifyStateChanged();
}

OktetaDocument::~OktetaDocument()
{
    delete mByteArrayDocument;
}

}
