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

  statusBar()->show();

  KTrader::OfferList offers = KTrader::self()->query(QString::fromLatin1("KDevelop/Editor"), QString::null);
  if (offers.isEmpty())
    return ;

  KService *service = *offers.begin();
  kdDebug(9000) << "Loading service " << service->name() << endl;

  KLibFactory *factory = KLibLoader::self()->factory(service->library());
  if (factory)
  {
    m_editor = static_cast < Editor* > (factory->create(this,
                                        "editortest_part", "KParts::ReadWritePart" ));

    if (m_editor)
    {
      setCentralWidget(m_editor->widget());

      createGUI(m_editor);

    }
  }
  else
  {
    KMessageBox::error(this, "Could not find our Part!");
    kapp->quit();
  }
}

EditorTest::~EditorTest()
{}



void EditorTest::load(const KURL& url)
{
  m_editor->openURL( url );
}

void EditorTest::setupActions()
{
  KStdAction::quit(kapp, SLOT(quit()), actionCollection());
  new KAction("Insert 'Hello'", 0, this, SLOT(insertText()), actionCollection(), "test_insert");
  new KAction("Append 'Hello'", 0, this, SLOT(appendText()), actionCollection(), "test_append");
}


void EditorTest::saveProperties(KConfig* /*config*/)
{
  // the 'config' object points to the session managed
  // config file.  anything you write here will be available
  // later when this app is restored
}



void EditorTest::readProperties(KConfig* /*config*/)
{
  // the 'config' object points to the session managed
  // config file.  this function is automatically called whenever
  // the app is being restored.  read in here whatever you wrote
  // in 'saveProperties'
}


void EditorTest::insertText()
{
/*
  int line, col;
  CursorIface *cursor = (CursorIface*)m_editor->getInterface("CursorIface");
  if (cursor)
	cursor->getCursorPosition(line, col);
  EditIface *edit = (EditIface*)m_editor->getInterface("EditIface");
  if (edit)
	edit->insertAt("Hello", line, col);
*/
}


void EditorTest::appendText()
{
/*		
  EditIface *edit = (EditIface*)m_editor->getInterface("EditIface");
  if (edit)
    edit->append("Hello");
*/
}


#include "editortest.moc"
