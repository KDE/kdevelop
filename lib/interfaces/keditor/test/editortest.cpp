/*
 * Copyright (C) 2001  <kurt@granroth.org>
 */

#include "editortest.h"

#include <kkeydialog.h>
#include <kconfig.h>
#include <kurl.h>
#include <ktrader.h>
#include <kservice.h>
#include <kdebug.h>

#include <kedittoolbar.h>

#include <kaction.h>
#include <kstdaction.h>

#include <klibloader.h>
#include <kmessagebox.h>


using namespace KEditor;


EditorTest::EditorTest()
  : KParts::MainWindow( 0L, "EditorTest" )
{
  setupActions();
  
  setXMLFile("editortest_shell.rc");

  tabWidget = new QTabWidget(this);
  setCentralWidget(tabWidget);

  partManager = new KParts::PartManager(this);

  connect(partManager, SIGNAL(activePartChanged(KParts::Part*)), this, SLOT(createGUI(KParts::Part*)));

  createGUI(0);

  statusBar()->show();

  KTrader::OfferList offers = KTrader::self()->query(QString::fromLatin1("KDevelop/Editor"), QString::null);
  if (offers.isEmpty())
    return ;

  KService *service = *offers.begin();
  kdDebug(9000) << "Loading service " << service->name() << endl;

  KLibFactory *factory = KLibLoader::self()->factory(service->library());
  if (factory)
  {
    m_editor = static_cast < Editor* > (factory->create(this, "editor"));

    if (m_editor)
	{
	  KParts::MainWindow::factory()->addClient(m_editor);

	  connect(m_editor, SIGNAL(partCreated(KParts::Part*)), this, SLOT(slotPartCreated(KParts::Part*)));
	  connect(m_editor, SIGNAL(viewCreated(QWidget*)), this, SLOT(slotViewCreated(QWidget*)));
	  connect(m_editor, SIGNAL(activatePart(KParts::Part*)), this, SLOT(slotPartActivated(KParts::Part*)));
	  connect(m_editor, SIGNAL(activateView(QWidget*)), this, SLOT(slotViewActivated(QWidget*)));
	}
  }
  else
  {
    KMessageBox::error(this, "Could not find our Part!");
    kapp->quit();
  }
}


EditorTest::~EditorTest()
{
}


void EditorTest::setupActions()
{
  KStdAction::quit(kapp, SLOT(quit()), actionCollection());
}


void EditorTest::slotPartCreated(KParts::Part *part)
{
  partManager->addPart(part);
  partManager->addManagedTopLevelWidget(part->widget());
}


void EditorTest::slotViewCreated(QWidget *view)
{
  tabWidget->addTab(view, view->caption());
  view->show();
  tabWidget->showPage(view);
}


void EditorTest::slotPartActivated(KParts::Part *part)
{
  partManager->setActivePart(part);
}


void EditorTest::slotViewActivated(QWidget *view)
{
  tabWidget->showPage(view);
}


#include "editortest.moc"
