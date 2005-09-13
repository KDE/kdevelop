#include <qtimer.h>
#include <qmenu.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kstatusbar.h>
#include <kmainwindow.h>
#include <kconfig.h>
#include <kdeversion.h>
#include <kstandarddirs.h>
#include <kglobal.h>

#include "toplevel.h"
#include "partcontroller.h"
#include "api.h"
#include "projectmanager.h"

#include "core.h"


Core *Core::s_instance = 0;


Core *Core::getInstance()
{
  if (!s_instance)
    s_instance = new Core;
  return s_instance;
}


Core::Core()
  : KDevCore()
{
}


Core::~Core()
{
}

bool Core::queryClose()
{
  // save the the project to open it automaticly on startup if needed
  KConfig* config = kapp->config();
  config->setGroup("General Options");
  config->writePathEntry("Last Project",ProjectManager::getInstance()->projectFile().url());

  if ( !PartController::getInstance()->querySaveFiles() )
	  return false;

  if ( !ProjectManager::getInstance()->closeProject( true ) )
      return false;

  if ( !PartController::getInstance()->readyToClose() )
      return false;

  return true;
}


void Core::running(KDevPlugin * which, bool runs)
{
  emit activeProcessChanged( which, runs );
}


void Core::fillContextMenu(QMenu *popup, const Context *context)
{
  emit contextMenu(popup, context);
}


void Core::openProject(const QString& projectFileName)
{
  ProjectManager::getInstance()->loadProject(KURL( projectFileName ));
}

namespace MainWindowUtils{
QString beautifyToolTip(const QString& text)
{
    QString temp = text;
    temp.replace(QRegExp("&"), "");
    temp.replace(QRegExp("\\.\\.\\."), "");
    return temp;
}
}

#include "core.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
