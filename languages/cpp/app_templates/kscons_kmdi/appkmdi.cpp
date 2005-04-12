%{CPP_TEMPLATE}

#include <qdragobject.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qdragobject.h>
#include <qstatusbar.h>
#include <qtoolbutton.h>

#include <kdebug.h>
#include <kglobal.h>
#include <ktip.h>
#include <kparts/part.h>
#include <kparts/partmanager.h>

#include <kio/netaccess.h>
#include <kurl.h>
#include <kurldrag.h>
#include <kurlrequesterdlg.h>
#include <kconfig.h>
#include <kconfigdialog.h>
#include <klibloader.h>
#include <kaboutdata.h>
#include <kfiledialog.h>
#include <kactionclasses.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdeversion.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <kkeydialog.h>
#include <kmessagebox.h>
#include <ktabwidget.h>
#include <kedittoolbar.h>
#include <kdeversion.h>
#include <kstdaccel.h>
#include <kaction.h>
#include <kstdaction.h>

#include "settings.h"
#include "prefs.h"
#include "%{APPNAMELC}kmdiView.h"
#include "kmdikonsole.h"
#include "%{APPNAMELC}kmdi.h"

%{APPNAMELC}kmdi::%{APPNAMELC}kmdi(KMdi::MdiMode mode)
	: KMdiMainFrm( 0, "%{APPNAMELC}", mode )
{
	resize( 800, 600 ); // start at 800x600 the first time

	setManagedDockPositionModeEnabled(true);
	setStandardMDIMenuEnabled(false);

	m_manager = new KParts::PartManager(this);
	connect(m_manager, SIGNAL(activePartChanged(KParts::Part*)),
			this, SLOT(createGUI(KParts::Part*)) );

#if KDE_IS_VERSION(3, 3, 0)
	setToolviewStyle( KMdi::TextAndIcon );
	tabWidget()->setHoverCloseButton( false );
#endif

	setMenuForSDIModeSysButtons( menuBar() );
	setManagedDockPositionModeEnabled(true); 

	m_console = NULL;

	// accept dnd
	setAcceptDrops(true);

	// then, setup our actions
	setupActions();

	//createShellGUI( true );

	// and a status bar
	statusBar()->show();

	// apply the saved mainwindow settings, if any, and ask the mainwindow
	// to automatically save settings if changed: window size, toolbar
	// position, icon size, etc.
	setAutoSaveSettings();

	connect( this, SIGNAL( viewActivated( KMdiChildView * ) ),
			this, SLOT( currentChanged( KMdiChildView * ) ) );

	m_console = new kmdikonsole(this, "konsole");
	m_console->setIcon( SmallIcon("konsole") );
	m_console->setCaption( i18n("Terminal") );
	addToolWindow( m_console, KDockWidget::DockBottom, getMainDockWidget(), 20 );

	if (Settings::showCloseTabsButton())
	{
		QToolButton *but = new QToolButton(tabWidget());
		but->setIconSet(SmallIcon("tab_remove"));
		but->adjustSize();
		but->hide();
		connect(but, SIGNAL(clicked()), actionCollection()->action( "file_close" ), SIGNAL(activated()));
		tabWidget()->setCornerWidget(but, TopRight);
	}

	showTipOnStart();
}

%{APPNAMELC}kmdi::~%{APPNAMELC}kmdi()
{
	delete m_console;
}

void %{APPNAMELC}kmdi::setupActions()
{
	setXMLFile("%{APPNAMELC}ui.rc");

	KStdAction::openNew(this, SLOT(slotFileNew()), actionCollection());

	KStdAction::tipOfDay( this, SLOT( showTip() ), actionCollection() 
			)->setWhatsThis(i18n("This shows useful tips on the use of this application."));

	KStdAction::close(this, SLOT(slotFileClose()), actionCollection());
	KStdAction::quit(this, SLOT(slotFileQuit()), actionCollection());

	m_toolbarAction = KStdAction::showToolbar(this, SLOT(optionsShowToolbar()), actionCollection());
	m_statusbarAction = KStdAction::showStatusbar(this, SLOT(optionsShowStatusbar()), actionCollection());

	KStdAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
	KStdAction::preferences(this, SLOT(optionsPreferences()), actionCollection());

	KAction* action = KStdAction::configureToolbars(this, 
	         SLOT(optionsConfigureToolbars()), actionCollection());

	createGUI( NULL );
}

void %{APPNAMELC}kmdi::showTip() 
{
	KTipDialog::showTip(this,QString::null,true);
}

void %{APPNAMELC}kmdi::showTipOnStart()
{
	KTipDialog::showTip(this);
}

void %{APPNAMELC}kmdi::slotFileNew()
{
	%{APPNAMELC}kmdiView *view = new %{APPNAMELC}kmdiView(this);
	m_manager->addPart( view->part() );

	addWindow( view );

	// add the tree view to the widget stack
	m_views += view;

	currentChanged( view );
}

void %{APPNAMELC}kmdi::openURL(const KURL & url)
{
	// check if the url is not already opened first
	QValueList<%{APPNAMELC}kmdiView*>::iterator it  = m_views.begin();
	QValueList<%{APPNAMELC}kmdiView*>::iterator end = m_views.end();
	for (; it != end; ++it)
	{
		%{APPNAMELC}kmdiView *view = *it;
		if (view->part()->url() == url)
		{
			activateView(view);
			return;
		}
	}

	%{APPNAMELC}kmdiView *view = new %{APPNAMELC}kmdiView(this);
	m_manager->addPart( view->part() );

	addWindow( view );

	m_views += view;

	view->part()->openURL(url);
	currentChanged( view );
}

void %{APPNAMELC}kmdi::currentChanged( KMdiChildView *current )
{
	//kdWarning()<<"current view changed"<<endl;
	if (!current)
	{
		m_manager->setActivePart(NULL);
		//setCaption("()");
		return;
	}

	// switch to the corresponding document
	if ( m_views.contains( (%{APPNAMELC}kmdiView*) current ) )
	{
		%{APPNAMELC}kmdiView *view = (%{APPNAMELC}kmdiView*) current;
		//view->updateCaption();
		m_manager->setActivePart( view->part() );
	}
}

void %{APPNAMELC}kmdi::slotFileClose()
{
	requestClose( activeWindow() );
}

bool %{APPNAMELC}kmdi::requestClose(KMdiChildView* v)
{
	//    kdWarning()<<"closing view"<<v<<endl;
	//    if (view == NULL)
	//	return;

	if (v == NULL)
	{
		if ( m_views.count() == 1)
		{
			%{APPNAMELC}kmdiView *view = m_views[0];
			if (view->part()->queryClose())
			{
				m_manager->removePart( view->part() );

				closeWindow( view );
				m_views.clear();
				setCaption("kdissert");
				return true;
			}
			return false;
		}
		return true;
	}

	%{APPNAMELC}kmdiView *view = (%{APPNAMELC}kmdiView*) v;
	if ( m_views.contains( view ) )
	{
		m_views.remove( view );
		if ( view->part()->queryClose() )
		{
			m_manager->removePart( view->part() );

			closeWindow( view );
			return true;
		}
		return false;
	}
	return true;
}

void %{APPNAMELC}kmdi::slotFileQuit()
{
	close();
}

void %{APPNAMELC}kmdi::optionsShowToolbar()
{
	if (m_toolbarAction->isChecked())
		toolBar()->show();
	else
		toolBar()->hide();
}

void %{APPNAMELC}kmdi::optionsShowStatusbar()
{
	if (m_statusbarAction->isChecked())
		statusBar()->show();
	else
		statusBar()->hide();
}

void %{APPNAMELC}kmdi::optionsConfigureKeys()
{
	//KKeyDialog::configure(actionCollection());
	KKeyDialog dlg( false, this );
	QPtrList<KXMLGUIClient> clients = guiFactory()->clients();

	for( QPtrListIterator<KXMLGUIClient> it( clients ); it.current(); ++it )
		dlg.insert ( (*it)->actionCollection(), (*it)->instance()->aboutData()->programName() );

	dlg.configure();

	for (int i=0; i<m_views.count(); i++)
	{
		m_views[i]->part()->reloadXML();
	}
}

void %{APPNAMELC}kmdi::optionsConfigureToolbars()
{
	//saveMainWindowSettings(KGlobal::config(), autoSaveGroup());
}

void %{APPNAMELC}kmdi::newToolbarConfig()
{
	// This slot is called when user clicks "Ok" or "Apply" in the toolbar editor.
	// recreate our GUI, and re-apply the settings (e.g. "text under icons", etc.)
	//createGUI();
	//applyMainWindowSettings(KGlobal::config(), autoSaveGroup());
}

void %{APPNAMELC}kmdi::optionsPreferences()
{
	// The preference dialog is derived from prefs-base.ui which is subclassed into Prefs
	//
	// compare the names of the widgets in the .ui file 
	// to the names of the variables in the .kcfg file
	if (KConfigDialog::showDialog("settings"))
		return;	

	KConfigDialog *dialog = new KConfigDialog(this, "settings", Settings::self(), KDialogBase::Swallow);
	dialog->addPage(new prefs(), i18n("General"), "package_settings");
	connect(dialog, SIGNAL(settingsChanged()), this, SLOT(settingsChanged()));
	dialog->show();
}

void %{APPNAMELC}kmdi::settingsChanged()
{
	// propagate the changes to our views
	/*QValueList<%{APPNAMELC}kmdiView*>::iterator it  = m_views.begin();
	  QValueList<%{APPNAMELC}kmdiView*>::iterator end = m_views.end();
	  for (; it != end; ++it)
	  {
	  %{APPNAMELC}kmdiView *view = *it;
	  view->settingsChanged();
	  }*/
}

void %{APPNAMELC}kmdi::changeStatusbar(const QString& text)
{
	// display the text on the statusbar
	statusBar()->message(text, 2000);
}

void %{APPNAMELC}kmdi::dragEnterEvent(QDragEnterEvent *event)
{
	// accept uri drops only
	event->accept(KURLDrag::canDecode(event));
}

void %{APPNAMELC}kmdi::dropEvent(QDropEvent *event)
{
	KURL::List urls;

	// see if we can decode a URI.. if not, just ignore it
	if (KURLDrag::decode(event, urls) && !urls.isEmpty())
	{
		// okay, we have a URI.. process it
		const KURL &url = urls.first();

		// load in the file
		openURL(url);
	}
}

bool %{APPNAMELC}kmdi::queryClose()
{
	QValueList<%{APPNAMELC}kmdiView*>::iterator it;

	// check if we can close all documents
	for (it = m_views.begin(); it != m_views.end(); ++it)
	{
		// kdWarning()<<"searching for the part"<<endl;
		%{APPNAMELC}kmdiView *view = *it;
		if (!view->part()->queryClose())
		{
			return false;
		}
	}

	// and now close the documents
	for (it = m_views.begin(); it != m_views.end(); ++it)
	{
		// now close all views
		%{APPNAMELC}kmdiView *view = *it;

		m_manager->removePart( view->part() );
		closeWindow( view );
	}

	return true;
}

#include "%{APPNAMELC}kmdi.moc"
