#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <qpopupmenu.h>
#include <qfile.h>
#include <qlayout.h>
#include <qmap.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qcheckbox.h>

#include <kmimetype.h>
#include <kservice.h>
#include <ktrader.h>
#include <kapplication.h>
#include <krun.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kparts/part.h>
#include <kparts/factory.h>
#include <kparts/partmanager.h>
#include <kparts/browserextension.h>
#include <kfiledialog.h>
#include <kmainwindow.h>
#include <kaction.h>
#include <kstatusbar.h>
#include <khtml_part.h>
#include <kpopupmenu.h>
#include <kio/netaccess.h>
#include <kdialogbase.h>
#include <klineedit.h>
#include <kshortcut.h>
#include <kcompletion.h>
#include <kdirwatch.h>
#include <kdeversion.h>
#include <kiconloader.h>
#include <kuserprofile.h>

#include <ktexteditor/view.h>
#include <ktexteditor/document.h>
#include <ktexteditor/viewcursorinterface.h>

#include "toplevel.h"
#include "api.h"
#include "core.h"
#include "editorproxy.h"
#include "documentationpart.h"
#include "ksavealldialog.h"

#include "kdevproject.h"
#include "urlutil.h"
#include "mimewarningdialog.h"

#include "partcontroller.h"

PartController *PartController::s_instance = 0;

using namespace MainWindowUtils;

struct HistoryEntry {
    KURL url;
    QString context;

    HistoryEntry( const KURL& u, const QString& c ): url( u ), context( c ) {}
};

PartController::PartController(QWidget *parent)
  : KDevPartController(parent), _editorFactory(0L)
{
  dirWatcher = new KDirWatch( this );

//  connect(this, SIGNAL(partRemoved(KParts::Part*)), this, SLOT(updateMenuItems()));
//  connect(this, SIGNAL(partAdded(KParts::Part*)), this, SLOT(updateMenuItems()));
  connect(this, SIGNAL(partRemoved(KParts::Part*)), this, SLOT(slotPartRemoved(KParts::Part* )) );
  connect(this, SIGNAL(partAdded(KParts::Part*)), this, SLOT(slotPartAdded(KParts::Part* )) );
  connect(this, SIGNAL(activePartChanged(KParts::Part*)), this, SLOT(slotActivePartChanged(KParts::Part*)));
  connect(dirWatcher, SIGNAL(dirty(const QString&)), this, SLOT(dirty(const QString&)));
  connect(this, SIGNAL(fileDirty(const KURL& )), this, SLOT(slotFileDirty(const KURL&)) );

  setupActions();
  
  m_Current = m_history.end();
  m_isJumping = false;
  
  m_openNextAsText = false;
}


PartController::~PartController()
{
}


void PartController::createInstance(QWidget *parent)
{
  if (!s_instance)
    s_instance = new PartController(parent);
}


PartController *PartController::getInstance()
{
  return s_instance;
}


void PartController::setupActions()
{
  KActionCollection *ac = TopLevel::getInstance()->main()->actionCollection();

  KAction* newAction = KStdAction::open(this, SLOT(slotOpenFile()),
    ac, "file_open");
  newAction->setToolTip( i18n("Open file") );
  newAction->setWhatsThis( i18n("<b>Open file</b><p>Opens an existing file without adding it to the project.</p>") );

  m_openRecentAction = KStdAction::openRecent( this, SLOT(slotOpenRecent(const KURL&) ),
    ac, "file_open_recent" );
  m_openRecentAction->setWhatsThis(QString("<b>%1</b><p>%2").arg(beautifyToolTip(m_openRecentAction->text())).arg(i18n("Opens recently opened file.")));
  m_openRecentAction->loadEntries( kapp->config(), "RecentFiles" );

  m_saveAllFilesAction = new KAction(i18n("Save Al&l"), 0,
    this, SLOT(slotSaveAllFiles()),
    ac, "file_save_all");
  m_saveAllFilesAction->setToolTip( i18n("Save all modified files") );
  m_saveAllFilesAction->setWhatsThis(i18n("<b>Save all</b><p>Saves all modified files."));
  m_saveAllFilesAction->setEnabled(false);

  m_revertAllFilesAction = new KAction(i18n("Rever&t All"), 0,
    this, SLOT(slotRevertAllFiles()),
    ac, "file_revert_all");
  m_revertAllFilesAction->setToolTip(i18n("Revert all changes"));
  m_revertAllFilesAction->setWhatsThis(i18n("<b>Revert all</b><p>Reverts all changes in opened files. Prompts to save changes so the reversion can be canceled for each modified file."));
  m_revertAllFilesAction->setEnabled(false);

  m_closeWindowAction = KStdAction::close(
    this, SLOT(slotCloseWindow()),
    ac, "file_close");
  m_closeWindowAction->setToolTip( i18n("Close current file") );
  m_closeWindowAction->setWhatsThis(QString("<b>%1</b><p>%2").arg(beautifyToolTip(m_closeWindowAction->text())).arg(i18n("Closes current file.")));
  m_closeWindowAction->setEnabled(false);

  m_closeAllWindowsAction = new KAction(i18n("Close All"), 0,
    this, SLOT(slotCloseAllWindows()),
    ac, "file_close_all");
  m_closeAllWindowsAction->setToolTip( i18n("Close all files") );
  m_closeAllWindowsAction->setWhatsThis(i18n("<b>Close all</b><p>Close all opened files."));
  m_closeAllWindowsAction->setEnabled(false);

  m_closeOtherWindowsAction = new KAction(i18n("Close All Others"), 0,
    this, SLOT(slotCloseOtherWindows()),
    ac, "file_closeother");
  m_closeOtherWindowsAction->setToolTip( i18n("Close other files") );
  m_closeOtherWindowsAction->setWhatsThis(i18n("<b>Close all others</b><p>Close all opened files except current."));
  m_closeOtherWindowsAction->setEnabled(false);

  m_switchToAction = new KAction(i18n("Switch To..."), KShortcut("CTRL+/"),
    this, SLOT(slotSwitchTo()),
    ac, "file_switchto");
  m_switchToAction->setToolTip(i18n("Switch to"));
  m_switchToAction->setWhatsThis(i18n("<b>Switch to</b><p>Prompts to enter the name of previously opened file to switch to."));

  new KActionSeparator(ac, "dummy_separator");
  
  m_backAction = new KToolBarPopupAction(i18n("Back"), "back", 0, this, SLOT(slotBack()), ac, "history_back");
  m_backAction->setEnabled( false );
  m_backAction->setToolTip(i18n("Back"));
  m_backAction->setWhatsThis(i18n("<b>Back</b><p>Moves backwards one step in the navigation history."));


  connect(m_backAction->popupMenu(), SIGNAL(aboutToShow()),
         this, SLOT(slotBackAboutToShow()));
  connect(m_backAction->popupMenu(), SIGNAL(activated(int)),
         this, SLOT(slotPopupActivated(int)));


  m_forwardAction = new KToolBarPopupAction(i18n("Forward"), "forward", 0, this, SLOT(slotForward()), ac, "history_forward");
  m_forwardAction->setEnabled( false );
  m_forwardAction->setToolTip(i18n("Forward"));
  m_forwardAction->setWhatsThis(i18n("<b>Forward</b><p>Moves forward one step in the navigation history."));

  connect(m_forwardAction->popupMenu(), SIGNAL(aboutToShow()),
         this, SLOT(slotForwardAboutToShow()));
  connect(m_forwardAction->popupMenu(), SIGNAL(activated(int)),
         this, SLOT(slotPopupActivated(int)));
  
}


void PartController::setEncoding(const QString &encoding)
{
  m_presetEncoding = encoding;
}

KParts::Part* PartController::findOpenDocument(const KURL& url)
{
	// if we find it this way, all is well
	KParts::Part * part = partForURL( url );
	if ( part )
	{
		return part;
	}
	
	// ok, let's see if we can try harder
	if ( API::getInstance()->project() )
	{
		KURL partURL = findURLInProject( url );
		partURL.cleanPath();
		return partForURL( partURL );
	}
	
	return 0L;
}

KURL PartController::findURLInProject(const KURL& url)
{
  QStringList fileList = API::getInstance()->project()->allFiles();

  bool filenameOnly = (url.url().find('/') == -1);
  QString filename = filenameOnly ? "/" : "";
  filename += url.url();

  for (QStringList::Iterator it = fileList.begin(); it != fileList.end(); ++it) {
    if ((*it).endsWith(filename)) {
      // Match! The first one is as good as any one, I guess...
      return KURL( API::getInstance()->project()->projectDirectory() + "/" + *it );
    }
  }

  return url;
}

void PartController::editDocument(const KURL &inputUrl, int lineNum, int col)
{
  kdDebug(9000) << k_funcinfo << inputUrl.prettyURL() << " linenum " << lineNum << endl;

  KURL url = inputUrl;

  // Make sure the URL exists
	if ( !url.isValid() || !KIO::NetAccess::exists(url, false, 0) ) 
	{
		// Try to find this file in the current project's list instead
		if ( API::getInstance()->project() ) 
		{
			url = findURLInProject(url);
		
			if ( !url.isValid() || !KIO::NetAccess::exists(url, false, 0) ) 
			{
				// See if this url is relative to the current project's directory
				url = API::getInstance()->project()->projectDirectory() + "/" + url.path();
			}
		}
	
		if ( !url.isValid() || !KIO::NetAccess::exists(url, false, 0) ) 
		{
			// Not found - prompt the user to find it?
			kdDebug(9000) << "cannot find URL: " << url.url() << endl;
			return;
		}
	}

  // We now have a url that exists ;)

  // clean it and resolve possible symlink
	url.cleanPath(true);
	if (url.isLocalFile())
	{
		QString path = url.path();
		path = URLUtil::canonicalPath(path);
		if ( !path.isEmpty() )
			url.setPath(path);
	}

	// is it already open?
	KParts::Part *existingPart = partForURL(url);
	if (existingPart)
	{
		activatePart(existingPart);
		EditorProxy::getInstance()->setLineNumber(existingPart, lineNum, col);
		addHistoryEntry( url, lineNum, col );
		return;
	}
	
	KMimeType::Ptr MimeType = KMimeType::findByURL( url );
	
	kdDebug(9000) << "mimeType = " << MimeType->name() << endl;
	
	// is the URL pointing to a directory?
	if ( MimeType->is( "inode/directory" ) )
	{
		return;
	}
  
	if ( !m_presetEncoding.isNull() )
	{
		m_openNextAsText = true;
	}
	
	// we generally prefer embedding, but if Qt-designer is the preferred application for this mimetype
	// make sure we launch designer instead of embedding KUIviewer
	if ( !m_openNextAsText && MimeType->is( "application/x-designer" ) )
	{
		KService::Ptr preferredApp = KServiceTypeProfile::preferredService( MimeType->name(), "Application" );
		if ( preferredApp && preferredApp->desktopEntryName() == "designer" )
		{
			KRun::runURL(url, MimeType->name() );
			return;
		}
	}
	
	KConfig *config = kapp->config();
	config->setGroup("General");
	QStringList texttypeslist = config->readListEntry( "TextTypes" );
	if ( texttypeslist.contains( MimeType->name() ) )
	{
		m_openNextAsText = true;
	}
  
	// is this regular text - open in editor
	if ( m_openNextAsText || MimeType->is( "text/plain" ) || MimeType->is( "text/html" ) || MimeType->is( "application/x-zerosize" ) )
	{
		KTextEditor::Editor * editorpart = createEditorPart();

		if ( editorpart )
		{
			if ( !m_presetEncoding.isNull() )
			{
				KParts::BrowserExtension * extension = KParts::BrowserExtension::childObject( editorpart );
				if ( extension )
				{
					KParts::URLArgs args;
					args.serviceType = QString( "text/plain;" ) + m_presetEncoding;
					extension->setURLArgs(args);
				}
				m_presetEncoding = QString::null;
			}
				
			editorpart->openURL( url );

			integratePart( editorpart, url, true );
			EditorProxy::getInstance()->setLineNumber( editorpart, lineNum, col );
			addHistoryEntry( url, lineNum, col );

			m_openNextAsText = false;
			
			m_openRecentAction->addURL( url );
			m_openRecentAction->saveEntries( kapp->config(), "RecentFiles" );

			return;
		}
	}
	
	// OK, it's not text and it's not a designer file.. let's see what else we can come up with..
	
	KParts::Factory *factory = 0;
	QString className;
	
	QString services[] = { "KParts/ReadWritePart", "KParts/ReadOnlyPart" };
	QString classnames[] = { "KParts::ReadWritePart", "KParts::ReadOnlyPart" };
	for (uint i=0; i<2; ++i)
	{
		factory = findPartFactory( MimeType->name(), services[i] );
		if (factory)
		{
			className = classnames[i];
			break;
		}
	}
	
	kdDebug(9000) << "factory = " << factory << endl;

	if (factory)
	{
		// create the object of the desired class
		KParts::ReadOnlyPart *part = static_cast<KParts::ReadOnlyPart*>( factory->createPart( TopLevel::getInstance()->main(), 0, 0, 0, className.latin1() ) );
		if ( part )
		{
			part->openURL( url );
			integratePart( part, url, false );
			addHistoryEntry( url, lineNum, col );
		
			m_openRecentAction->addURL( url );
			m_openRecentAction->saveEntries( kapp->config(), "RecentFiles" );
		}
	}
	else
	{
		MimeWarningDialog dlg;
		dlg.text->setText( dlg.text->text().arg(url.path()).arg(MimeType->name()) );
		
		if ( dlg.exec() == QDialog::Accepted )
		{
			if ( dlg.open_with_kde->isChecked() )
			{
				KRun::runURL(url, MimeType->name() );
			}
			else
			{
				if ( dlg.always_open_as_text->isChecked() )
				{
					KConfig *config = kapp->config();  
					config->setGroup("General");
					QStringList texttypeslist = config->readListEntry( "TextTypes" );
					texttypeslist << MimeType->name();
					config->writeEntry( "TextTypes", texttypeslist );
				}
				m_openNextAsText = true;
				editDocument( url, lineNum, col );
			}
		}
	}
}


void PartController::showDocument(const KURL &url, bool newWin)
{
  QString fixedPath = DocumentationPart::resolveEnvVarsInURL(url.url()); // possibly could env vars
  KURL docUrl(fixedPath);
  kdDebug(9000) << "SHOW: " << docUrl.url() << endl;

  if ( docUrl.isLocalFile() && KMimeType::findByURL(docUrl)->name() != "text/html" ) {
    // a link in a html-file pointed to a local text file - display
    // it in the editor instead of a html-view to avoid uglyness
    editDocument( docUrl );
    return;
  }
  
  
  DocumentationPart *part = dynamic_cast<DocumentationPart*>(activePart());
  if (!part || newWin)
  {
    part = new DocumentationPart;
    integratePart(part,docUrl);
    connect(part, SIGNAL(fileNameChanged(KParts::ReadOnlyPart* )),
        this, SIGNAL(partURLChanged(KParts::ReadOnlyPart* )));
  }
  else
  {
    activatePart(part);
  }
  part->openURL(docUrl);
  

  //adymo: context has gone
/*  DocumentationPart *part = 0;
  
  if (!context.isEmpty())
    part = findDocPart(context);

  if (!part)
  {
    part = new DocumentationPart;
    part->setContext(context);
    integratePart(part,docUrl);
  }
  else
    activatePart(part);
  
  bool bSuccess = part->openURL(docUrl);
  if (!bSuccess) {
    // part->showError(...);
  }*/
}


DocumentationPart *PartController::findDocPart(const QString &context)
{
  QPtrListIterator<KParts::Part> it(*parts());
  for ( ; it.current(); ++it)
  {
    DocumentationPart *part = dynamic_cast<DocumentationPart*>(it.current());
    if (part && (part->context() == context))
      return part;
  }

  return 0;
}


KParts::Factory *PartController::findPartFactory(const QString &mimeType, const QString &partType, const QString &preferredName)
{
  KTrader::OfferList offers = KTrader::self()->query(mimeType, QString("'%1' in ServiceTypes").arg(partType));

  if (offers.count() > 0)
  {
    KService::Ptr ptr = 0;
    // if there is a preferred plugin we'll take it
    if ( !preferredName.isEmpty() ) {
      KTrader::OfferList::Iterator it;
      for (it = offers.begin(); it != offers.end(); ++it) {
        if ((*it)->name() == preferredName) {
          ptr = (*it);
        }
      }
    }
    // else we just take the first in the list
    if ( !ptr ) {
      ptr = offers.first();
    }
    return static_cast<KParts::Factory*>(KLibLoader::self()->factory(QFile::encodeName(ptr->library())));
  }

  return 0;
}

KTextEditor::Editor * PartController::createEditorPart( )
{
	
	if ( !_editorFactory )
	{
		kapp->config()->setGroup("Editor");
		QString preferred = kapp->config()->readPathEntry("EmbeddedKTextEditor");
		
		_editorFactory = findPartFactory( "text/plain", "KTextEditor/Document", preferred );
		
		if ( !_editorFactory ) return 0L;
	}
	
	return static_cast<KTextEditor::Editor*>( _editorFactory->createPart( TopLevel::getInstance()->main(), 0, 0, 0, "KTextEditor/Editor" ) );
}

void PartController::integratePart(KParts::Part *part, const KURL &url, bool isTextEditor )
{
  if (!part->widget()) {
    /// @todo error handling
      kdDebug(9000) << "no widget for this part!!" << endl;
      return; // to avoid later crash
  }

  TopLevel::getInstance()->embedPartView(part->widget(), url.filename(), url.url());

  addPart(part);

  if( isTextEditor )
  {
      EditorProxy::getInstance()->installPopup(part, contextPopupMenu());
  }

  // tell the parts we loaded a document
  KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(part);
  if ( !ro_part ) return;
  
  emit loadedFile( ro_part->url() );
  
  if ( ro_part->url().isLocalFile() ) 
  {
	updateTimestamp( ro_part->url() );
	emit loadedFile(ro_part->url().path());
  }

  // let's get notified when a document has been changed
  connect(part, SIGNAL(completed()), this, SLOT(slotUploadFinished()));
  
  // yes, we're cheating again. this signal exists for katepart's 
  // Document object and our DocumentationPart
//  connect(part, SIGNAL(fileNameChanged()), this, SLOT(slotFileNameChanged()));

  // Connect to the document's views newStatus() signal in order to keep track of the
  // modified-status of the document.

  // What's potentially problematic is that this signal isn't officially part of the
  // KTextEditor::View interface. It is nevertheless there, and used in kate and kwrite.
  // There doesn't seem to be any othere way of making this work with katepart, and since
  // signals are dynamic, if we try to connect to an editorpart that lacks this signal,
  // all we get is a runtime warning. At this point in time we are only really supported
  // by katepart anyway so IMHO this hack is justified. //teatime
  if (isTextEditor)
  {
    KTextEditor::Document * doc = static_cast<KTextEditor::Document*>( part );
    QPtrList<KTextEditor::View> list = doc->views();
    QPtrListIterator<KTextEditor::View> it( list );
    while ( it.current() )
    {
      connect( it, SIGNAL( newStatus() ), this, SLOT( slotNewStatus() ) );
      ++it;
    }
  }
}

void PartController::slotPartAdded( KParts::Part * part )
{
	kdDebug(9000) << k_funcinfo << endl;
	
	if ( KParts::ReadOnlyPart * ro_part = dynamic_cast<KParts::ReadOnlyPart*>( part ) )
	{
		updatePartURL( ro_part );
	}
	
	updateMenuItems();
}

void PartController::slotPartRemoved( KParts::Part * part )
{
	kdDebug(9000) << k_funcinfo << endl;
	
	_partURLMap.remove( static_cast<KParts::ReadOnlyPart*>(part) );
	
	updateMenuItems();
}

void PartController::updatePartURL( KParts::ReadOnlyPart * ro_part )
{
	if ( ro_part->url().isEmpty() )
	{
		kdDebug(9000) << "updatePartURL() called with empty URL for part: " << ro_part << endl;
		return;
	}
	_partURLMap[ ro_part ] = ro_part->url();
}

bool PartController::partURLHasChanged( KParts::ReadOnlyPart * ro_part )
{
	if ( _partURLMap.contains( ro_part ) )
	{
		if ( _partURLMap[ ro_part ] != ro_part->url() )
		{
			return true;
		}
	}
	return false;
}

KURL PartController::storedURLForPart( KParts::ReadOnlyPart * ro_part )
{
	if ( _partURLMap.contains( ro_part ) )
	{
		return _partURLMap[ ro_part ];
	}
	return KURL();
}


void PartController::slotUploadFinished()
{
	kdDebug(9000) << k_funcinfo << endl;
	
	KParts::ReadOnlyPart *ro_part = const_cast<KParts::ReadOnlyPart*>( dynamic_cast<const KParts::ReadOnlyPart*>(sender()) );
	if ( !ro_part ) return;
	
	if ( ro_part->url().isLocalFile() && isDirty( ro_part->url() ) ) 
	{
		emit savedFile( ro_part->url() );
		emit savedFile( ro_part->url().path() ); // @todo kill this one
		emit fileDirty( ro_part->url() );

		updateTimestamp( ro_part->url() );
	}

	if ( partURLHasChanged( ro_part ) )
	{
		emit partURLChanged( ro_part );
		
		removeTimestamp( storedURLForPart( ro_part ) );
		updatePartURL( ro_part );
		updateTimestamp( ro_part->url() );
	}	
	
}
/*
void PartController::slotFileNameChanged()
{
	kdDebug(9000) << k_funcinfo << endl;

	const KParts::ReadOnlyPart *ro_part = dynamic_cast<const KParts::ReadOnlyPart*>(sender());
	if ( !ro_part ) return; 
	
	emit partURLChanged( const_cast<KParts::ReadOnlyPart*>(ro_part) );

	if ( !ro_part->url().isLocalFile() ) return;
	
	updateTimestamp( ro_part->url() );

	emit fileDirty( ro_part->url() );
}
*/

void PartController::reinstallPopups( ){

  EditorProxy* editorProxy = EditorProxy::getInstance();
  QPopupMenu* popup = contextPopupMenu();

  QPtrListIterator<KParts::Part> it(*parts());
  for ( ; it.current(); ++it)
    editorProxy->installPopup( it.current(), popup, true );
}

QPopupMenu *PartController::contextPopupMenu()
{
    QPopupMenu * popup = (QPopupMenu*)(TopLevel::getInstance()->main())->factory()->container("rb_popup", TopLevel::getInstance()->main());

  kdDebug( 9000 ) << "PartController::contextPopupMenu() will return " << popup << endl;

  return popup;
}

KParts::ReadOnlyPart *PartController::partForURL(const KURL &url)
{
	QPtrListIterator<KParts::Part> it(*parts());
	for ( ; it.current(); ++it)
	{
		KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(it.current());
		if (ro_part && url == ro_part->url())
			return ro_part;
	}
	return 0;
}

KParts::Part * PartController::partForWidget( const QWidget * widget )
{
	QPtrListIterator<KParts::Part> it(*parts());
	for ( ; it.current(); ++it)
	{
		if ( it.current()->widget() == widget )
		{
			return *it;
		}
	}
	return 0;
}


void PartController::activatePart(KParts::Part *part)
{
	if ( !part ) return;

  setActivePart(part);

  if (part->widget())
  {
    TopLevel::getInstance()->raiseView(part->widget());
    part->widget()->setFocus();
  }
}

bool PartController::closePart(KParts::Part *part)
{
	if ( !part ) return true;

	if ( KParts::ReadOnlyPart * ro_part = dynamic_cast<KParts::ReadOnlyPart*>( part ) )
	{
		KURL url;
		if ( ! ro_part->closeURL() )
		{
			return false;
		}
		removeTimestamp( url );
	}
	
  // If we didn't call removePart(), KParts::PartManager::slotObjectDestroyed would
  // get called from the destroyed signal of the part being deleted below.
  // The call chain from that looks like this:
  // QObject::destroyed()
  //   KParts::PartManager::slotWidgetDestroyed() -> setActivePart() -> activePartChanged()
  //     TopLevelXXX::createGUI()
  //       KXMLGUIFactory::removeClient()
  // But then KXMLGUIFactory tries to remove the already-deleted part.
  // Normally this would work, because the factory uses a QGuardedPtr to the part.
  // But the QGuardedPtr is connected to the _same_ destroyed() slot that got us to
  // that point (slots are called in an undefined order)!

  // The following line can be removed with kdelibs HEAD! (2002-05-26)
//  removePart( part );

  if (part->widget())
    TopLevel::getInstance()->removeView(part->widget());

  delete part;

  return true;
}


void PartController::updateMenuItems()
{
  bool hasWriteParts = false;
  bool hasReadOnlyParts = false;

  QPtrListIterator<KParts::Part> it(*parts());
  for ( ; it.current(); ++it)
  {
    if (it.current()->inherits("KParts::ReadWritePart"))
      hasWriteParts = true;
    if (it.current()->inherits("KParts::ReadOnlyPart"))
      hasReadOnlyParts = true;
  }

  m_saveAllFilesAction->setEnabled(hasWriteParts);
  m_revertAllFilesAction->setEnabled(hasWriteParts);
  m_closeWindowAction->setEnabled(hasReadOnlyParts);
  m_closeAllWindowsAction->setEnabled(hasReadOnlyParts);
  m_closeOtherWindowsAction->setEnabled(hasReadOnlyParts);

  m_backAction->setEnabled( m_Current != m_history.begin() );
  m_forwardAction->setEnabled( m_Current != m_history.fromLast() );
}


void PartController::slotSaveAllFiles()
{
  saveAllFiles();
}


void PartController::saveFile(KParts::Part *part)
{
  KParts::ReadWritePart *rw_part = dynamic_cast<KParts::ReadWritePart*>(part);
  if ( !rw_part )
    return;
  if ( isDirty( rw_part->url() ) ) {
    kdDebug(9000) << "DIRTY SAVE" << endl;
  }

  if( rw_part->isModified() ) {
    rw_part->save();
    TopLevel::getInstance()->statusBar()->message(i18n("Saved %1").arg(rw_part->url().prettyURL()), 2000);
  }
}

void PartController::saveAllFiles()
{
  QPtrListIterator<KParts::Part> it(*parts());
  for ( ; it.current(); ++it)
    saveFile( it.current() );
}


void PartController::slotRevertAllFiles()
{
  revertAllFiles();
}

void PartController::revertFile(KParts::Part *part)
{
  if ( !part )
    return;

  if (part->inherits("KParts::ReadWritePart")) {
      KParts::ReadWritePart *rw_part = static_cast<KParts::ReadWritePart*>(part);
      if ( rw_part->url().isLocalFile() )
      {
          updateTimestamp( rw_part->url() );
      }
      rw_part->openURL(rw_part->url());
    }
}

void PartController::revertAllFiles()
{
  QPtrListIterator<KParts::Part> it(*parts());
  for ( ; it.current(); ++it)
    revertFile( it.current() );
}


void PartController::slotCloseWindow()
{
	closePart( activePart() );
}

KURL::List PartController::modifiedDocuments()
{
	KURL::List modFiles;
	
	QPtrListIterator<KParts::Part> it( *parts() );
	while( it.current() )
	{
		KParts::ReadWritePart *rw_part = dynamic_cast<KParts::ReadWritePart*>(it.current());
		if ( rw_part && rw_part->isModified() )
		{
			modFiles << rw_part->url();
		}
		++it;
	}
	return modFiles;
}

void PartController::saveFiles( KURL::List const & filelist )
{
	KURL::List::ConstIterator it = filelist.begin();
	while ( it != filelist.end() )
	{
		KParts::ReadWritePart * rw_part = dynamic_cast<KParts::ReadWritePart*>( partForURL( *it ) );
		if ( rw_part )
		{
			rw_part->save();
		}
		++it;
	}
}

void PartController::clearModified( KURL::List const & filelist )
{
	KURL::List::ConstIterator it = filelist.begin();
	while ( it != filelist.end() )
	{
		KParts::ReadWritePart * rw_part = dynamic_cast<KParts::ReadWritePart*>( partForURL( *it ) );
		if ( rw_part )
		{
			rw_part->setModified( false );
		}
		++it;
	}
}

bool PartController::closeFilesDialog( KURL::List const & ignoreList )
{
	KURL::List modList = modifiedDocuments();
	
	if ( modList.count() > 0 && modList != ignoreList ) 
	{
		KSaveSelectDialog dlg( modList, ignoreList, TopLevel::getInstance()->main() );
		if ( dlg.exec() == QDialog::Accepted )
		{
			saveFiles( dlg.filesToSave() );
			clearModified( dlg.filesNotToSave() );
		}
		else
		{
			return false;
		}
	}

	QPtrList<KParts::Part> partList( *parts() );
	QPtrListIterator<KParts::Part> it( partList );
	while ( KParts::Part* part = it.current() )
	{
		KParts::ReadOnlyPart * ro_part = dynamic_cast<KParts::ReadOnlyPart*>( part );
		if ( ro_part && !ignoreList.contains( ro_part->url() ) || !ro_part )
		{
			closePart( part );
		}
		++it;
	}
	
	return true;
}

bool PartController::closeFiles( const KURL::List & list )
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

bool PartController::closeAllFiles()
{
	return closeFilesDialog( KURL::List() );
}

void PartController::slotCloseAllWindows()
{
	closeAllFiles();
}

void PartController::slotCloseOtherWindows()
{
	KParts::ReadOnlyPart * active = dynamic_cast<KParts::ReadOnlyPart*>(activePart());
	if ( !active ) return;

	KURL::List ignoreList;
	ignoreList.append( active->url() );
	
	closeFilesDialog( ignoreList );
}

void PartController::slotCurrentChanged(QWidget *)
{
	kdDebug(9000) << k_funcinfo << " - Doing nothing!!" << endl;
/*	
  QPtrListIterator<KParts::Part> it(*parts());
  for ( ; it.current(); ++it)
    if (it.current()->widget() == w)
    {
      setActivePart(it.current(), w);
      break;
    }
*/	
}

void PartController::slotOpenFile()
{
  KURL::List fileNames = KFileDialog::getOpenURLs(QString::null, QString::null, TopLevel::getInstance()->main(), QString::null);

  for ( KURL::List::Iterator it = fileNames.begin(); it != fileNames.end(); ++it )
  {
    editDocument( *it );
  }
}

void PartController::slotOpenRecent( const KURL& url )
{
  editDocument( url );
  // stupid bugfix - don't allow an active item in the list
  m_openRecentAction->setCurrentItem( -1 );
}

bool PartController::readyToClose()
{
	return closeAllFiles();
}

void PartController::slotActivePartChanged( KParts::Part * part )
{
	updateMenuItems();

	if ( m_isJumping ) return;

	if ( _partURLMap.contains( m_latestPart ) )
	{
		addHistoryEntry( _partURLMap[ m_latestPart ] );
	}

	if ( dynamic_cast<DocumentationPart*>( part ) ) return;

	KParts::ReadOnlyPart * ro_part = dynamic_cast<KParts::ReadOnlyPart*>( part );
	if ( ro_part )
	{
		m_latestPart = ro_part;
	}
}

void PartController::slotSwitchTo()
{
    QMap<QString,KParts::ReadOnlyPart*> parts_map;
    QStringList part_list;
    QPtrList<KParts::Part> pl = *parts();
    KParts::Part *part;
    for(part=pl.first();part;part=pl.next()) {
        kdDebug(9000) << "Part..." << endl;
	if (part->inherits("KParts::ReadOnlyPart")) {
	    KParts::ReadOnlyPart *ro_part = static_cast<KParts::ReadOnlyPart*>(part);
            QString name = ro_part->url().fileName();
	    part_list.append(name);
            parts_map[name] = ro_part;
            kdDebug(9000) << "Found part for URL " << ro_part->url().prettyURL() << endl;
	}
    }

    KDialogBase dialog(KDialogBase::Plain, i18n("Switch To..."), KDialogBase::Ok|KDialogBase::Cancel,
		       KDialogBase::Ok, 0, "Switch to", true);
    QGridLayout *grid = new QGridLayout( dialog.plainPage(), 2, 1, 10, 10);
    KLineEdit *editbox = new KLineEdit(dialog.plainPage());
    grid->addWidget(new QLabel( i18n("Switch to buffer:"), dialog.plainPage() ), 0, 0);
    grid->addWidget(editbox, 1, 0);
    editbox->completionObject()->setItems( part_list );
    editbox->setFocus();
    int result = dialog.exec();
    if (result==KDialogBase::KDialogBase::Accepted) {
        if (parts_map.contains(editbox->text())) {
            activatePart(parts_map[editbox->text()]);
        }
    }
}

void PartController::showPart( KParts::Part* part, const QString& name, const QString& shortDescription )
{
  if (!part->widget()) {
    /// @todo error handling
    return; // to avoid later crash
  }

  QPtrListIterator<KParts::Part> it(*parts());
  for ( ; it.current(); ++it)
  {
    if( it.current() == part ){
        // part already embedded
	activatePart( it.current() );
	return;
    }
  }

  // embed the part
  TopLevel::getInstance()->embedPartView( part->widget(), name, shortDescription );
  addPart( part );
}

void PartController::dirty( const QString& fileName )
{
	KURL url;
	url.setPath( fileName );
	emit fileDirty( url );
}

bool PartController::isDirty( KURL const & url )
{
	if ( !url.isLocalFile() ) return false;
	
	if ( accessTimeMap.contains( url ) )
	{
		return ( accessTimeMap[ url ] < QFileInfo( url.path() ).lastModified() );
	}
	
	return false;
}

void PartController::updateTimestamp( KURL const & url )
{
	if ( !accessTimeMap.contains( url ) )
	{
		dirWatcher->addFile( url.path() );
	}
	accessTimeMap[ url ] = QFileInfo( url.path() ).lastModified();
}

void PartController::removeTimestamp( KURL const & url )
{
	accessTimeMap.remove( url );
	dirWatcher->removeFile( url.path() );
}

void PartController::slotNewStatus( )
{
	kdDebug(9000) << k_funcinfo << endl;

    QObject * senderobj = const_cast<QObject*>( sender() );
    KTextEditor::View * view = dynamic_cast<KTextEditor::View*>( senderobj );
    if ( view )
    {
		doEmitState( view->document()->url() );
   }
}

DocumentState PartController::documentState( KURL const & url )
{
	KParts::ReadWritePart * rw_part = dynamic_cast<KParts::ReadWritePart*>( partForURL( url ) );
	if ( !rw_part ) return Clean;
	
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

void PartController::doEmitState( KURL const & url )
{
	emit documentChangedState( url, documentState( url ) );
}
 
void PartController::slotFileDirty( const KURL & url )
{
	kdDebug(9000) << k_funcinfo << endl;

	doEmitState( url );
}

KURL::List PartController::openURLs( )
{
	KURL::List list;
	QPtrListIterator<KParts::Part> it(*parts());
	for ( ; it.current(); ++it)
	{
		if ( KParts::ReadOnlyPart *ro_part = dynamic_cast<KParts::ReadOnlyPart*>(it.current()) )
		{
			list << ro_part->url();
		}
	}
	return list;
}

void PartController::revertFiles( const KURL::List &  )
{
#warning not implemented	
}

/////////////////////////////////////////////////////////////////////////////


PartController::HistoryEntry::HistoryEntry( const KURL & u, int l, int c) 
	: url(u), line(l), col(c)
{
	id = abs( QTime::currentTime().msecsTo( QTime() ) );	// should provide a reasonably unique number
}

void PartController::slotBack()
{
	if ( m_Current != m_history.begin() )
	{
		jumpTo( *(--m_Current) );
	}
}

void PartController::slotForward()
{
	if (  m_Current != m_history.fromLast() )
	{
		jumpTo( *(++m_Current) );
	}
}

void PartController::slotBackAboutToShow()
{
	KPopupMenu *popup = m_backAction->popupMenu();
	popup->clear();

	if ( m_Current == m_history.begin() ) return;

	QValueList<HistoryEntry>::Iterator it = m_Current;
	--it;
	
	int i = 0;
	while( i < 10 )
	{
		if ( it == m_history.begin() )
		{
			popup->insertItem( (*it).url.fileName() + QString(" (%1,%2)").arg( (*it).line).arg((*it).col), (*it).id );
			return;
		} 
		
		popup->insertItem( (*it).url.fileName() + QString(" (%1,%2)").arg( (*it).line).arg((*it).col), (*it).id );
		++i;
		--it;
	} 
}

void PartController::slotForwardAboutToShow()
{
	KPopupMenu *popup = m_forwardAction->popupMenu();
	popup->clear();

	if ( m_Current == m_history.fromLast() ) return;

	QValueList<HistoryEntry>::Iterator it = m_Current;
	++it;
	
	int i = 0;
	while( i < 10 )
	{
		if ( it == m_history.fromLast() )
		{
			popup->insertItem( (*it).url.fileName() + QString(" (%1,%2)").arg( (*it).line).arg((*it).col), (*it).id );
			return;
		} 
		
		popup->insertItem( (*it).url.fileName()  + QString(" (%1,%2)").arg( (*it).line).arg((*it).col), (*it).id );
		++i;
		++it;
	} 
}

void PartController::slotPopupActivated( int id )
{
	QValueList<HistoryEntry>::Iterator it = m_history.begin();
	while( it != m_history.end() )
	{
		if ( (*it).id == id )
		{
			m_Current = it;
			jumpTo( *m_Current );
			return;
		}
		++it;
	}
}

void PartController::jumpTo( const HistoryEntry & entry )
{
	m_isJumping = true;
	editDocument( entry.url, entry.line, entry.col );
	m_isJumping = false;
}

void PartController::addHistoryEntry(const KURL & url, int line, int col )
{
	if ( m_isJumping ) return;

	QValueList<HistoryEntry>::Iterator it = m_Current;
	// if We're not already the last entry, we truncate the list here before adding an entry
	if ( it != m_history.end() && it != m_history.fromLast() )
	{
		m_history.erase( ++it, m_history.end() );
	}
	
	HistoryEntry newEntry( url, line, col );
	
	// Only save the new entry if it is different from the last
	if ( newEntry.url == (*m_Current).url )
	{
		if ( newEntry.line == -1 )
		{
			return;
		}
		if ( (*m_Current).line == -1 )
		{
			(*m_Current).line = line;
			(*m_Current).col = col;
			return;
		}
	}
	
	// add entry
	m_history.append( newEntry );
	m_Current = m_history.fromLast();
	
	updateMenuItems();
}

#include "partcontroller.moc"
