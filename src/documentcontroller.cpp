#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <qmap.h>
#include <qfile.h>
#include <qdebug.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qradiobutton.h>

#include <krun.h>
#include <kmenu.h>
#include <kdebug.h>
#include <klocale.h>
#include <kaction.h>
#include <ktrader.h>
#include <kservice.h>
#include <kmimetype.h>
#include <klineedit.h>
#include <kshortcut.h>
#include <kdirwatch.h>
#include <kstatusbar.h>
#include <khtml_part.h>
#include <kdeversion.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kmainwindow.h>
#include <kdialogbase.h>
#include <kcompletion.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kuserprofile.h>
#include <kxmlguifactory.h>
#include <ksqueezedtextlabel.h>
#include <kencodingfiledialog.h>

#include <kio/netaccess.h>

#include <kparts/part.h>
#include <kparts/factory.h>
#include <kparts/partmanager.h>
#include <kparts/browserextension.h>

#include <ktexteditor/view.h>
#include <ktexteditor/editor.h>
#include <ktexteditor/document.h>
#include <ktexteditor/modificationinterface.h>

#include "api.h"
#include "core.h"
#include "urlutil.h"
#include "toplevel.h"
#include "editorproxy.h"
#include "kdevproject.h"
#include "ksavealldialog.h"
#include "documentationpart.h"
#include "mimewarningdialog.h"
#include "kdevlanguagesupport.h"

#include "documentcontroller.h"

DocumentController *DocumentController::s_instance = 0;

using namespace MainWindowUtils;

DocumentController::DocumentController( QWidget *parent )
        : KDevDocumentController( parent ), m_editorFactory( 0L )
{
    connect( this, SIGNAL( partRemoved( KParts::Part* ) ),
             this, SLOT( slotPartRemoved( KParts::Part* ) ) );
    connect( this, SIGNAL( partAdded( KParts::Part* ) ),
             this, SLOT( slotPartAdded( KParts::Part* ) ) );
    connect( this, SIGNAL( activePartChanged( KParts::Part* ) ),
             this, SLOT( slotActivePartChanged( KParts::Part* ) ) );

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

void DocumentController::editDocument( const KURL &inputUrl,
                                       int lineNum, int col )
{
    editDocumentInternal( inputUrl, lineNum, col );
}

void DocumentController::showDocument( const KURL &url, bool newWin )
{
    // possibly could env vars
    QString fixedPath = HTMLDocumentationPart::resolveEnvVarsInURL( url.url() );
    KURL docUrl( fixedPath );
    kdDebug( 9000 ) << "SHOW: " << docUrl.url() << endl;

    if ( docUrl.isLocalFile()
            && KMimeType::findByURL( docUrl ) ->name() != "text/html" )
    {
        // a link in a html-file pointed to a local text file - display
        // it in the editor instead of a html-view to avoid uglyness
        editDocument( docUrl );
        return ;
    }

    addHistoryEntry();

    KDevHTMLPart *html = htmlPartForURL( activeDocument() );
    if ( !html || newWin )
    {
        html = new HTMLDocumentationPart;
        integratePart( html, docUrl );
        connect( html, SIGNAL( documentURLChanged( const KURL &, const KURL & ) ),
                 this, SIGNAL( documentURLChanged( const KURL &, const KURL & ) ) );
    }
    else
    {
        activatePart( html );
    }
    html->openURL( docUrl );
}

void DocumentController::showPart( KParts::Part* part,
                                   const QString& name,
                                   const QString& shortDescription )
{
    if ( !part->widget() )
    {
        /// @todo error handling
        return ; // to avoid later crash
    }

    foreach (KParts::Part* p, parts())
    {
        if ( p == part )
        {
            // part already embedded
            activatePart( p );
            return ;
        }
    }

    // embed the part
    TopLevel::getInstance() ->embedPartView( part->widget(),
            name, shortDescription );
    addPart( part );
}

KParts::ReadOnlyPart *DocumentController::partForURL( const KURL &url ) const
{
    foreach (KParts::Part* part, parts())
    {
        KParts::ReadOnlyPart *ro_part = readOnly( part );
        if ( ro_part && url == ro_part->url() )
            return ro_part;
    }
    return 0;
}

KTextEditor::Document * DocumentController::textPartForURL( const KURL & url ) const
{
    return qobject_cast<KTextEditor::Document*>( partForURL( url ) );
}

// void* DocumentController::designerPartForURL( const KURL & url )
// {
//     return qobject_cast<void*>( partForURL( url ) );
// }

KDevHTMLPart* DocumentController::htmlPartForURL( const KURL & url ) const
{
    return qobject_cast<KDevHTMLPart*>( partForURL( url ) );
}

KDevDocumentType DocumentController::documentTypeForURL( const KURL & url ) const
{
    if ( textPartForURL( url ) )
        return TextDocument;
    else if ( htmlPartForURL( url ) )
        return HTMLDocument;
    else
        return Invalid;
}

KParts::Part * DocumentController::partForWidget( const QWidget * widget ) const
{
    foreach (KParts::Part* part, parts())
    {
        if ( part->widget() == widget )
        {
            return part;
        }
    }
    return 0;
}

KURL::List DocumentController::openURLs( ) const
{
    KURL::List list;
    foreach (KParts::Part* part, parts())
    {
        if ( KParts::ReadOnlyPart * ro_part = readOnly( part ) )
        {
            list << ro_part->url();
        }
    }
    return list;
}

bool DocumentController::saveAllDocuments()
{
    return saveDocuments( openURLs() );
}

bool DocumentController::saveDocument( const KURL & url, bool force )
{
    KParts::ReadWritePart * part = readWriteForURL( url );
    if ( !part )
        return true;

    switch ( documentState( url ) )
    {
    case Clean:
        if ( !force )
        {
            return true;
        }
        kdDebug( 9000 ) << "Forced save" << endl;
        break;

    case Modified:
        kdDebug( 9000 ) << "Normal save" << endl;
        break;

    case Dirty:
    case DirtyAndModified:
        {
            int code = KMessageBox::warningYesNoCancel(
                           TopLevel::getInstance() ->main(),
                           i18n( "The file \"%1\" is modified on disk.\n\nAre "
                                 "you sure you want to overwrite it? (External "
                                 "changes will be lost.)" ).arg( url.path() ),
                           i18n( "Document Externally Modified" ) );
            if ( code == KMessageBox::Yes )
            {
                kdDebug( 9000 ) << "Dirty save!!" << endl;
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
        ;
    }

    if ( part->save() )
    {
        m_dirtyDocuments.remove( part );
        emit documentStateChanged( url, Clean );
        emit documentSaved( url );
    }

    return true;
}

bool DocumentController::saveDocuments( KURL::List const & filelist )
{
    KURL::List::ConstIterator it = filelist.begin();
    while ( it != filelist.end() )
    {
        if ( saveDocument( *it ) == false )
            return false; //user cancelled
        ++it;
    }
    return true;
}

void DocumentController::reloadAllDocuments()
{
    reloadDocuments( openURLs() );
}

void DocumentController::reloadDocument( const KURL & url )
{
    KParts::ReadWritePart * part = readWriteForURL( url );
    if ( part )
    {
        if ( part->isModified() )
        {
            if ( KMessageBox::warningYesNo( TopLevel::getInstance() ->main(),
                                            i18n( "The file \"%1\" is modified "
                                                  "in memory. Are you sure you "
                                                  "want to reload it? (Local "
                                                  "changes will be lost.)" ).arg( url.path() ),
                                            i18n( "Document is Modified" ) ) == KMessageBox::Yes )
            {
                part->setModified( false );
            }
            else
            {
                return ;
            }
        }

        KTextEditor::Cursor cursor;
        KTextEditor::View *view =
            qobject_cast<KTextEditor::View *>( part->widget() );
        if ( view )
            cursor = view->cursorPosition();

        part->openURL( url );

        m_dirtyDocuments.remove( part );
        emit documentStateChanged( url, Clean );

        if ( view )
            view->setCursorPosition( cursor );
    }
}

void DocumentController::reloadDocuments( const KURL::List & list )
{
    KURL::List::ConstIterator it = list.begin();
    while ( it != list.end() )
    {
        reloadDocument( *it );
        ++it;
    }
}

bool DocumentController::closeAllDocuments()
{
    return closeDocumentsDialog( KURL::List() );
}

bool DocumentController::closeDocuments( const KURL::List & list )
{
    KURL::List::ConstIterator it = list.begin();
    while ( it != list.end() )
    {
        if ( !closePart( partForURL( *it ) ) )
        {
            return false;
        }
        ++it;
    }
    return true;
}

bool DocumentController::closeDocument( const KURL & url )
{
    return closePart( partForURL( url ) );
}

bool DocumentController::closeAllOthers( const KURL & url )
{
    KURL::List ignoreList;
    ignoreList.append( url );

    return closeDocumentsDialog( ignoreList );
}

bool DocumentController::closePart( KParts::Part *part )
{
    if ( !part )
        return false;

    if ( KParts::ReadWritePart * rw_part = readWrite( part ) )
    {
        KURL url = rw_part->url();
        if ( ! rw_part->closeURL() )
            return false;

        m_dirtyDocuments.remove( rw_part );
        emit documentClosed( url );
    }

    TopLevel::getInstance() ->main() ->guiFactory() ->removeClient( part );

    if ( QWidget * w = EditorProxy::getInstance() ->topWidgetForPart( part ) )
        TopLevel::getInstance() ->removeView( w );

    delete part;
    return true;
}

void DocumentController::activatePart( KParts::Part *part )
{
    if ( !part )
        return ;

    QWidget * widget = EditorProxy::getInstance() ->widgetForPart( part );
    if ( widget )
    {
        TopLevel::getInstance() ->raiseView( widget );
        widget->show();
        widget->setFocus();
    }

    setActivePart( part );

    QWidget* w2 = EditorProxy::getInstance() ->widgetForPart( part );
    if ( w2 != widget )
        w2->setFocus();

    emit documentActivated( activeDocument() );
}

DocumentState DocumentController::documentState( KURL const & url )
{
    KParts::ReadWritePart * rw_part = readWriteForURL( url );

    if ( !rw_part )
        return Clean;

    DocumentState state = Clean;
    if ( rw_part->isModified() )
    {
        state = Modified;
    }

    if ( isDirty( url ) )
    {
        if ( state == Modified )
        {
            state = DirtyAndModified;
        }
        else
        {
            state = Dirty;
        }
    }

    return state;
}

KURL DocumentController::activeDocument() const
{
    if ( !activePart() )
        return KURL();

    KParts::ReadOnlyPart * ro_part = activeReadOnly();
    return ro_part->url();
}

KDevDocumentType DocumentController::activeDocumentType() const
{
    return documentTypeForURL( activeDocument() );
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
    return saveDocumentsDialog( KURL::List() );
}

void DocumentController::openEmptyTextDocument()
{
    KTextEditor::Document * document = createEditorPart( true );

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

        //        editorpart->openURL( url );

        QWidget* widget = document->widget();

        if ( !widget )
        {
            // We're being lazy about creating the view, but kmdi _needs_ a
            // widget to create a tab for it, so use a QWidgetStack subclass
            // instead
            kdDebug() << k_lineinfo << "Creating Editor wrapper..." << endl;
            widget = new EditorWrapper( document, true,
                                        TopLevel::getInstance() ->main() );
        }

        addHistoryEntry();
        integratePart( document, KURL( i18n( "unnamed" ) ), widget,
                       true, true );

        EditorProxy::getInstance() ->setLineNumber( document, 0, 0 );
    }
}

void DocumentController::integrateTextEditorPart( KTextEditor::Document* doc )
{
    connect( doc, SIGNAL( textChanged( KTextEditor::Document* ) ),
             this, SLOT( slotNewStatus( KTextEditor::Document* ) ) );
}

void DocumentController::editDocumentInternal( const KURL & inputUrl,
        int lineNum, int col,
        bool activate )
{
    kdDebug( 9000 ) << k_funcinfo
    << inputUrl.prettyURL()
    << " linenum " << lineNum
    << " activate? " << activate << endl;

    KURL url = inputUrl;

    // is it already open?
    // (Try this once before verifying the URL, we could be dealing with a
    // document that no longer exists on disc)
    if ( KParts::Part * existingPart = partForURL( url ) )
    {
        addHistoryEntry();
        activatePart( existingPart );
        EditorProxy::getInstance() ->setLineNumber( existingPart,
                lineNum, col );
        return ;
    }

    // Make sure the URL exists
    if ( !url.isValid() || !KIO::NetAccess::exists( url, false, 0 ) )
    {
        bool done = false;

        // Try to find this file in the current project's list instead
        if ( API::getInstance() ->project() )
        {
            if ( url.isRelativeURL( url.url() ) )
            {
                KURL dir( API::getInstance() ->project() ->projectDirectory() );
                KURL relURL = KURL( dir, url.url() );

                kdDebug( 9000 ) << k_funcinfo
                << "Looking for file in project dir: "
                << API::getInstance() ->project() ->projectDirectory()
                << " url " << url.url()
                << " transformed to " << relURL.url()
                << ": " << done << endl;
                if ( relURL.isValid()
                        && KIO::NetAccess::exists( url, false, 0 ) )
                {
                    url = relURL;
                    done = true;
                }
                else
                {
                    KURL b( API::getInstance() ->project() ->buildDirectory() );
                    KURL relURL = KURL( b, url.url() );
                    kdDebug( 9000 ) << k_funcinfo
                    << "Looking for file in build dir: "
                    << API::getInstance() ->project() ->buildDirectory()
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

            if ( !done )
            {
                url = findURLInProject( url );

                if ( !url.isValid()
                        || !KIO::NetAccess::exists( url, false, 0 ) )
                    // See if this url is relative to the
                    // current project's directory
                    url = API::getInstance() ->project() ->projectDirectory()
                          + "/" + url.path();

                else
                    done = true;
            }
        }

        if ( !done && ( !url.isValid()
                        || !KIO::NetAccess::exists( url, false, 0 ) ) )
        {
            // Not found - prompt the user to find it?
            kdDebug( 9000 ) << "cannot find URL: " << url.url() << endl;
            return ;
        }
    }

    // We now have a url that exists ;)

    // clean it and resolve possible symlink
    url.cleanPath( true );
    if ( url.isLocalFile() )
    {
        QString path = url.path();
        path = URLUtil::canonicalPath( path );
        if ( !path.isEmpty() )
            url.setPath( path );
    }

    KMimeType::Ptr mimeType = KMimeType::findByURL( url );

    kdDebug( 9000 ) << "mimeType = " << mimeType->name() << endl;

    // is the URL pointing to a directory?
    if ( mimeType->is( "inode/directory" ) )
    {
        return ;
    }

    if ( !m_presetEncoding.isNull() )
    {
        m_openNextAsText = true;
    }

    KConfig *config = kapp->config();
    config->setGroup( "General" );

    QStringList textTypesList = config->readListEntry( "TextTypes" );
    if ( textTypesList.contains( mimeType->name() ) )
    {
        m_openNextAsText = true;
    }

    // is this regular text - open in editor
    if ( m_openNextAsText
            || mimeType->is( "text/plain" )
            || mimeType->is( "text/html" )
            || mimeType->is( "application/x-zerosize" ) )
    {
        KTextEditor::Document * editorPart = createEditorPart( activate );

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

            QWidget* widget = editorPart->widget();

            if ( !widget )
            {
                // We're being lazy about creating the view,
                // but kmdi _needs_ a widget to
                // create a tab for it, so use a QWidgetStack subclass instead
                kdDebug() << k_lineinfo << "Creating Editor wrapper..." << endl;
                KTextEditor::Document* doc =
                    qobject_cast<KTextEditor::Document*>( editorPart );
                widget = new EditorWrapper( doc,
                                            activate,
                                            TopLevel::getInstance() ->main() );
            }

            addHistoryEntry();
            integratePart( editorPart, url, widget, true, activate );

            EditorProxy::getInstance() ->setLineNumber( editorPart,
                    lineNum, col );

            m_openNextAsText = false;

            m_openRecentAction->addURL( url );
            m_openRecentAction->saveEntries( kapp->config(),
                                             "RecentDocuments" );

            return ;
        }
    }

    // OK, it's not text and it's not a designer file..
    // let's see what else we can come up with..

    KParts::Factory *factory = 0;
    QString className;

    QString services[] = {
                             "KDevelop/ReadWritePart", "KDevelop/ReadOnlyPart",
                             "KParts/ReadWritePart", "KParts/ReadOnlyPart"
                         };

    QString classNames[] = {
                               "KParts::ReadWritePart", "KParts::ReadOnlyPart",
                               "KParts::ReadWritePart", "KParts::ReadOnlyPart"
                           };

    for ( uint i = 0; i < 4; ++i )
    {
        factory = findPartFactory( mimeType->name(), services[ i ] );
        if ( factory )
        {
            className = classNames[ i ];
            break;
        }
    }

    kdDebug( 9000 ) << "factory = " << factory << endl;

    if ( factory )
    {
        // create the object of the desired class
        KParts::ReadOnlyPart * part = readOnly( factory->createPart(
                                                    TopLevel::getInstance() ->main(),
                                                    0, 0, 0, className.toLatin1() ) );
        if ( part )
        {
            part->openURL( url );
            addHistoryEntry();

            // we can have ended up with a texteditor,
            // in which case need to treat it as such
            if ( dynamic_cast<KTextEditor::Editor*>( part ) )
            {
                integratePart( part, url, part->widget(), true, activate );
                EditorProxy::getInstance() ->setLineNumber( part,
                        lineNum, col );
            }
            else
            {
                integratePart( part, url );
            }

            m_openRecentAction->addURL( url );
            m_openRecentAction->saveEntries( kapp->config(),
                                             "RecentDocuments" );
        }
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
                KRun::runURL( url, mimeType->name() );
            }
            else
            {
                if ( dlg.always_open_as_text->isChecked() )
                {
                    KConfig * config = kapp->config();
                    config->setGroup( "General" );
                    QStringList textTypesList =
                        config->readListEntry( "TextTypes" );
                    textTypesList << mimeType->name();
                    config->writeEntry( "TextTypes", textTypesList );
                }
                m_openNextAsText = true;
                editDocument( url, lineNum, col );
            }
        }
    }
}

void DocumentController::slotSave()
{
    kdDebug( 9000 ) << k_funcinfo << endl;

    if ( KParts::ReadWritePart * part = activeReadWrite() )
    {
        saveDocument( part->url() );
    }
}

void DocumentController::slotReload()
{
    kdDebug( 9000 ) << k_funcinfo << endl;

    if ( KParts::ReadWritePart * part = activeReadWrite() )
    {
        reloadDocument( part->url() );
    }
}

void DocumentController::slotCloseWindow()
{
    closePart( activePart() );
}

void DocumentController::slotCloseAllWindows()
{
    closeAllDocuments();
}

void DocumentController::slotCloseOtherWindows()
{
    if ( KParts::ReadOnlyPart * active = activeReadOnly() )
    {
        closeAllOthers( active->url() );
    }
}

void DocumentController::slotActivePartChanged( KParts::Part *part )
{
    kdDebug( 9000 ) << k_funcinfo << endl;
    updateMenuItems();
    QTimer::singleShot( 100, this, SLOT( slotWaitForFactoryHack() ) );
    emit documentActivated( activeDocument() );
}

void DocumentController::slotWaitForFactoryHack( )
{
    //kdDebug(9000) << k_funcinfo << endl;

    if ( !activePart() )
        return ;

    if ( dynamic_cast<KTextEditor::View*>( activePart() ->widget() ) )
    {
        if ( !activePart() ->factory() )
        {
            QTimer::singleShot( 100, this, SLOT( slotWaitForFactoryHack() ) );
        }
        else
        {
            EditorProxy::getInstance() ->installPopup( activePart() );
        }
    }
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

    for ( KURL::List::Iterator it = result.URLs.begin();
            it != result.URLs.end(); ++it )
    {
        m_presetEncoding = result.encoding;
        editDocument( *it );
    }
}

void DocumentController::slotOpenRecent( const KURL& url )
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
                           + QString( " (%1)" ).arg( ( *it ).line + 1 ),
                           ( *it ).id );
        ++i;
        ++it;
    }
}

void DocumentController::slotForwardAboutToShow( )
{
    KMenu * popup = m_forwardAction->popupMenu();
    popup->clear();

    if ( m_forwardHistory.isEmpty() )
        return ;

    int i = 0;
    for ( int i = 0; i < 10 || i < m_forwardHistory.count(); ++i )
    {
        HistoryEntry entry = m_forwardHistory.at( i );
        popup->insertItem( entry.url.fileName()
                           + QString( " (%1)" ).arg( entry.line + 1 ),
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
    QMap<QString, KParts::ReadOnlyPart*> parts_map;
    QStringList part_list;
    foreach (KParts::Part* part, parts())
    {
        kdDebug( 9000 ) << "Part..." << endl;
        if ( part->inherits( "KParts::ReadOnlyPart" ) )
        {
            KParts::ReadOnlyPart * ro_part = readOnly( part );
            QString name = ro_part->url().fileName();
            part_list.append( name );
            parts_map[ name ] = ro_part;
            kdDebug( 9000 ) << "Found part for URL "
            << ro_part->url().prettyURL() << endl;
        }
    }

    KDialogBase dialog( KDialogBase::Plain, i18n( "Switch To" ),
                        KDialogBase::Ok | KDialogBase::Cancel,
                        KDialogBase::Ok, 0, "Switch to", true );
    QGridLayout *grid = new QGridLayout( dialog.plainPage(), 2, 1, 10, 10 );
    KLineEdit *editbox = new KLineEdit( dialog.plainPage() );
    grid->addWidget( new QLabel( i18n( "Switch to buffer:" ),
                                 dialog.plainPage() ), 0, 0 );
    grid->addWidget( editbox, 1, 0 );
    editbox->completionObject() ->setItems( part_list );
    editbox->setFocus();
    int result = dialog.exec();
    if ( result == KDialogBase::KDialogBase::Accepted )
    {
        if ( parts_map.contains( editbox->text() ) )
        {
            activatePart( parts_map[ editbox->text() ] );
        }
    }
}

void DocumentController::slotPartAdded( KParts::Part * part )
{
    kdDebug( 9000 ) << k_funcinfo << endl;

    if ( KParts::ReadOnlyPart * ro_part = readOnly( part ) )
    {
        updatePartURL( ro_part );
    }

    updateMenuItems();
}

void DocumentController::slotPartRemoved( KParts::Part * part )
{
    kdDebug( 9000 ) << k_funcinfo << endl;

    m_partURLMap.remove( readOnly( part ) );

    updateMenuItems();
}

void DocumentController::slotUploadFinished()
{
    KParts::ReadOnlyPart * ro_part =
        const_cast<KParts::ReadOnlyPart*>(
            dynamic_cast<const KParts::ReadOnlyPart*>( sender() ) );

    if ( partURLHasChanged( ro_part ) )
    {
        updatePartURL( ro_part );
    }
}

void DocumentController::updateMenuItems()
{
    bool hasWriteParts = false;
    bool hasReadOnlyParts = false;

    foreach (KParts::Part* part, parts())
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
        ModifiedOnDiskReason reason )
{
    kdDebug( 9000 ) << k_funcinfo << endl;

    KTextEditor::Document * doc = 0;

    foreach (KParts::Part* part, parts())
    {
        if ( ( void* ) part == ( void* ) d )
        {
            doc = dynamic_cast<KTextEditor::Document*>( part );
            break;
        }
    }

    if ( !doc )
        return ;
    KURL url = storedURLForPart( doc );
    if ( url.isEmpty() )
    {
        kdDebug( 9000 ) << "Warning!! the stored url is empty. Bailing out!"
        << endl;
    }

    if ( reason > 0 )
    {
        if ( !m_dirtyDocuments.contains( doc ) )
        {
            m_dirtyDocuments.append( doc );
        }

        if ( reactToDirty( url, reason ) )
        {
            // file has been reloaded
            emit documentStateChanged( url, Clean );
            m_dirtyDocuments.remove( doc );
        }
        else
        {
            doEmitState( url );
        }
    }
    else
    {
        m_dirtyDocuments.remove( doc );
        emit documentStateChanged( url, Clean );
    }

    kdDebug( 9000 ) << doc->url().url() << endl;
    kdDebug( 9000 ) << isModified << endl;
    kdDebug( 9000 ) << reason << endl;
}


void DocumentController::slotNewStatus( KTextEditor::Document * doc )
{
    doEmitState( doc ->url() );
}

void DocumentController::slotNewDesignerStatus( const QString &formName,
        int status )
{
    kdDebug( 9000 ) << k_funcinfo << endl;
    kdDebug( 9000 ) << " formName: " << formName
    << ", status: " << status << endl;
    emit documentStateChanged( KURL::fromPathOrURL( formName ),
                               DocumentState( status ) );
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
        KStdAction::openRecent( this, SLOT( slotOpenRecent( const KURL& ) ),
                                ac, "file_open_recent" );
    m_openRecentAction->setWhatsThis( QString( "<b>%1</b><p>%2" ).arg( beautifyToolTip( m_openRecentAction->text() ) ).arg( i18n( "Opens recently opened file." ) ) );
    m_openRecentAction->loadEntries( kapp->config(), "RecentDocuments" );

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

    new KActionSeparator( ac, "dummy_separator" );

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

void DocumentController::doEmitState( KURL const & url )
{
    emit documentStateChanged( url, documentState( url ) );
}

KURL DocumentController::findURLInProject( const KURL& url )
{
    QStringList fileList = API::getInstance() ->project() ->allFiles();

    bool filenameOnly = ( url.url().find( '/' ) == -1 );
    QString filename = filenameOnly ? "/" : "";
    filename += url.url();

    for ( QStringList::Iterator it = fileList.begin();
            it != fileList.end(); ++it )
    {
        if ( ( *it ).endsWith( filename ) )
        {
            // Match! The first one is as good as any one, I guess...
            return KURL( API::getInstance() ->project() ->projectDirectory()
                         + "/" + *it );
        }
    }

    return url;
}

KParts::Part* DocumentController::findOpenDocument( const KURL& url )
{
    // if we find it this way, all is well
    KParts::Part * part = partForURL( url );
    if ( part )
    {
        return part;
    }

    // ok, let's see if we can try harder
    if ( API::getInstance() ->project() )
    {
        KURL partURL = findURLInProject( url );
        partURL.cleanPath();
        return partForURL( partURL );
    }

    return 0L;
}

KParts::Factory *DocumentController::findPartFactory( const QString &mimeType,
        const QString &partType,
        const QString &preferredName )
{
    KTrader::OfferList offers = KTrader::self() ->query( mimeType,
                                QString( "'%1' in ServiceTypes" ).arg( partType ) );

    if ( offers.count() > 0 )
    {
        KService::Ptr ptr = 0;
        // if there is a preferred plugin we'll take it
        if ( !preferredName.isEmpty() )
        {
            KTrader::OfferList::Iterator it;
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

KTextEditor::Document *DocumentController::createEditorPart( bool activate )
{
    if ( !m_editorFactory )
    {
        kapp->config() ->setGroup( "Editor" );
        QString preferred =
            kapp->config() ->readPathEntry( "EmbeddedKTextEditor" );

        m_editorFactory = findPartFactory( "text/plain",
                                           "KTextEditor/Document", preferred );

        if ( !m_editorFactory )
            return 0;
    }

    KTextEditor::Document *doc =
        qobject_cast<KTextEditor::Document *>( m_editorFactory->createPart(
                                                   TopLevel::getInstance() ->main(),
                                                   0, 0, 0, "KParts::ReadWritePart" ) );
    return doc;
}

void DocumentController::integratePart( KParts::Part *part, const KURL &url,
                                        QWidget* widget, bool isTextEditor,
                                        bool activate )
{
    if ( !widget )
        widget = part->widget();

    if ( !widget )
    {
        /// @todo error handling
        kdDebug( 9000 ) << "no widget for this part!!" << endl;
        return ; // to avoid later crash
    }

    TopLevel::getInstance() ->embedPartView( widget,
            url.fileName(),
            url.url() );

    addPart( part, activate );

    // tell the parts we loaded a document
    KParts::ReadOnlyPart *ro_part = readOnly( part );
    if ( !ro_part )
        return ;

    emit documentLoaded( ro_part->url() );

    if ( qobject_cast<KTextEditor::ModificationInterface*>( part ) )
        connect( part, SIGNAL( modifiedOnDisk( KTextEditor::Document*,
                                               bool,
                                               ModifiedOnDiskReason ) ),
                 this, SLOT( slotDocumentDirty( KTextEditor::Document*,
                                                bool,
                                                ModifiedOnDiskReason ) ) );

    // let's get notified when a document has been changed
    connect( part, SIGNAL( completed() ), this, SLOT( slotUploadFinished() ) );

    if ( isTextEditor )
        integrateTextEditorPart( qobject_cast<KTextEditor::Document*>( part ) );
}

KURL::List DocumentController::modifiedDocuments()
{
    KURL::List modDocuments;

    foreach (KParts::Part* part, parts())
    {
        KParts::ReadWritePart * rw_part = readWrite( part );
        if ( rw_part && rw_part->isModified() )
        {
            modDocuments << rw_part->url();
        }
    }
    return modDocuments;
}

void DocumentController::clearModified( KURL::List const & filelist )
{
    KURL::List::ConstIterator it = filelist.begin();
    while ( it != filelist.end() )
    {
        KParts::ReadWritePart * rw_part = readWriteForURL( *it );
        if ( rw_part )
        {
            rw_part->setModified( false );
        }
        ++it;
    }
}

bool DocumentController::isDirty( KURL const & url )
{
    return m_dirtyDocuments.contains( textPartForURL( url ) );
}

bool DocumentController::reactToDirty( KURL const & url, unsigned char reason )
{
    KConfig * config = kapp->config();
    config->setGroup( "Editor" );
    QString dirtyAction = config->readEntry( "DirtyAction" );

    if ( dirtyAction == "nothing" )
        return false;

    bool isModified = true;
    if ( KParts::ReadWritePart * part = readWriteForURL( url ) )
    {
        isModified = part->isModified();
    }
    else
    {
        kdDebug( 9000 ) << k_funcinfo
        << " Warning. Not a ReadWritePart." << endl;
        return false;
    }

    if ( isModified )
    {
        KMessageBox::sorry( TopLevel::getInstance() ->main(),
                            i18n( "Conflict: The file \"%1\" has changed on "
                                  "disk while being modified in memory.\n\n"
                                  "You should investigate before saving to make"
                                  "sure you are not losing data." ).arg( url.path() ),
                            i18n( "Conflict" ) );
        return false;
    }

    if ( reason == 3 )                                 // means the file was deleted
    {
        KMessageBox::sorry( TopLevel::getInstance() ->main(),
                            i18n( "Warning: The file \"%1\" has been deleted on"
                                  "disk.\n\n"
                                  "If this was not your intention, make sure to"
                                  "save this file now." ).arg( url.path() ),
                            i18n( "Document Deleted" ) );
        return false;
    }

    if ( dirtyAction == "alert" )
    {
        if ( KMessageBox::warningYesNo( TopLevel::getInstance() ->main(),
                                        i18n( "The file \"%1\" has changed on"
                                              "disk.\n\nDo you want to reload it?" ).arg( url.path() ),
                                        i18n( "Document Changed" ) ) == KMessageBox::No )
        {
            return false;
        }
    }

    // here we either answered yes above or are in autoreload mode
    reloadDocument( url );

    return true;
}

KURL DocumentController::storedURLForPart( KParts::ReadOnlyPart * ro_part )
{
    if ( m_partURLMap.contains( ro_part ) )
    {
        return m_partURLMap[ ro_part ];
    }
    return KURL();
}

void DocumentController::updatePartURL( KParts::ReadOnlyPart * ro_part )
{
    if ( ro_part->url().isEmpty() )
    {
        kdDebug( 9000 ) << "updatePartURL() called with empty URL for part: "
        << ro_part << endl;
        return ;
    }
    m_partURLMap[ ro_part ] = ro_part->url();
}

bool DocumentController::partURLHasChanged( KParts::ReadOnlyPart * ro_part )
{
    if ( m_partURLMap.contains( ro_part ) && !ro_part->url().isEmpty() )
    {
        if ( m_partURLMap[ ro_part ] != ro_part->url() )
        {
            emit documentURLChanged( m_partURLMap[ ro_part ], ro_part->url() );
            return true;
        }
    }
    return false;
}

bool DocumentController::saveDocumentsDialog( KURL::List const & ignoreList )
{
    KURL::List modList = modifiedDocuments();

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

bool DocumentController::closeDocumentsDialog( KURL::List const & ignoreList )
{
    if ( !saveDocumentsDialog( ignoreList ) )
        return false;

    foreach (KParts::Part* part, parts())
    {
        KParts::ReadOnlyPart * ro_part = readOnly( part );
        if ( ro_part && !ignoreList.contains( ro_part->url() ) || !ro_part )
        {
            closePart( part );
        }
    }
    return true;
}

KParts::ReadOnlyPart* DocumentController::activeReadOnly() const
{
    return readOnly( activePart() );
}

KParts::ReadWritePart* DocumentController::activeReadWrite() const
{
    return readWrite( activePart() );
}

KParts::ReadOnlyPart* DocumentController::readOnly( KParts::Part *part ) const
{
    return qobject_cast<KParts::ReadOnlyPart*>( part );
}

KParts::ReadWritePart* DocumentController::readWrite( KParts::Part *part ) const
{
    return qobject_cast<KParts::ReadWritePart*>( part );
}

KParts::ReadWritePart* DocumentController::readWriteForURL( const KURL& url ) const
{
    return qobject_cast<KParts::ReadWritePart*>( partForURL( url ) );
}

DocumentController::HistoryEntry::HistoryEntry( const KURL & u, int l, int c )
        : url( u ), line( l ), col( c )
{
    // should provide a reasonably unique number
    id = abs( QTime::currentTime().msecsTo( QTime() ) );
}

DocumentController::HistoryEntry DocumentController::createHistoryEntry()
{
    KTextEditor::Document * doc = textPartForURL( activeDocument() );
    if ( !doc )
        return HistoryEntry();

    KTextEditor::View *view =
        qobject_cast<KTextEditor::View *>( doc->activeView() );
    if ( !view )
        return HistoryEntry();

    KTextEditor::Cursor cursor = view->cursorPosition();
    return HistoryEntry( activeDocument(), cursor.line(), cursor.column() );
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
    editDocument( entry.url, entry.line, entry.col );
    m_isJumping = false;
}

#include "documentcontroller.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
