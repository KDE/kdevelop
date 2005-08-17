#include <qatomic.h>
#include <kapplication.h>

#include "toplevel.h"
#include "core.h"

#include "simplemainwindow.h"

static KDevMainWindow *TopLevel::s_instance = 0;

bool TopLevel::isMainWindowValid()
{
  return s_instance != 0;
}

KDevMainWindow *TopLevel::getInstance()
{
  if (!s_instance)
  {
    SimpleMainWindow *mainWindow = new SimpleMainWindow(0);
    mainWindow->setObjectName(QLatin1String("SimpleMainWindow"));
    s_instance = mainWindow;
    mainWindow->init();
    kapp->setMainWidget(mainWindow);
  }

  return s_instance;
}

void TopLevel::invalidateInstance(KDevMainWindow *instance)
{
  if ( s_instance == instance )
    s_instance = 0;
}
