#include <qtimer.h>


#include <kapp.h>
#include <kdebug.h>
#include <kstatusbar.h>
#include <kmainwindow.h>


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


void Core::wantsToQuit()
{
  QTimer::singleShot(0, this, SLOT(slotQuit()));
}


void Core::slotQuit()
{
  ProjectManager::getInstance()->closeProject();
  if (PartController::getInstance()->readyToClose())
    TopLevel::getInstance()->realClose();
}


void Core::setBreakpoint(const QString &fileName, int lineNum, int id, bool enabled, bool pending)
{
    (void) fileName;
    (void) lineNum;
    (void) id;
    (void) enabled;
    (void) pending;
  kdDebug(9000) << "Core::setBreakpoint" << endl;
}


void Core::running(KDevPlugin *which, bool runs)
{
    (void) which;
    (void) runs;
  kdDebug(9000) << "Core::running " << endl;
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
