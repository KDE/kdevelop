#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kstdaction.h>
#include <kapplication.h>
#include <kaction.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kcmdlineargs.h>
#include <klibloader.h>
#include <qwidget.h>
#include <qdir.h>

#include <ktrader.h>

#include "viewmanager.h"
#include "docmanager.h"
#include "app.h"

Shell::Shell()
{
  setXMLFile("manager.rc");

  DocManager *dm = new DocManager ();
  ViewManager *vm = new ViewManager (this, dm);
  setCentralWidget (vm );

  KStdAction::openNew( vm, SLOT( slotDocumentNew() ), actionCollection(), "file_new" );
  KStdAction::open( vm, SLOT( slotDocumentOpen() ), actionCollection(), "file_open" );
  KStdAction::close( vm, SLOT( slotDocumentClose() ), actionCollection(), "file_close" );

  createGUI();

  resize( 600, 350 );
}

Shell::~Shell()
{
}

int main( int argc, char **argv )
{
  KApplication app( argc, argv, "manager" );
  Shell *shell = new Shell;
  shell->show();
  return app.exec();
}

#include "app.moc"
