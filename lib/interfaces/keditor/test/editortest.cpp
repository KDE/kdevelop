/*
 * Copyright (C) 2001  <kurt@granroth.org>
 */

#include "editortest.h"

#include <kconfig.h>
#include <kurl.h>
#include <ktrader.h>
#include <kservice.h>
#include <kdebug.h>
#include <kaction.h>
#include <kstdaction.h>
#include <klocale.h>
#include <klibloader.h>
#include <kmessagebox.h>
#include <kfiledialog.h>


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
	  KParts::MainWindow::factory()->addClient(m_editor);
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
  KStdAction::openNew(this, SLOT(slotNew()), actionCollection());
  KStdAction::open(this, SLOT(slotOpen()), actionCollection());
}


void EditorTest::slotNew()
{
  KEditor::Document *doc = m_editor->createDocument();
  if (!doc)
    return;

  partManager->addPart(doc);
  if (doc->widget())
  {
    tabWidget->addTab(doc->widget(), i18n("Unknown"));
    tabWidget->showPage(doc->widget());
  }
}


void EditorTest::slotOpen()
{
  KURL url = KFileDialog::getOpenURL();
  if (url.isEmpty())
    return; 

  KEditor::Document *doc = m_editor->document(url);
  if (!doc)
    doc = m_editor->createDocument(url);
  if (!doc)
    return;

  partManager->addPart(doc);
  if (doc->widget())
  {
    tabWidget->addTab(doc->widget(), url.url());
    tabWidget->showPage(doc->widget());
  }
}


#include "editortest.moc"
