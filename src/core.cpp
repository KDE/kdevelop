#include <qtimer.h>


#include <kapp.h>
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
  , m_activeProcesses( 0 )
{
}


Core::~Core()
{
}


void Core::wantsToQuit()
{
  QTimer::singleShot(0, this, SLOT(slotQuit()));
}


void Core::slotQuit()
{
  // save the the project to open it automaticly on startup if needed
  KConfig* config = kapp->config();
  config->setGroup("General Options");
  config->writeEntry("Last Project",ProjectManager::getInstance()->projectFile());

  if ( !ProjectManager::getInstance()->closeProject() )
    return;
  if (PartController::getInstance()->readyToClose())
    TopLevel::getInstance()->realClose();
}


void Core::running(KDevPlugin * /* which */, bool runs)
{
  kdDebug(9000) << "Core::running " << endl;
  m_activeProcesses += (runs ? 1 : -1);
  emit activeProcessCountChanged( m_activeProcesses );
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
