#include <kapplication.h>
#include <kconfig.h>


#include "toplevel.h"
#include "core.h"


#include "mainwindow.h"
#include "mainwindowideal.h"


KDevMainWindow *TopLevel::s_instance = 0;


bool TopLevel::mainWindowValid()
{
  return s_instance != 0;
}

KDevMainWindow *TopLevel::getInstance()
{
  if (!s_instance)
  {
    KConfig *config = kapp->config();
    config->setGroup("UI");

    if (config->readEntry("MajorUIMode", "IDEAl") == "QextMDI")
    {
      MainWindow *mainWindow = new MainWindow;
      s_instance = mainWindow;

      mainWindow->init();

      kapp->setMainWidget(mainWindow);

      QObject::connect(mainWindow, SIGNAL(wantsToQuit()), Core::getInstance(), SLOT(wantsToQuit()));
    }
    else
    {
      MainWindowIDEAl *mainWindowIDEAl = new MainWindowIDEAl;
      s_instance = mainWindowIDEAl;

      mainWindowIDEAl->init();

      kapp->setMainWidget(mainWindowIDEAl);

      QObject::connect(mainWindowIDEAl, SIGNAL(wantsToQuit()), Core::getInstance(), SLOT(wantsToQuit()));
    }
  }

  return s_instance;
}

void TopLevel::invalidateInstance(KDevMainWindow *instance) 
{
  if ( s_instance == instance )
    s_instance = 0;
}
