#include <kmainwindow.h>
#include <kstatusbar.h>


#include "kdevmainwindow.h"


KStatusBar *KDevMainWindow::statusBar()
{
  return main()->statusBar();
}
