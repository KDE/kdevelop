#include <kmainwindow.h>
#include <kstatusbar.h>


#include "kdevtoplevel.h"


KStatusBar *KDevTopLevel::statusBar()
{
  return main()->statusBar();
}


void KDevTopLevel::realClose()
{
}
