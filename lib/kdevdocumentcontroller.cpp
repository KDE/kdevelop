/* This file is part of the KDE project
Copyright (C) 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>

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
#include "kdevdocumentcontroller.h"

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
#include <QtDBus/QtDBus>

#include <krun.h>
#include <kmenu.h>
#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>
#include <kmimetype.h>
#include <kmimetypetrader.h>
#include <klineedit.h>
#include <kshortcut.h>
#include <kdirwatch.h>
#include <kstatusbar.h>
#include <khtml_part.h>
#include <kdeversion.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kmainwindow.h>
#include <kdialog.h>
#include <kcompletion.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kxmlguifactory.h>
#include <kservicetypetrader.h>
#include <ksqueezedtextlabel.h>
#include <kencodingfiledialog.h>
#include <krecentfilesaction.h>
#include <ktoolbarpopupaction.h>
#include <kstdaction.h>
#include <kseparatoraction.h>

#include <kio/netaccess.h>

#include <kparts/part.h>
#include <kparts/factory.h>
#include <kparts/partmanager.h>
#include <kparts/browserextension.h>

#include <ktexteditor/view.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/document.h>

#include "kdevcore.h"
#include "kdevconfig.h"
#include "kdevproject.h"
#include "kdevpartcontroller.h"
#include "kdevmainwindow.h"
#include "mimewarningdialog.h"
#include "kdevlanguagesupport.h"

using namespace MainWindowUtils;

KDevDocumentController::KDevDocumentController( QObject *parent )
        : QObject( parent )
{
    QDBus::sessionBus().registerObject( "/org/kdevelop/DocumentController",
                                        this, QDBusConnection::ExportSlots );
    m_isJumping = false;

    m_openNextAsText = false;
}

KDevDocumentController::~KDevDocumentController()
{}

void KDevDocumentController::cleanUp()
{
    querySaveDocuments();
    blockSignals( true ); //No more signals as we're ready to close
    closeAllDocuments();
}

void KDevDocumentController::setEncoding( const QString &encoding )
{
    m_presetEncoding = encoding;
}

KDevDocument* KDevDocumentController::editDocument( const KUrl & inputUrl,
        const KTextEditor::Cursor& cursor,
        bool activate )
{
    KUrl url = inputUrl;

    // is it already open?
    // (Try this once before verifying the URL, we could be dealing with a
    // document that no longer exists on disc)
    if ( KDevDocument * existingDoc = documentForUrl( url ) )
    {
        if ( !existingDoc->isInitialized() )
        {
            KParts::ReadWritePart *rw = readWrite( existingDoc->part() );
            rw->openURL( url );
        }

        addHistoryEntry();
        activateDocument( existingDoc );
        setCursorPosition( existingDoc->part(), cursor );
        return existingDoc;
    }

    // Make sure the URL exists
    if ( !url.isValid() || !KIO::NetAccess::exists( url, false, 0 ) )
    {
        kDebug( 9000 ) << "cannot find URL: " << url.url() << endl;
        return 0L;
    }

    // For the splash screen... FIXME this isn't optimized so disable.
    //emit openingDocument( "Opening: " + url.fileName() );
    //kDebug( 9000 ) << "Opening: " + url.fileName() << endl;

    // clean it and resolve possible symlink
    url.cleanPath( KUrl::SimplifyDirSeparators );
    if ( url.isLocalFile() )
    {
        QString path = QFileInfo( url.path() ).canonicalFilePath();
        if ( !path.isEmpty() )
            url.setPath( path );
    }

    KMimeType::Ptr mimeType = KMimeType::findByURL( url );
    // kDebug( 9000 ) << "mimeType = " << mimeType->name() << endl;

    // is the URL pointing to a directory?
    if ( mimeType->is( "inode/directory" ) )
    {
        kDebug( 9000 ) << "cannot open directory: " << url.url() << endl;
        return 0L;
    }

    KParts::Part * part = 0;

    if ( !m_presetEncoding.isNull() || KDevCore::partController() ->isTextType( mimeType ) )
        part = KDevCore::partController() ->createTextPart( url, m_presetEncoding, activate );
    else
        part = KDevCore::partController() ->createPart( url );

    /*The open as dialog asks for text right now*/
    if ( !part )
        if ( openAsDialog( url, mimeType ) )
            part = KDevCore::partController() ->createTextPart( url, m_presetEncoding, activate );
        else
            return 0;

    addHistoryEntry();

    // we can have ended up with a texteditor,
    // in which case need to treat it as such
    KDevDocument *document = 0;
    if ( qobject_cast<KTextEditor::Document*>( part ) )
        document = integratePart( part, activate );
    else
        document = integratePart( part );

    document->setUrl( url );
    document->setMimeType( mimeType );
    m_url2Document.insert( url, document );

    //FIXME by moving all of this out of documentcontroller preferably
    //     m_openRecentAction->addUrl( url );
    //     m_openRecentAction->saveEntries( KDevConfig::localProject(),
    //                                      "RecentDocuments" );

    if ( activate )
        setCursorPosition( part, cursor );

    emit documentLoaded( document );

    return document;
}

bool KDevDocumentController::saveAllDocuments()
{
    return saveDocuments( openDocuments() );
}

void KDevDocumentController::reloadAllDocuments()
{
    reloadDocuments( openDocuments() );
}

bool KDevDocumentController::closeAllDocuments()
{
    return closeDocuments( openDocuments() );
}

KDevDocument* KDevDocumentController::showDocumentation( const KUrl &url, bool newWin )
{
    Q_UNUSED( url );
    Q_UNUSED( newWin );
    //FIXME Port to new documentcontroller API
    return 0;
}

KDevDocument * KDevDocumentController::documentForUrl( const KUrl & url ) const
{
    if ( m_url2Document.contains( url ) )
        return m_url2Document.value( url );

    return 0L;
}

QList<KDevDocument*> KDevDocumentController::openDocuments() const
{
    return m_partHash.values();
}

bool KDevDocumentController::saveDocument( KDevDocument* document, bool force )
{
    KParts::ReadWritePart * part = readWrite( document->part() );
    if ( !part )
        return true;

    switch ( document->state() )
    {
    case KDevDocument::Clean:
        if ( !force )
        {
            return true;
        }
        kDebug( 9000 ) << "Forced save" << endl;
        break;

    case KDevDocument::Modified:
        kDebug( 9000 ) << "Normal save" << endl;
        break;

    case KDevDocument::Dirty:
    case KDevDocument::DirtyAndModified:
        {
            int code = KMessageBox::warningYesNoCancel(
                           KDevCore::mainWindow(),
                           i18n( "The file \"%1\" is modified on disk.\n\nAre "
                                 "you sure you want to overwrite it? (External "
                                 "changes will be lost.)", part->url().path() ),
                           i18n( "Document Externally Modified" ) );
            if ( code == KMessageBox::Yes )
            {
                kDebug( 9000 ) << "Dirty save!!" << endl;
            }
            else if ( code == KMessageBox::No )
            {
                return true;
            }
            else
            {
                // a 'false' return means to interrupt the process that
                // caused the save
                return false;
            }
        }
        break;

    default:
        break;
    }

    if ( part->save() )
    {
        emit documentStateChanged( document );
        emit documentSaved( document );
    }

    return true;
}

bool KDevDocumentController::saveDocuments( const QList<KDevDocument*> & list )
{
    foreach ( KDevDocument * document, list )
    if ( saveDocument( document ) == false )
        return false; //user cancelled

    return true;
}

void KDevDocumentController::reloadDocument( KDevDocument* document )
{
    KParts::ReadWritePart * part = readWrite( document->part() );
    if ( part )
    {
        if ( part->isModified() )
        {
            if ( KMessageBox::warningYesNo( KDevCore::mainWindow(),
                                            i18n( "The file \"%1\" is modified "
                                                  "in memory. Are you sure you "
                                                  "want to reload it? (Local "
                                                  "changes will be lost.)", part->url().path() ),
                                            i18n( "Document is Modified" ) ) == KMessageBox::Yes )
            {
                part->setModified( false );
            }
            else
            {
                return ;
            }
        }

        QList<KTextEditor::Cursor> cursors;
        foreach ( KTextEditor::View * view, document->textDocument() ->textViews() )
        cursors << view->cursorPosition();

        part->openURL( part->url() );

        emit documentStateChanged( document );

        int i = 0;
        foreach ( KTextEditor::View * view, document->textDocument() ->textViews() )
        view->setCursorPosition( cursors[ i++ ] );
    }
}

void KDevDocumentController::reloadDocuments( const QList<KDevDocument*> & list )
{
    QList<KDevDocument*>::ConstIterator it = list.begin();
    for ( ; it != list.end(); ++it )
    {
        //FIXME return a bool?
        reloadDocument( *it );
    }
}

bool KDevDocumentController::closeDocument( KDevDocument* document )
{
    if ( !document )
        return false;

    if ( KParts::ReadWritePart * rw = readWrite( document->part() ) )
    {
        if ( ! rw->closeURL() )
            return false;
    }

    KDevCore::mainWindow() ->guiFactory() ->removeClient( document->part() );

    emit documentClosed( document );
    removeDocument( document );

    return true;
}

bool KDevDocumentController::closeDocuments( const QList<KDevDocument*>& list )
{
    QList<KDevDocument*>::ConstIterator it = list.begin();
    for ( ; it != list.end(); ++it )
    {
        if ( !closeDocument( *it ) )
        {
            return false;
        }
    }
    return true;
}

bool KDevDocumentController::closeAllOthers( KDevDocument* document )
{
    QList<KParts::Part*> docs = KDevCore::partController() ->parts();
    QList<KParts::Part*>::ConstIterator it = docs.begin();
    for ( ; it != docs.end(); ++it )
    {
        KDevDocument * doc = documentForPart( *it );
        if ( doc != document )
        {
            if ( !closeDocument( doc ) )
            {
                return false;
            }
        }
    }
    return true;
}

void KDevDocumentController::activateDocument( KDevDocument * document )
{
    Q_ASSERT( document );

    if ( document->isActive() )
        return ;

    setActiveDocument( document );

    QWidget * widget = document->part() ->widget();
    if ( widget )
    {
        if ( !KDevCore::mainWindow() ->containsWidget( widget ) )
        {
            KParts::ReadOnlyPart * ro = readOnly( document->part() );
            if ( !ro )
                return ;

            KDevCore::mainWindow() ->embedPartView( widget,
                                                    ro->url().fileName(),
                                                    ro->url().url() );
        }

        KDevCore::mainWindow() ->setCurrentWidget( widget );
        widget->show();
        widget->setFocus();
    }
}

KDevDocument* KDevDocumentController::activeDocument() const
{
    if ( !KDevCore::partController() ->activePart() )
        return 0L;

    return documentForPart( KDevCore::partController() ->activePart() );
}

KUrl KDevDocumentController::activeDocumentUrl() const
{
    if ( activeDocument() )
        return activeDocument() ->url();

    return KUrl();
}

bool KDevDocumentController::querySaveDocuments()
{
    //     return saveDocumentsDialog();
    return true;
}

void KDevDocumentController::saveActiveDocument()
{
    if ( activeReadWrite() )
        saveDocument( activeDocument() );
}

void KDevDocumentController::reloadActiveDocument()
{
    if ( activeReadWrite() )
        reloadDocument( activeDocument() );
}

void KDevDocumentController::closeActiveDocument()
{
    closeDocument( activeDocument() );
}

void KDevDocumentController::closeAllExceptActiveDocument()
{
    if ( activeReadOnly() )
        closeAllOthers( activeDocument() );
}

void KDevDocumentController::slotOpenDocument()
{
    KEncodingFileDialog::Result result =
        KEncodingFileDialog::getOpenURLsAndEncoding(
            QString::null, QString::null, QString::null,
            KDevCore::mainWindow(), QString::null );

    for ( KUrl::List::Iterator it = result.URLs.begin();
            it != result.URLs.end(); ++it )
    {
        m_presetEncoding = result.encoding;
        editDocument( *it );
    }
}

void KDevDocumentController::slotOpenRecent( const KUrl& url )
{
    editDocument( url );
    // stupid bugfix - don't allow an active item in the list
    m_openRecentAction->setCurrentItem( -1 );
}

void KDevDocumentController::slotBack()
{
    HistoryEntry thatEntry = m_backHistory.front();
    m_backHistory.pop_front();
    m_backAction->setEnabled( !m_backHistory.isEmpty() );

    HistoryEntry thisEntry = createHistoryEntry();
    if ( !thisEntry.url.isEmpty() )
    {
        m_forwardHistory.push_front( thisEntry );
        m_forwardAction->setEnabled( true );
    }

    jumpTo( thatEntry );
}

void KDevDocumentController::slotForward()
{
    HistoryEntry thatEntry = m_forwardHistory.front();
    m_forwardHistory.pop_front();
    m_forwardAction->setEnabled( !m_forwardHistory.isEmpty() );

    HistoryEntry thisEntry = createHistoryEntry();
    if ( !thisEntry.url.isEmpty() )
    {
        m_backHistory.push_front( thisEntry );
        m_backAction->setEnabled( true );
    }

    jumpTo( thatEntry );
}

void KDevDocumentController::slotBackAboutToShow()
{
    QMenu * popup = m_backAction->menu();
    popup->clear();

    if ( m_backHistory.isEmpty() )
        return ;

    int i = 0;
    QList<HistoryEntry>::ConstIterator it = m_backHistory.begin();
    while ( i < 10 && it != m_backHistory.end() )
    {
        popup->insertItem( ( *it ).url.fileName()
                           + QString( " (%1)" ).arg( ( *it ).cursor.line() + 1 ),
                           ( *it ).id );
        ++i;
        ++it;
    }
}

void KDevDocumentController::slotForwardAboutToShow( )
{
    QMenu * popup = m_forwardAction->menu();
    popup->clear();

    if ( m_forwardHistory.isEmpty() )
        return ;

    for ( int i = 0; i < 10 || i < m_forwardHistory.count(); ++i )
    {
        HistoryEntry entry = m_forwardHistory.at( i );
        popup->insertItem( entry.url.fileName()
                           + QString( " (%1)" ).arg( entry.cursor.line() + 1 ),
                           entry.id );
    }
}

void KDevDocumentController::slotBackPopupActivated( int id )
{
    QList<HistoryEntry>::iterator it = m_backHistory.begin();
    while ( it != m_backHistory.end() )
    {
        if ( ( *it ).id == id )
        {
            HistoryEntry entry = *it;
            m_backHistory.erase( m_backHistory.begin(), ++it );
            m_backAction->setEnabled( !m_backHistory.isEmpty() );

            HistoryEntry thisEntry = createHistoryEntry();
            if ( !thisEntry.url.isEmpty() )
            {
                m_forwardHistory.push_front( thisEntry );
                m_forwardAction->setEnabled( true );
            }

            jumpTo( entry );
            return ;
        }
        ++it;
    }
}

void KDevDocumentController::slotForwardPopupActivated( int id )
{
    QList<HistoryEntry>::iterator it = m_forwardHistory.begin();
    while ( it != m_forwardHistory.end() )
    {
        if ( ( *it ).id == id )
        {
            HistoryEntry entry = *it;
            m_forwardHistory.erase( m_forwardHistory.begin(), ++it );
            m_forwardAction->setEnabled( !m_forwardHistory.isEmpty() );

            HistoryEntry thisEntry = createHistoryEntry();
            if ( !thisEntry.url.isEmpty() )
            {
                m_backHistory.push_front( thisEntry );
                m_backAction->setEnabled( true );
            }

            jumpTo( entry );
            return ;
        }
        ++it;
    }
}

void KDevDocumentController::slotSwitchTo()
{
    //FIXME Port to new documentcontroller API
    //     QMap<QString, KParts::ReadOnlyPart*> parts_map;
    //     QStringList part_list;
    //     foreach ( KParts::Part * part, parts() )
    //     {
    //         kDebug( 9000 ) << "Part..." << endl;
    //         if ( part->inherits( "KParts::ReadOnlyPart" ) )
    //         {
    //             KParts::ReadOnlyPart * ro = readOnly( part );
    //             QString name = ro->url().fileName();
    //             part_list.append( name );
    //             parts_map[ name ] = ro;
    //             kDebug( 9000 ) << "Found part for URL "
    //             << ro->url().prettyUrl() << endl;
    //         }
    //     }
    //
    //     KDialog dialog;
    //     dialog.setCaption( i18n( "Switch To" ) );
    //     dialog.setButtons( KDialog::Ok | KDialog::Cancel );
    //     dialog.setDefaultButton( KDialog::Ok );
    //     QGridLayout *grid = new QGridLayout( &dialog, 2, 1, 10, 10 );
    //     KLineEdit *editbox = new KLineEdit( &dialog );
    //     grid->addWidget( new QLabel( i18n( "Switch to buffer:" ),
    //                                  &dialog ), 0, 0 );
    //     grid->addWidget( editbox, 1, 0 );
    //     editbox->completionObject() ->setItems( part_list );
    //     editbox->setFocus();
    //     int result = dialog.exec();
    //     if ( result == QDialog::Accepted )
    //     {
    //         if ( parts_map.contains( editbox->text() ) )
    //         {
    //             activatePart( parts_map[ editbox->text() ] );
    //         }
    //     }
}

void KDevDocumentController::slotUploadFinished()
{
    KDevDocument * document = documentForPart( qobject_cast<KParts::Part*>( const_cast<QObject*>( sender() ) ) );

    if ( document && documentUrlHasChanged( document ) )
        updateDocumentUrl( document );
}

void KDevDocumentController::updateMenuItems()
{
    //FIXME by moving all of this out of documentcontroller preferably

    bool hasWriteParts = false;
    bool hasReadOnlyParts = false;

    foreach ( KParts::Part * part, KDevCore::partController() ->parts() )
    {
        if ( part->inherits( "KParts::ReadWritePart" ) )
            hasWriteParts = true;
        if ( part->inherits( "KParts::ReadOnlyPart" ) )
            hasReadOnlyParts = true;
    }

    m_saveAllDocumentsAction->setEnabled( hasWriteParts );
    m_reloadAllDocumentsAction->setEnabled( hasWriteParts );
    m_closeWindowAction->setEnabled( hasReadOnlyParts );
    m_closeAllWindowsAction->setEnabled( hasReadOnlyParts );
    m_closeOtherWindowsAction->setEnabled( hasReadOnlyParts );

    m_backAction->setEnabled( !m_backHistory.isEmpty() );
}

void KDevDocumentController::modifiedOnDisk( KTextEditor::Document * d, bool isModified,
        KTextEditor::ModificationInterface::ModifiedOnDiskReason reason )
{
    Q_UNUSED( isModified );
    KDevDocument* doc = documentForPart( d );

    if ( !doc )
        return ;

    KParts::ReadWritePart *rw = readWrite( doc->part() );
    if ( !rw )
        return ;

    switch ( reason )
    {
    case KTextEditor::ModificationInterface::OnDiskUnmodified:
        if ( !rw->isModified() )
            doc->setState( KDevDocument::Clean );
        else
            doc->setState( KDevDocument::Dirty );
        break;
    case KTextEditor::ModificationInterface::OnDiskModified:
        doc->setState( KDevDocument::DirtyAndModified );
        break;
    case KTextEditor::ModificationInterface::OnDiskCreated:
        doc->setState( KDevDocument::DirtyAndModified );
        break;
    case KTextEditor::ModificationInterface::OnDiskDeleted:
        doc->setState( KDevDocument::DirtyAndModified );
        break;
    default:
        break;
    }
    emit documentStateChanged( doc );
}

void KDevDocumentController::newDocumentStatus( KTextEditor::Document * doc )
{
    KDevDocument * document = documentForPart( doc );

    KParts::ReadWritePart *rw = readWrite( document->part() );
    if ( !rw )
        return ;

    if ( !rw->isModified() )
        document->setState( KDevDocument::Clean );
    else
        document->setState( KDevDocument::Dirty );
    emit documentStateChanged( document );
}

void KDevDocumentController::setCursorPosition( KParts::Part *part,
        const KTextEditor::Cursor &cursor )
{
    if ( cursor.line() < 0 )
        return ;

    if ( !qobject_cast<KTextEditor::Document*>( part ) )
        return ;

    KTextEditor::View *view = qobject_cast<KTextEditor::View *>( part->widget() );

    KTextEditor::Cursor c = cursor;
    if ( c.column() == 1 )
        c.setColumn( 0 );

    if ( view )
        view->setCursorPosition( c );
}

bool KDevDocumentController::openAsDialog( const KUrl &url, KMimeType::Ptr mimeType )
{
    MimeWarningDialog dialog;
    dialog.text2->setText( QString( "<qt><b>%1</b></qt>" ).arg( url.path() ) );
    dialog.text3->setText( dialog.text3->text().arg( mimeType->name() ) );

    if ( dialog.exec() == QDialog::Accepted )
    {
        if ( dialog.open_with_kde->isChecked() )
        {
            KRun::runUrl( url, mimeType->name(), 0 );
            return false; //FIXME
        }
        else if ( dialog.always_open_as_text->isChecked() )
        {
            KConfig * config = KDevConfig::standard();
            config->setGroup( "General" );
            QStringList textTypesList = config->readEntry( "TextTypes",
                                        QStringList() );
            textTypesList << mimeType->name();
            config->writeEntry( "TextTypes", textTypesList );
            return true;
        }
    }
    return false;
}

KDevDocument * KDevDocumentController::addDocument( KParts::Part * part, bool setActive )
{
    KDevDocument * document =
        new KDevDocument( static_cast<KParts::ReadOnlyPart*>( part ), this );
    m_partHash.insert( static_cast<KParts::ReadOnlyPart*>( part ), document );
    KDevCore::partController() ->addPart( part, setActive );
    //     updateDocumentUrl( document );
    //     updateMenuItems();

    return document;
}

void KDevDocumentController::removeDocument( KDevDocument * document )
{
    Q_ASSERT( document );

    KDevCore::partController() ->removePart( document->part() );
    m_documentUrls.remove( document );
    m_partHash.remove( static_cast<KParts::ReadOnlyPart*>( document->part() ) );

    delete document->part();
    delete document;

    updateMenuItems();
}

void KDevDocumentController::replaceDocument( KDevDocument * oldDocument,
        KDevDocument * newDocument, bool setActive )
{
    Q_ASSERT( oldDocument );
    m_partHash.remove( static_cast<KParts::ReadOnlyPart*>( oldDocument->part() ) );
    KDevCore::partController() ->replacePart( oldDocument->part(), newDocument->part(), setActive );
    delete oldDocument;
}

void KDevDocumentController::setActiveDocument( KDevDocument *document, QWidget *widget )
{
    KDevCore::partController() ->setActivePart( document->part(), widget );

    //     kDebug( 9000 ) << k_funcinfo
    //     << KDevCore::partController()->activePart()
    //     << " " << activeDocument() << endl;

    updateMenuItems();

    if ( activeDocument() )
        emit documentActivated( activeDocument() );
}

void KDevDocumentController::init()
{
    KActionCollection * ac =
        KDevCore::mainWindow() ->actionCollection();

    KAction* newAction =
        KStdAction::open( this,
                          SLOT( slotOpenDocument() ),
                          ac, "file_open" );
    newAction->setToolTip( i18n( "Open file" ) );
    newAction->setWhatsThis( i18n( "<b>Open file</b><p>Opens an existing file "
                                   "without adding it to the project.</p>" ) );

    m_openRecentAction =
        KStdAction::openRecent( this, SLOT( slotOpenRecent( const KUrl& ) ),
                                ac, "file_open_recent" );
    m_openRecentAction->setWhatsThis( QString( "<b>%1</b><p>%2" ).arg( beautifyToolTip( m_openRecentAction->text() ) ).arg( i18n( "Opens recently opened file." ) ) );
    m_openRecentAction->loadEntries( KDevConfig::localProject(), "RecentDocuments" );

    m_saveAllDocumentsAction = new KAction( i18n( "Save Al&l" ), ac, "file_save_all" );
    connect( m_saveAllDocumentsAction, SIGNAL( triggered( bool ) ), SLOT( saveAllDocuments() ) );
    m_saveAllDocumentsAction->setToolTip( i18n( "Save all modified files" ) );
    m_saveAllDocumentsAction->setWhatsThis( i18n( "<b>Save all</b><p>Saves all "
                                            "modified files." ) );
    m_saveAllDocumentsAction->setEnabled( false );

    m_reloadAllDocumentsAction = new KAction( i18n( "Reloa&d All" ), ac, "file_revert_all" );
    connect( m_reloadAllDocumentsAction, SIGNAL( triggered() ), SLOT( reloadAllDocuments() ) );
    m_reloadAllDocumentsAction->setToolTip( i18n( "Reload all changes" ) );
    m_reloadAllDocumentsAction->setWhatsThis( i18n( "<b>Reload all</b>"
            "<p>Reloads open documents. Prompts to save changes so"
            "the reload can be canceled for each modified file." ) );
    m_reloadAllDocumentsAction->setEnabled( false );

    m_closeWindowAction = KStdAction::close(
                              this, SLOT( closeActiveDocument() ),
                              ac, "file_close" );
    m_closeWindowAction->setToolTip( i18n( "Close current file" ) );
    m_closeWindowAction->setWhatsThis( QString( "<b>%1</b><p>%2" ).arg( beautifyToolTip( m_closeWindowAction->text() ) ).arg( i18n( "Closes current file." ) ) );
    m_closeWindowAction->setEnabled( false );

    m_closeAllWindowsAction = new KAction( i18n( "Close All" ), ac, "file_close_all" );
    connect( m_closeAllWindowsAction, SIGNAL( triggered() ), SLOT( closeAllDocuments() ) );
    m_closeAllWindowsAction->setToolTip( i18n( "Close all files" ) );
    m_closeAllWindowsAction->setWhatsThis( i18n( "<b>Close all</b><p>Close all "
                                           "opened files." ) );
    m_closeAllWindowsAction->setEnabled( false );

    m_closeOtherWindowsAction = new KAction( i18n( "Close All Others" ), ac, "file_closeother" );
    connect( m_closeOtherWindowsAction, SIGNAL( triggered() ), SLOT( closeAllExceptActiveDocument() ) );
    m_closeOtherWindowsAction->setToolTip( i18n( "Close other files" ) );
    m_closeOtherWindowsAction->setWhatsThis( i18n( "<b>Close all others</b>"
            "<p>Close all opened files except current." ) );
    m_closeOtherWindowsAction->setEnabled( false );

    m_switchToAction = new KAction( i18n( "Switch To..." ), ac, "file_switchto" );
    m_switchToAction->setShortcut( KShortcut( "CTRL+/" ) );
    connect( m_switchToAction, SIGNAL( triggered() ), SLOT( slotSwitchTo() ) );
    m_switchToAction->setToolTip( i18n( "Switch to" ) );
    m_switchToAction->setWhatsThis( i18n( "<b>Switch to</b><p>Prompts to enter "
                                          "the name of previously opened file "
                                          "to switch to." ) );

    new KSeparatorAction( ac, "dummy_separator" );

    m_backAction = new KToolBarPopupAction( i18n( "Back" ), "back", 0, this,
                                            SLOT( slotBack() ), ac,
                                            "history_back" );
    m_backAction->setEnabled( false );
    m_backAction->setToolTip( i18n( "Back" ) );
    m_backAction->setWhatsThis( i18n( "<b>Back</b><p>Moves backwards one step "
                                      "in the navigation history." ) );
    connect( m_backAction->menu(), SIGNAL( aboutToShow() ),
             this, SLOT( slotBackAboutToShow() ) );
    connect( m_backAction->menu(), SIGNAL( activated( int ) ),
             this, SLOT( slotBackPopupActivated( int ) ) );

    m_forwardAction = new KToolBarPopupAction( i18n( "Forward" ), "forward", 0,
                      this, SLOT( slotForward() ), ac,
                      "history_forward" );
    m_forwardAction->setEnabled( false );
    m_forwardAction->setToolTip( i18n( "Forward" ) );
    m_forwardAction->setWhatsThis( i18n( "<b>Forward</b><p>Moves forward one "
                                         "step in the navigation history." ) );
    connect( m_forwardAction->menu(), SIGNAL( aboutToShow() ),
             this, SLOT( slotForwardAboutToShow() ) );
    connect( m_forwardAction->menu(), SIGNAL( activated( int ) ),
             this, SLOT( slotForwardPopupActivated( int ) ) );

}

KParts::Factory *KDevDocumentController::findPartFactory( const QString &mimeType,
        const QString &partType,
        const QString &preferredName )
{
    KService::List offers = KMimeTypeTrader::self() ->query( mimeType, "KParts/ReadWritePart", QString( "'%1' in ServiceTypes" ).arg( partType ) );

    if ( offers.count() > 0 )
    {
        KService::Ptr ptr;
        // if there is a preferred plugin we'll take it
        if ( !preferredName.isEmpty() )
        {
            KService::List::ConstIterator it;
            for ( it = offers.begin(); it != offers.end(); ++it )
            {
                if ( ( *it ) ->desktopEntryName() == preferredName )
                {
                    ptr = ( *it );
                }
            }
        }
        // else we just take the first in the list
        if ( !ptr )
        {
            ptr = offers.first();
        }
        return static_cast<KParts::Factory*>( KLibLoader::self() ->factory(
                                                  QFile::encodeName( ptr->library() ) ) );
    }

    return 0;
}

KDevDocument* KDevDocumentController::integratePart( KParts::Part *part, bool activate )
{
    // tell the parts we loaded a document
    KParts::ReadOnlyPart * ro = readOnly( part );
    if ( !ro )
    {
        kDebug( 9000 ) << k_funcinfo << "no part!!" << endl;
        return 0L;
    }

    if ( activate )
    {
        QWidget * widget = part->widget();

        if ( !widget )
        {
            kDebug( 9000 ) << k_funcinfo << "no widget for this part!!" << endl;
            return 0L; // to avoid later crash
        }

        KDevCore::mainWindow() ->embedPartView( widget,
                                                ro->url().fileName(),
                                                ro->url().url() );
    }

    KDevDocument* doc = addDocument( part, activate );

    KTextEditor::ModificationInterface* iface =
        qobject_cast<KTextEditor::ModificationInterface*>( part );
    if ( iface )
    {
        iface->setModifiedOnDiskWarning( true );
        connect( part, SIGNAL(
                     modifiedOnDisk( KTextEditor::Document*, bool,
                                     KTextEditor::ModificationInterface::ModifiedOnDiskReason ) ),
                 this, SLOT(
                     modifiedOnDisk( KTextEditor::Document*, bool,
                                     KTextEditor::ModificationInterface::ModifiedOnDiskReason ) )
               );
    }

    // let's get notified when a document has been changed
    connect( part, SIGNAL( completed() ), this, SLOT( slotUploadFinished() ) );

    if ( doc->textDocument() )
    {
        connect( doc->textDocument(), SIGNAL( modifiedChanged( KTextEditor::Document* ) ),
                 this, SLOT( newDocumentStatus( KTextEditor::Document* ) ) );
    }

    return doc;
}

KUrl KDevDocumentController::storedUrlForDocument( KDevDocument* document ) const
{
    return m_documentUrls[ document ];
}

void KDevDocumentController::updateDocumentUrl( KDevDocument* document )
{
    if ( document->url().isEmpty() )
    {
        kDebug( 9000 ) << "updatePartURL() called with empty URL for document: "
        << document << endl;
        return ;
    }
    m_documentUrls[ document ] = document->url();
}

bool KDevDocumentController::documentUrlHasChanged( KDevDocument* document )
{
    if ( m_documentUrls.contains( document ) && !document->url().isEmpty() )
    {
        if ( m_documentUrls[ document ] != document->url() )
        {
            emit documentUrlChanged( document, m_documentUrls[ document ], document->url() );
            return true;
        }
    }
    return false;
}

KParts::ReadOnlyPart* KDevDocumentController::activeReadOnly() const
{
    return KDevCore::partController() ->activeReadOnly();
}

KParts::ReadWritePart* KDevDocumentController::activeReadWrite() const
{
    return KDevCore::partController() ->activeReadWrite();
}

KParts::ReadOnlyPart* KDevDocumentController::readOnly( KParts::Part *part ) const
{
    return KDevCore::partController() ->readOnly( part );
}

KParts::ReadWritePart* KDevDocumentController::readWrite( KParts::Part *part ) const
{
    return KDevCore::partController() ->readWrite( part );
}

KDevDocumentController::HistoryEntry::HistoryEntry( const KUrl & u, const KTextEditor::Cursor& c )
        : url( u ), cursor( c )
{
    // should provide a reasonably unique number
    id = abs( QTime::currentTime().msecsTo( QTime() ) );
}

KDevDocumentController::HistoryEntry KDevDocumentController::createHistoryEntry()
{
    if ( !activeDocument() )
        return HistoryEntry();

    KTextEditor::Document * doc = activeDocument() ->textDocument();
    if ( !doc )
        return HistoryEntry();

    KTextEditor::View *view =
        qobject_cast<KTextEditor::View *>( doc->activeView() );
    if ( !view )
        return HistoryEntry();

    KTextEditor::Cursor cursor = view->cursorPosition();
    return HistoryEntry( activeDocument() ->url(), cursor );
}

// this should be called _before_ a jump is made
void KDevDocumentController::addHistoryEntry()
{
    if ( m_isJumping )
        return ;

    HistoryEntry thisEntry = createHistoryEntry();
    if ( !thisEntry.url.isEmpty() )
    {
        m_backHistory.push_front( thisEntry );
        m_backAction->setEnabled( true );

        m_forwardHistory.clear();
        m_forwardAction->setEnabled( false );
    }
}

void KDevDocumentController::jumpTo( const HistoryEntry & entry )
{
    m_isJumping = true;
    editDocument( entry.url, entry.cursor );
    m_isJumping = false;
}

KDevDocument * KDevDocumentController::documentForPart( KParts::Part * part ) const
{
    return m_partHash.value( static_cast<KParts::ReadOnlyPart*>( part ) );
}

#include "kdevdocumentcontroller.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
