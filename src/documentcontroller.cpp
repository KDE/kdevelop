#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <QMap>
#include <QFile>

#include <qdebug.h>
#include <QLabel>
#include <QLayout>
#include <QCheckBox>
#include <qradiobutton.h>

#include "partcontroller.h"

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
#include <kglobal.h>

#include "core.h"
#include "toplevel.h" 
// #include "editorproxy.h"
#include "kdevproject.h"
#include "ksavealldialog.h"
#include "documentationpart.h"
#include "mimewarningdialog.h"
#include "kdevlanguagesupport.h"
#include "kdevhtmlpart.h"

#include "documentcontroller.h"

DocumentController *DocumentController::s_instance = 0;

using namespace MainWindowUtils;

DocumentController::DocumentController( QWidget *parent )
        : KDevDocumentController( parent ),
        m_editorFactory( 0L ),
        m_partController( new PartController( parent ) )
{
    setupActions();

    m_isJumping = false;

    m_openNextAsText = false;
}

DocumentController::~DocumentController()
{}

void DocumentController::setEncoding( const QString &encoding )
{
    m_presetEncoding = encoding;
}

KDevDocument* DocumentController::editDocument( const KUrl &inputUrl, const KTextEditor::Cursor& cursor )
{
    return editDocumentInternal( inputUrl, cursor );
}

KDevDocument* DocumentController::showDocumentation( const KUrl &url, bool newWin )
{
    //FIXME Port to new documentcontroller API
    // possibly could env vars
    //     QString fixedPath = HTMLDocumentationPart::resolveEnvVarsInURL( url.url() );
    //     KUrl docUrl( fixedPath );
    //     kDebug( 9000 ) << "SHOW: " << docUrl.url() << endl;
    //
    //     if ( docUrl.isLocalFile()
    //             && KMimeType::findByURL( docUrl ) ->name() != "text/html" )
    //     {
    //         // a link in a html-file pointed to a local text file - display
    //         // it in the editor instead of a html-view to avoid uglyness
    //         return editDocument( docUrl );
    //     }
    //
    //     addHistoryEntry();
    //
    //     KDevHTMLPart *html = htmlPartForURL( activeDocument() );
    //     if ( !html || newWin )
    //     {
    //         html = new HTMLDocumentationPart();
    //         html->openURL(docUrl);
    //         integratePart( html );
    //         connect( html, SIGNAL( documentURLChanged( const KUrl &, const KUrl & ) ),
    //                  this, SLOT( slotHTMLDocumentURLChanged( const KUrl &, const KUrl & ) ) );
    //     }
    //     else
    //     {
    //         activatePart( html );
    //     }
    //
    //     html->openURL( docUrl );
    //
    //     return documentForPart(html);
    return 0;
}

KDevHTMLPart* DocumentController::htmlPartForURL( KDevDocument* document ) const
{
    return qobject_cast<KDevHTMLPart*>( document->part() );
}

// KParts::Part * DocumentController::partForWidget( const QWidget * widget ) const
// {
//     foreach ( KParts::Part * part, parts() )
//     {
//         if ( part->widget() == widget )
//         {
//             return part;
//         }
//     }
//     return 0;
// }

QList<KDevDocument*> DocumentController::openDocuments( ) const
{
    return m_partHash.values();
}

bool DocumentController::saveAllDocuments()
{
    return saveDocuments( openDocuments() );
}

bool DocumentController::saveDocument( KDevDocument* document, bool force )
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
                           TopLevel::getInstance() ->main(),
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
        m_dirtyDocuments.removeAll( document );
        emit documentStateChanged( document, KDevDocument::Clean );
        emit documentSaved( document );
    }

    return true;
}

bool DocumentController::saveDocuments( const QList<KDevDocument*> & list )
{
    foreach ( KDevDocument * document, list )
    if ( saveDocument( document ) == false )
        return false; //user cancelled

    return true;
}

void DocumentController::reloadAllDocuments()
{
    reloadDocuments( openDocuments() );
}

void DocumentController::reloadDocument( KDevDocument* document )
{
    KParts::ReadWritePart * part = readWrite( document->part() );
    if ( part )
    {
        if ( part->isModified() )
        {
            if ( KMessageBox::warningYesNo( TopLevel::getInstance() ->main(),
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

        m_dirtyDocuments.removeAll( document );
        emit documentStateChanged( document, KDevDocument::Clean );

        int i = 0;
        foreach ( KTextEditor::View * view, document->textDocument() ->textViews() )
        view->setCursorPosition( cursors[ i++ ] );
    }
}

void DocumentController::reloadDocuments( const QList<KDevDocument*> & list )
{
    foreach ( KDevDocument * document, list )
    reloadDocument( document );
}

bool DocumentController::closeAllDocuments()
{
    return closeDocumentsDialog();
}

bool DocumentController::closeDocuments( const QList<KDevDocument*>& list )
{
    foreach ( KDevDocument * document, list )
    if ( !closeDocument( document ) )
        return false;

    return true;
}

bool DocumentController::closeDocument( KDevDocument* document )
{
    if ( !document )
        return false;

    if ( KParts::ReadWritePart * rw_part = readWrite( document->part() ) )
    {
        KUrl url = rw_part->url();
        if ( ! rw_part->closeURL() )
            return false;

        m_dirtyDocuments.removeAll( document );
        emit documentClosed( document );
    }

    TopLevel::getInstance() ->main() ->guiFactory() ->removeClient( document->part() );

    //     if ( QWidget * w = EditorProxy::getInstance() ->topWidgetForPart( part ) )
    //         TopLevel::getInstance() ->removeView( w );

    removeDocument( document );
    return true;
}

bool DocumentController::closeAllOthers( KDevDocument* document )
{
    return closeDocumentsDialog( QList<KDevDocument*>() << document );
}

KDevDocument::DocumentState DocumentController::documentState( KDevDocument* document ) const
{
    KParts::ReadWritePart * rw_part = readWrite( document->part() );

    if ( !rw_part )
        return KDevDocument::Clean;

    KDevDocument::DocumentState state = KDevDocument::Clean;
    if ( rw_part->isModified() )
    {
        state = KDevDocument::Modified;
    }

    if ( isDirty( document ) )
    {
        if ( state == KDevDocument::Modified )
        {
            state = KDevDocument::DirtyAndModified;
        }
        else
        {
            state = KDevDocument::Dirty;
        }
    }

    return state;
}

KDevDocument* DocumentController::activeDocument() const
{
    if ( !m_partController->activePart() )
        return 0L;

    return documentForPart( m_partController->activePart() );
}

void DocumentController::createInstance( QWidget *parent )
{
    if ( !s_instance )
        s_instance = new DocumentController( parent );
}

DocumentController *DocumentController::getInstance()
{
    return s_instance;
}

bool DocumentController::readyToClose()
{
    blockSignals( true );

    // this should never return false, as the files are already saved
    closeAllDocuments();

    return true;
}

bool DocumentController::querySaveDocuments()
{
    return saveDocumentsDialog();
}

void DocumentController::openEmptyTextDocument()
{
    KTextEditor::Document * document = createEditorPart( KUrl(), true );

    if ( document )
    {
        if ( !m_presetEncoding.isNull() )
        {
            KParts::BrowserExtension * extension =
                KParts::BrowserExtension::childObject( document );
            if ( extension )
            {
                KParts::URLArgs args;
                args.serviceType = QString( "text/plain;" ) + m_presetEncoding;
                extension->setURLArgs( args );
            }
            m_presetEncoding = QString::null;
        }

        if ( !document->widget() )
        {
            document->createView( TopLevel::getInstance() ->main() );
        }

        addHistoryEntry();
        integratePart( document, document->widget(),
                       true );

        //         EditorProxy::getInstance() ->setCursorPosition( document, KTextEditor::Cursor() );
    }
}

void DocumentController::integrateTextEditorPart( KTextEditor::Document* doc )
{
    connect( doc, SIGNAL( textChanged( KTextEditor::Document* ) ),
             this, SLOT( slotNewStatus( KTextEditor::Document* ) ) );
}

KDevDocument* DocumentController::editDocumentInternal( const KUrl & inputUrl,
        const KTextEditor::Cursor& cursor,
        bool activate )
{
    // For the splash screen...
    emit openingDocument("Opening: " + inputUrl.fileName());

    //     kDebug( 9000 ) << k_funcinfo
    //     << inputUrl.prettyUrl()
    //     << " cursor " << cursor
    //     << " activate? " << activate << endl;

    KUrl url = inputUrl;

    // is it already open?
    // (Try this once before verifying the URL, we could be dealing with a
    // document that no longer exists on disc)
    if ( KDevDocument * existingDoc = documentForUrl( url ) )
    {
        addHistoryEntry();
        activateDocument( existingDoc );
        //         EditorProxy::getInstance() ->setCursorPosition( existingPart, cursor );
        return existingDoc;
    }

    // Make sure the URL exists
    if ( !url.isValid() || !KIO::NetAccess::exists( url, false, 0 ) )
    {
        bool done = false;

        // Try to find this file in the current project's list instead
        if ( KDevApi::self() ->project() )
        {
            if ( url.isRelativeUrl( url.url() ) )
            {
                KUrl dir( KDevApi::self() ->project() ->projectDirectory() );
                KUrl relURL = KUrl( dir, url.url() );

                kDebug( 9000 ) << k_funcinfo
                << "Looking for file in project dir: "
                << KDevApi::self() ->project() ->projectDirectory()
                << " url " << url.url()
                << " transformed to " << relURL.url()
                << ": " << done << endl;
                if ( relURL.isValid()
                        && KIO::NetAccess::exists( url, false, 0 ) )
                {
                    url = relURL;
                    done = true;
                }
            }
        }

        if ( !done && ( !url.isValid()
                        || !KIO::NetAccess::exists( url, false, 0 ) ) )
        {
            // Not found - prompt the user to find it?
            kDebug( 9000 ) << "cannot find URL: " << url.url() << endl;
            return 0L;
        }
    }

    // We now have a url that exists ;)

    // clean it and resolve possible symlink
    url.cleanPath( KUrl::SimplifyDirSeparators );
    if ( url.isLocalFile() )
    {
        QString path = QFileInfo( url.path() ).canonicalFilePath();
        if ( !path.isEmpty() )
            url.setPath( path );
    }

    KMimeType::Ptr mimeType = KMimeType::findByURL( url );

    kDebug( 9000 ) << "mimeType = " << mimeType->name() << endl;

    // is the URL pointing to a directory?
    if ( mimeType->is( "inode/directory" ) )
    {
        return 0L;
    }

    if ( !m_presetEncoding.isNull() )
    {
        m_openNextAsText = true;
    }

    KConfig *config = KGlobal::config();
    config->setGroup( "General" );

    QStringList textTypesList = config->readEntry( "TextTypes", QStringList() );
    if ( textTypesList.contains( mimeType->name() ) )
    {
        m_openNextAsText = true;
    }

    bool isText = false;
    QVariant v = mimeType->property( "X-KDE-text" );
    kDebug( 9000 ) << mimeType->property( "X-KDE-text" ) << endl;
    if ( v.isValid() )
        isText = v.toBool();

    // is this regular text - open in editor
    if ( m_openNextAsText || isText
            || mimeType->is( "text/plain" )
            || mimeType->is( "text/html" )
            || mimeType->is( "application/x-zerosize" ) )
    {
        KTextEditor::Document * editorPart = createEditorPart( url, activate );
        if ( editorPart )
        {
            if ( !m_presetEncoding.isNull() )
            {
                KParts::BrowserExtension * extension =
                    KParts::BrowserExtension::childObject( editorPart );
                if ( extension )
                {
                    KParts::URLArgs args;
                    args.serviceType = QString( "text/plain;" )
                                       + m_presetEncoding;
                    extension->setURLArgs( args );
                }
                m_presetEncoding = QString::null;
            }

            editorPart->openURL( url );

            if ( !editorPart->widget() )
            {
                // The tab widget does the reparenting
                editorPart->createView( TopLevel::getInstance() ->main() ->centralWidget() );
            }

            addHistoryEntry();
            KDevDocument *document =
                integratePart( editorPart, editorPart->widget(), activate );

            //             EditorProxy::getInstance() ->setCursorPosition( editorPart,
            //                     cursor );

            m_openNextAsText = false;

            m_openRecentAction->addUrl( url );
            m_openRecentAction->saveEntries( KGlobal::config(),
                                             "RecentDocuments" );

            return document;
        }
    }

    // OK, it's not text let's see what else we can come up with..
    if ( KParts::ReadOnlyPart * part = readOnly( m_partController->createPart( url ) ) )
    {
        // create the object of the desired class
        part->openURL( url );
        addHistoryEntry();

        // we can have ended up with a texteditor,
        // in which case need to treat it as such
        KDevDocument *document = 0;
        if ( dynamic_cast<KTextEditor::Editor*>( part ) )
        {
            document = integratePart( part, part->widget(), activate );
            //                 EditorProxy::getInstance() ->setCursorPosition( part, cursor );
        }
        else
        {
            document = integratePart( part );
        }

        m_openRecentAction->addUrl( url );
        m_openRecentAction->saveEntries( KGlobal::config(),
                                         "RecentDocuments" );

        return document;
    }
    else
    {
        MimeWarningDialog dlg;
        dlg.text2->setText( QString( "<qt><b>%1</b></qt>" ).arg( url.path() ) );
        dlg.text3->setText( dlg.text3->text().arg( mimeType->name() ) );

        if ( dlg.exec() == QDialog::Accepted )
        {
            if ( dlg.open_with_kde->isChecked() )
            {
                KRun::runUrl( url, mimeType->name(), 0 );
            }
            else
            {
                if ( dlg.always_open_as_text->isChecked() )
                {
                    KConfig * config = KGlobal::config();
                    config->setGroup( "General" );
                    QStringList textTypesList = config->readEntry( "TextTypes",
                                                QStringList() );
                    textTypesList << mimeType->name();
                    config->writeEntry( "TextTypes", textTypesList );
                }
                m_openNextAsText = true;
                return editDocument( url, cursor );
            }
        }
    }

    return 0L;
}

void DocumentController::slotSave()
{
    kDebug( 9000 ) << k_funcinfo << endl;

    if ( activeReadWrite() )
        saveDocument( activeDocument() );
}

void DocumentController::slotReload()
{
    kDebug( 9000 ) << k_funcinfo << endl;

    if ( activeReadWrite() )
        reloadDocument( activeDocument() );
}

void DocumentController::slotCloseWindow()
{
    closeDocument( activeDocument() );
}

void DocumentController::slotCloseAllWindows()
{
    closeAllDocuments();
}

void DocumentController::slotCloseOtherWindows()
{
    if ( activeReadOnly() )
        closeAllOthers( activeDocument() );
}

void DocumentController::slotSaveAllDocuments()
{
    saveAllDocuments();
}

void DocumentController::slotRevertAllDocuments()
{
    reloadAllDocuments();
}

void DocumentController::slotOpenDocument()
{
    KEncodingFileDialog::Result result =
        KEncodingFileDialog::getOpenURLsAndEncoding(
            QString::null, QString::null, QString::null,
            TopLevel::getInstance() ->main(), QString::null );

    for ( KUrl::List::Iterator it = result.URLs.begin();
            it != result.URLs.end(); ++it )
    {
        m_presetEncoding = result.encoding;
        editDocument( *it );
    }
}

void DocumentController::slotOpenRecent( const KUrl& url )
{
    editDocument( url );
    // stupid bugfix - don't allow an active item in the list
    m_openRecentAction->setCurrentItem( -1 );
}

void DocumentController::slotBack()
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

void DocumentController::slotForward()
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

void DocumentController::slotBackAboutToShow()
{
    KMenu * popup = m_backAction->popupMenu();
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

void DocumentController::slotForwardAboutToShow( )
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

void DocumentController::slotBackPopupActivated( int id )
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

void DocumentController::slotForwardPopupActivated( int id )
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

void DocumentController::slotSwitchTo()
{
    //FIXME Port to new documentcontroller API
    //     QMap<QString, KParts::ReadOnlyPart*> parts_map;
    //     QStringList part_list;
    //     foreach ( KParts::Part * part, parts() )
    //     {
    //         kDebug( 9000 ) << "Part..." << endl;
    //         if ( part->inherits( "KParts::ReadOnlyPart" ) )
    //         {
    //             KParts::ReadOnlyPart * ro_part = readOnly( part );
    //             QString name = ro_part->url().fileName();
    //             part_list.append( name );
    //             parts_map[ name ] = ro_part;
    //             kDebug( 9000 ) << "Found part for URL "
    //             << ro_part->url().prettyUrl() << endl;
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

void DocumentController::slotUploadFinished()
{
    KDevDocument * document = documentForPart( qobject_cast<KParts::Part*>( const_cast<QObject*>( sender() ) ) );

    if ( document && documentUrlHasChanged( document ) )
        updateDocumentUrl( document );
}

void DocumentController::updateMenuItems()
{
    bool hasWriteParts = false;
    bool hasReadOnlyParts = false;

    foreach ( KParts::Part * part, m_partController->parts() )
    {
        if ( part->inherits( "KParts::ReadWritePart" ) )
            hasWriteParts = true;
        if ( part->inherits( "KParts::ReadOnlyPart" ) )
            hasReadOnlyParts = true;
    }

    m_saveAllDocumentsAction->setEnabled( hasWriteParts );
    m_revertAllDocumentsAction->setEnabled( hasWriteParts );
    m_closeWindowAction->setEnabled( hasReadOnlyParts );
    m_closeAllWindowsAction->setEnabled( hasReadOnlyParts );
    m_closeOtherWindowsAction->setEnabled( hasReadOnlyParts );

    m_backAction->setEnabled( !m_backHistory.isEmpty() );
}

void DocumentController::slotDocumentDirty( KTextEditor::Document * d,
        bool isModified,
        KTextEditor::ModificationInterface::ModifiedOnDiskReason reason )
{
    kDebug( 9000 ) << k_funcinfo << endl;

    KDevDocument* doc = documentForPart( d );

    if ( !doc )
        return ;

    KUrl url = storedUrlForDocument( doc );
    if ( url.isEmpty() )
    {
        kDebug( 9000 ) << "Warning!! the stored url is empty. Bailing out!"
        << endl;
        return ;
    }

    if ( reason > 0 )
    {
        if ( !m_dirtyDocuments.contains( doc ) )
        {
            m_dirtyDocuments.append( doc );
        }

        if ( reactToDirty( doc, reason ) )
        {
            // file has been reloaded
            emit documentStateChanged( doc, KDevDocument::Clean );
            m_dirtyDocuments.removeAll( doc );
        }
        else
        {
            doEmitState( doc );
        }
    }
    else
    {
        m_dirtyDocuments.removeAll( doc );
        emit documentStateChanged( doc, KDevDocument::Clean );
    }

    kDebug( 9000 ) << doc->url() << endl;
    kDebug( 9000 ) << isModified << endl;
    kDebug( 9000 ) << reason << endl;
}


void DocumentController::slotNewStatus( KTextEditor::Document * doc )
{
    doEmitState( documentForPart( doc ) );
}

void DocumentController::slotNewDesignerStatus( const QString &formName,
        int status )
{
    kDebug( 9000 ) << k_funcinfo << endl;
    kDebug( 9000 ) << " formName: " << formName
    << ", status: " << status << endl;
    KDevDocument* document = documentForUrl( KUrl( formName ) );
    if ( document )
        emit documentStateChanged( document, KDevDocument::DocumentState( status ) );
}

void DocumentController::setupActions()
{
    KActionCollection * ac =
        TopLevel::getInstance() ->main() ->actionCollection();

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
    m_openRecentAction->loadEntries( KGlobal::config(), "RecentDocuments" );

    m_saveAllDocumentsAction =
        new KAction( i18n( "Save Al&l" ), 0, this,
                     SLOT( slotSaveAllDocuments() ), ac, "file_save_all" );
    m_saveAllDocumentsAction->setToolTip( i18n( "Save all modified files" ) );
    m_saveAllDocumentsAction->setWhatsThis( i18n( "<b>Save all</b><p>Saves all "
                                            "modified files." ) );
    m_saveAllDocumentsAction->setEnabled( false );

    m_revertAllDocumentsAction = new KAction( i18n( "Rever&t All" ), 0,
                                 this, SLOT( slotRevertAllDocuments() ),
                                 ac, "file_revert_all" );
    m_revertAllDocumentsAction->setToolTip( i18n( "Revert all changes" ) );
    m_revertAllDocumentsAction->setWhatsThis( i18n( "<b>Revert all</b>"
            "<p>Reverts all changes in opened files. Prompts to save changes so"
            "the reversion can be canceled for each modified file." ) );
    m_revertAllDocumentsAction->setEnabled( false );

    m_closeWindowAction = KStdAction::close(
                              this, SLOT( slotCloseWindow() ),
                              ac, "file_close" );
    m_closeWindowAction->setToolTip( i18n( "Close current file" ) );
    m_closeWindowAction->setWhatsThis( QString( "<b>%1</b><p>%2" ).arg( beautifyToolTip( m_closeWindowAction->text() ) ).arg( i18n( "Closes current file." ) ) );
    m_closeWindowAction->setEnabled( false );

    m_closeAllWindowsAction = new KAction( i18n( "Close All" ), 0,
                                           this, SLOT( slotCloseAllWindows() ),
                                           ac, "file_close_all" );
    m_closeAllWindowsAction->setToolTip( i18n( "Close all files" ) );
    m_closeAllWindowsAction->setWhatsThis( i18n( "<b>Close all</b><p>Close all "
                                           "opened files." ) );
    m_closeAllWindowsAction->setEnabled( false );

    m_closeOtherWindowsAction = new KAction( i18n( "Close All Others" ), 0,
                                this, SLOT( slotCloseOtherWindows() ),
                                ac, "file_closeother" );
    m_closeOtherWindowsAction->setToolTip( i18n( "Close other files" ) );
    m_closeOtherWindowsAction->setWhatsThis( i18n( "<b>Close all others</b>"
            "<p>Close all opened files except current." ) );
    m_closeOtherWindowsAction->setEnabled( false );

    m_switchToAction = new KAction( i18n( "Switch To..." ),
                                    KShortcut( "CTRL+/" ),
                                    this, SLOT( slotSwitchTo() ),
                                    ac, "file_switchto" );
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
    connect( m_backAction->popupMenu(), SIGNAL( aboutToShow() ),
             this, SLOT( slotBackAboutToShow() ) );
    connect( m_backAction->popupMenu(), SIGNAL( activated( int ) ),
             this, SLOT( slotBackPopupActivated( int ) ) );

    m_forwardAction = new KToolBarPopupAction( i18n( "Forward" ), "forward", 0,
                      this, SLOT( slotForward() ), ac,
                      "history_forward" );
    m_forwardAction->setEnabled( false );
    m_forwardAction->setToolTip( i18n( "Forward" ) );
    m_forwardAction->setWhatsThis( i18n( "<b>Forward</b><p>Moves forward one "
                                         "step in the navigation history." ) );
    connect( m_forwardAction->popupMenu(), SIGNAL( aboutToShow() ),
             this, SLOT( slotForwardAboutToShow() ) );
    connect( m_forwardAction->popupMenu(), SIGNAL( activated( int ) ),
             this, SLOT( slotForwardPopupActivated( int ) ) );

}

void DocumentController::doEmitState( KDevDocument* document )
{
    emit documentStateChanged( document, document->state() );
}

KParts::Factory *DocumentController::findPartFactory( const QString &mimeType,
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

KTextEditor::Document *DocumentController::createEditorPart( const KUrl &url, bool activate )
{
    kDebug( 9000 ) << k_funcinfo << endl;
    KGlobal::config() ->setGroup( "Editor" );
    QString preferred =
        KGlobal::config() ->readPathEntry( "EmbeddedKTextEditor" );

    return qobject_cast<KTextEditor::Document *>( m_partController->createPart(
                "text/plain",
                "KTextEditor/Document",
                "KTextEditor::Document",
                preferred ) );
}

KDevDocument* DocumentController::integratePart( KParts::Part *part, QWidget* widget, bool activate )
{
    // tell the parts we loaded a document
    KParts::ReadOnlyPart * ro_part = readOnly( part );
    if ( !ro_part )
        return 0L;

    if ( !widget )
        widget = part->widget();

    if ( !widget )
    {
        /// @todo error handling
        kDebug( 9000 ) << "no widget for this part!!" << endl;
        return 0L; // to avoid later crash
    }

    TopLevel::getInstance() ->embedPartView( widget,
            ro_part->url().fileName(),
            ro_part->url().url() );

    KDevDocument* doc = addDocument( part, activate );
    emit documentLoaded( doc );

    if ( qobject_cast<KTextEditor::ModificationInterface*>( part ) )
        connect( part, SIGNAL( modifiedOnDisk( KTextEditor::Document*,
                                               bool,
                                               KTextEditor::ModificationInterface::ModifiedOnDiskReason ) ),
                 this, SLOT( slotDocumentDirty( KTextEditor::Document*,
                                                bool,
                                                KTextEditor::ModificationInterface::ModifiedOnDiskReason ) ) );

    // let's get notified when a document has been changed
    connect( part, SIGNAL( completed() ), this, SLOT( slotUploadFinished() ) );

    if ( doc->textDocument() )
        integrateTextEditorPart( doc->textDocument() );

    return doc;
}

QList<KDevDocument*> DocumentController::modifiedDocuments() const
{
    QList<KDevDocument*> modDocuments;

    foreach ( KDevDocument * document, openDocuments() )
    {
        KParts::ReadWritePart * rw_part = readWrite( document->part() );
        if ( rw_part && rw_part->isModified() )
        {
            modDocuments << document;
        }
    }
    return modDocuments;
}

void DocumentController::clearModified( const QList<KDevDocument*>& list )
{
    foreach ( KDevDocument * document, list )
    {
        KParts::ReadWritePart * rw_part = readWrite( document->part() );
        if ( rw_part )
            rw_part->setModified( false );
    }
}

bool DocumentController::isDirty( KDevDocument* document ) const
{
    return m_dirtyDocuments.contains( document );
}

bool DocumentController::reactToDirty( KDevDocument* document, unsigned char reason )
{
    KConfig * config = KGlobal::config();
    config->setGroup( "Editor" );
    QString dirtyAction = config->readEntry( "DirtyAction" );

    if ( dirtyAction == "nothing" )
        return false;

    bool isModified = true;
    if ( KParts::ReadWritePart * part = readWrite( document->part() ) )
    {
        isModified = part->isModified();
    }
    else
    {
        kDebug( 9000 ) << k_funcinfo
        << " Warning. Not a ReadWritePart." << endl;
        return false;
    }

    if ( isModified )
    {
        KMessageBox::sorry( TopLevel::getInstance() ->main(),
                            i18n( "Conflict: The file \"%1\" has changed on "
                                  "disk while being modified in memory.\n\n"
                                  "You should investigate before saving to make"
                                  "sure you are not losing data.", document->url().path() ),
                            i18n( "Conflict" ) );
        return false;
    }

    if ( reason == 3 )                                                         // means the file was deleted
    {
        KMessageBox::sorry( TopLevel::getInstance() ->main(),
                            i18n( "Warning: The file \"%1\" has been deleted on"
                                  "disk.\n\n"
                                  "If this was not your intention, make sure to"
                                  "save this file now.", document->url().path() ),
                            i18n( "Document Deleted" ) );
        return false;
    }

    if ( dirtyAction == "alert" )
    {
        if ( KMessageBox::warningYesNo( TopLevel::getInstance() ->main(),
                                        i18n( "The file \"%1\" has changed on"
                                              "disk.\n\nDo you want to reload it?", document->url().path() ),
                                        i18n( "Document Changed" ) ) == KMessageBox::No )
        {
            return false;
        }
    }

    // here we either answered yes above or are in autoreload mode
    reloadDocument( document );

    return true;
}

KUrl DocumentController::storedUrlForDocument( KDevDocument* document ) const
{
    return m_documentUrls[ document ];
}

void DocumentController::updateDocumentUrl( KDevDocument* document )
{
    if ( document->url().isEmpty() )
    {
        kDebug( 9000 ) << "updatePartURL() called with empty URL for document: "
        << document << endl;
        return ;
    }
    m_documentUrls[ document ] = document->url();
}

bool DocumentController::documentUrlHasChanged( KDevDocument* document )
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

bool DocumentController::saveDocumentsDialog( const QList<KDevDocument*>& ignoreList )
{
    QList<KDevDocument*> modList = modifiedDocuments();

    if ( modList.count() > 0 && modList != ignoreList )
    {
        KSaveSelectDialog dlg( modList, ignoreList,
                               TopLevel::getInstance() ->main() );
        if ( dlg.exec() == QDialog::Accepted )
        {
            saveDocuments( dlg.filesToSave() );
            clearModified( dlg.filesNotToSave() );
        }
        else
        {
            return false;
        }
    }
    return true;
}

bool DocumentController::closeDocumentsDialog( const QList<KDevDocument*>& ignoreList )
{
    if ( !saveDocumentsDialog( ignoreList ) )
        return false;

    foreach ( KParts::Part * part, m_partController->parts() )
    {
        KDevDocument * document = documentForPart( part );
        if ( readOnly( document->part() ) && !ignoreList.contains( document ) )
        {
            closeDocument( document );
        }
    }
    return true;
}

KParts::ReadOnlyPart* DocumentController::activeReadOnly() const
{
    return m_partController->activeReadOnly();
}

KParts::ReadWritePart* DocumentController::activeReadWrite() const
{
    return m_partController->activeReadWrite();
}

KParts::ReadOnlyPart* DocumentController::readOnly( KParts::Part *part ) const
{
    return m_partController->readOnly( part );
}

KParts::ReadWritePart* DocumentController::readWrite( KParts::Part *part ) const
{
    return m_partController->readWrite( part );
}

DocumentController::HistoryEntry::HistoryEntry( const KUrl & u, const KTextEditor::Cursor& c )
        : url( u ), cursor( c )
{
    // should provide a reasonably unique number
    id = abs( QTime::currentTime().msecsTo( QTime() ) );
}

DocumentController::HistoryEntry DocumentController::createHistoryEntry()
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
void DocumentController::addHistoryEntry()
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

void DocumentController::jumpTo( const HistoryEntry & entry )
{
    m_isJumping = true;
    editDocument( entry.url, entry.cursor );
    m_isJumping = false;
}

KDevDocument * DocumentController::documentForPart( KParts::Part * part ) const
{
    return m_partHash.value( static_cast<KParts::ReadOnlyPart*>( part ) );
}

KDevDocument * DocumentController::addDocument( KParts::Part * part, bool setActive )
{
    KDevDocument * document =
        new KDevDocument( static_cast<KParts::ReadOnlyPart*>( part ), this );
    m_partHash.insert( static_cast<KParts::ReadOnlyPart*>( part ), document );
    m_partController->addPart( part, setActive );
    updateDocumentUrl( document );
    updateMenuItems();

    return document;
}

void DocumentController::removeDocument( KDevDocument * document )
{
    Q_ASSERT( document );

    m_partController->removePart( document->part() );
    m_documentUrls.remove( document );
    m_partHash.remove( static_cast<KParts::ReadOnlyPart*>( document->part() ) );

    delete document->part();
    delete document;

    updateMenuItems();
}

void DocumentController::replaceDocument( KDevDocument * oldDocument,
        KDevDocument * newDocument, bool setActive )
{
    Q_ASSERT( oldDocument );
    m_partHash.remove( static_cast<KParts::ReadOnlyPart*>( oldDocument->part() ) );
    m_partController->replacePart( oldDocument->part(), newDocument->part(), setActive );
    delete oldDocument;
}

void DocumentController::setActiveDocument( KDevDocument *document, QWidget *widget )
{
    m_partController->setActivePart( document->part(), widget );

    kDebug( 9000 ) << k_funcinfo
    << m_partController->activePart()
    << " " << activeDocument() << endl;

    updateMenuItems();

    if ( activeDocument() )
        emit documentActivated( activeDocument() );
}

void DocumentController::activateDocument( KDevDocument * document )
{
    Q_ASSERT( document );

    if ( document->isActive() )
        return ;

    setActiveDocument( document );

    QWidget * widget = document->part() ->widget();
    if ( widget )
    {
        TopLevel::getInstance() ->setCurrentWidget( widget );
        widget->show();
        widget->setFocus();
    }
}

void DocumentController::slotHTMLDocumentURLChanged( const KUrl & oldUrl, const KUrl & newUrl )
{
    KParts::Part * part = qobject_cast<KParts::Part*>( sender() );
    if ( !part )
        return ;
    emit documentUrlChanged( documentForPart( part ), oldUrl, newUrl );
}

#include "documentcontroller.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
