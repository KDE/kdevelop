#include <kapp.h>
#include <kconfig.h>


#include "toplevel.h"
#include "core.h"


#include "mainwindow.h"
#include "mainwindowideal.h"


KDevTopLevel *TopLevel::s_instance = 0;


bool TopLevel::topLevelValid()
{
  return s_instance != 0;
}

KDevTopLevel *TopLevel::getInstance()
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

void TopLevel::invalidateInstance(KDevTopLevel *instance) 
{
  if ( s_instance == instance )
    s_instance = 0;
}
