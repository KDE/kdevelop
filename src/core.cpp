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


void Core::gotoExecutionPoint(const QString &fileName, int lineNum)
{
  PartController::getInstance()->editDocument(KURL(fileName), lineNum);
}


void Core::setBreakpoint(const QString &fileName, int lineNum, int id, bool enabled, bool pending)
{
  kdDebug() << "Core::setBreakpoint" << endl;
}


void Core::running(KDevPart *which, bool runs)
{
  kdDebug() << "Core::running " << endl;
}


void Core::fillContextMenu(QPopupMenu *popup, const Context *context)
{
  emit contextMenu(popup, context);
}


void Core::message(const QString &str)
{
  TopLevel::getInstance()->statusBar()->message(str);
}


void Core::openProject(const QString& projectFileName)
{
  ProjectManager::getInstance()->loadProject(projectFileName);
}


#include "core.moc"
