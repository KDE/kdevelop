/*
    SPDX-FileCopyrightText: 2010-2011 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "oktetadocument.h"

// plugin
#include "oktetaplugin.h"
#include "oktetaview.h"
// Okteta
#include <Kasten/Okteta/ByteArrayViewProfileManager>
#include <Kasten/Okteta/ByteArrayViewProfileSynchronizer>
#include <Kasten/Okteta/ByteArrayRawFileSynchronizerFactory>
#include <Kasten/Okteta/ByteArrayDocument>
// Kasten
#include <Kasten/JobManager>
#include <Kasten/AbstractLoadJob>
#include <Kasten/AbstractSyncToRemoteJob>
#include <Kasten/AbstractSyncFromRemoteJob>
#include <Kasten/AbstractModelSynchronizer>
// KDevelop
#include <shell/core.h>
#include <shell/uicontroller.h>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
// Sublime
#include <sublime/mainwindow.h>
#include <sublime/view.h>
#include <sublime/area.h>
#include <sublime/controller.h>
// KF
#include <KMessageBox>
#include <KMessageBox_KDevCompat>
#include <KLocalizedString>
#include <KTextEditor/Cursor>
// Qt
#include <QApplication>
#include <QMimeType>

namespace KDevelop
{

OktetaDocument::OktetaDocument( const QUrl &url , ICore* core )
    : Sublime::UrlDocument( core->uiController()->controller(), url )
    , IDocument( core )
    , mPlugin( nullptr )
    , mByteArrayDocument( nullptr )
{
}

QUrl OktetaDocument::url() const { return Sublime::UrlDocument::url(); }

QMimeType OktetaDocument::mimeType() const
{
    return Sublime::UrlDocument::mimeType();
}

QIcon OktetaDocument::icon() const
{
    return Sublime::UrlDocument::icon();
}

KParts::Part* OktetaDocument::partForView( QWidget* ) const { return nullptr; }
KTextEditor::Document* OktetaDocument::textDocument() const { return nullptr; }
KTextEditor::Cursor OktetaDocument::cursorPosition() const { return KTextEditor::Cursor(); }

IDocument::DocumentState OktetaDocument::state() const
{
    return mByteArrayDocument ?
               ( mByteArrayDocument->synchronizer()->localSyncState() == Kasten::LocalHasChanges ?
                   IDocument::Modified :
                   IDocument::Clean ) :
               IDocument::Clean;
}


bool OktetaDocument::save( IDocument::DocumentSaveMode mode )
{
    if( mode & Discard )
        return true;

    if(  state() == IDocument::Clean )
        return false;

    Kasten::AbstractModelSynchronizer* synchronizer = mByteArrayDocument->synchronizer();

    Kasten::AbstractSyncToRemoteJob* syncJob = synchronizer->startSyncToRemote();
    const bool syncSucceeded = Kasten::JobManager::executeJob( syncJob );

    if( syncSucceeded )
    {
        notifySaved();
        notifyStateChanged();
    }

    return syncSucceeded;
}


void OktetaDocument::reload()
{
    Kasten::AbstractModelSynchronizer* synchronizer = mByteArrayDocument->synchronizer();

    Kasten::AbstractSyncFromRemoteJob* syncJob = synchronizer->startSyncFromRemote();
    const bool syncSucceeded = Kasten::JobManager::executeJob( syncJob );

    if( syncSucceeded )
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
            if( state() == IDocument::Modified )
            {
                // TODO: use Kasten::*Manager
                int code = KMessageBox::warningTwoActionsCancel(
                    qApp->activeWindow(),
                    i18n("The document \"%1\" has unsaved changes. Would you like to save them?", url().toLocalFile()),
                    i18nc("@title:window", "Close Document"), KStandardGuiItem::save(), KStandardGuiItem::cancel());

                if (code == KMessageBox::PrimaryAction) {
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
    const QList<Sublime::Area*>& allAreas =
        ICore::self()->uiController()->controller()->allAreas();
    for (Sublime::Area* area : allAreas ) {
        const QList<Sublime::View*> areaViews = area->views();
        for (Sublime::View* view : areaViews) {
            if (views().contains(view))
            {
                area->removeView(view);
                delete view;
            }
        }
    }

    // The document is deleted automatically when there are no views left

    return true;
}

bool OktetaDocument::isActive() const
{
    return Core::self()->uiControllerInternal()->activeSublimeWindow()->activeView()->document() == this;
}


void OktetaDocument::setCursorPosition( const KTextEditor::Cursor& ) {}
void OktetaDocument::setTextSelection( const KTextEditor::Range& ) {}

void OktetaDocument::activate( Sublime::View* /* view */, KParts::MainWindow* /* mainWindow */ )
{
    notifyActivated();
}

void OktetaDocument::setPlugin( OktetaPlugin* plugin )
{
    mPlugin = plugin;
}

Sublime::View* OktetaDocument::newView( Sublime::Document* /* document */ )
{
    if( mByteArrayDocument == nullptr )
    {
        auto* synchronizerFactory =
            new Kasten::ByteArrayRawFileSynchronizerFactory();
        Kasten::AbstractModelSynchronizer* synchronizer = synchronizerFactory->createSynchronizer();

        Kasten::AbstractLoadJob* loadJob = synchronizer->startLoad( url() );
        connect( loadJob, &Kasten::AbstractLoadJob::documentLoaded,
                 this, &OktetaDocument::onByteArrayDocumentLoaded );
        Kasten::JobManager::executeJob( loadJob );

        delete synchronizerFactory;
    }

    Kasten::ByteArrayViewProfileManager* const viewProfileManager = mPlugin->viewProfileManager();
    auto* viewProfileSynchronizer =
        new Kasten::ByteArrayViewProfileSynchronizer( viewProfileManager );
    viewProfileSynchronizer->setViewProfileId( viewProfileManager->defaultViewProfileId() );
    return new OktetaView( this, viewProfileSynchronizer );
}

bool OktetaDocument::closeDocument(bool silent)
{
    return close(silent ? Silent : Default);
}

void OktetaDocument::onByteArrayDocumentLoaded( Kasten::AbstractDocument* document )
{
    if( document )
    {
        mByteArrayDocument = static_cast<Kasten::ByteArrayDocument*>( document );
        connect( mByteArrayDocument->synchronizer(), &Kasten::AbstractModelSynchronizer::localSyncStateChanged,
                 this, &OktetaDocument::onByteArrayDocumentChanged );
    }
}

void OktetaDocument::onByteArrayDocumentChanged()
{
    notifyStateChanged();
}

OktetaDocument::~OktetaDocument()
{
    delete mByteArrayDocument;
}

}

#include "moc_oktetadocument.cpp"
