#include <qtimer.h>


#include <kapplication.h>
#include <kdebug.h>
#include <kstatusbar.h>
#include <kmainwindow.h>
#include <kconfig.h>


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

void Core::insertNewAction( KAction* action )
{
    Q_UNUSED( action );
}


bool Core::queryClose()
{
  // save the the project to open it automaticly on startup if needed
  KConfig* config = kapp->config();
  config->setGroup("General Options");
  config->writeEntry("Last Project",ProjectManager::getInstance()->projectFile());

  if ( ProjectManager::getInstance()->projectLoaded()
   && !ProjectManager::getInstance()->closeProject() )
      return false;
  if ( !PartController::getInstance()->readyToClose() )
      return false;
  return true;
}


void Core::running(KDevPlugin * which, bool runs)
{
  emit activeProcessChanged( which, runs );
}


void Core::fillContextMenu(QPopupMenu *popup, const Context *context)
{
  emit contextMenu(popup, context);
}


void Core::openProject(const QString& projectFileName)
{
  ProjectManager::getInstance()->loadProject(projectFileName);
}

#include "core.moc"
