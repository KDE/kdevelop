#include <kapp.h>
#include <kdebug.h>
#include <kstatusbar.h>
#include <dcopclient.h>
#include <kmainwindow.h>


#include "KDevCoreIface.h"


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
  s_instance = this;
  
  m_dcopIface = new KDevCoreIface(this);
  kapp->dcopClient()->registerAs("gideon");
}


Core::~Core()
{
  delete m_dcopIface;
}


void Core::embedWidget(QWidget *w, Role role, const QString &shortCaption)
{
  if (role == SelectView)
    TopLevel::getInstance()->embedSelectView(w, shortCaption);
  if (role == OutputView)
    TopLevel::getInstance()->embedOutputView(w, shortCaption);
}


void Core::raiseWidget(QWidget *w)
{
  TopLevel::getInstance()->raiseView(w);
}


void Core::lowerWidget(QWidget *w)
{
  kdDebug() << "Lower Widget " << w << endl;

  TopLevel::getInstance()->lowerView(w);
}


void Core::removeWidget(QWidget* w, Role)
{
  TopLevel::getInstance()->removeView(w);
}


void Core::gotoFile(const KURL &url)
{
  PartController::getInstance()->editDocument(url);
}


void Core::gotoDocumentationFile(const KURL& url, Embedding)
{
  PartController::getInstance()->showDocument(url);
}


void Core::gotoSourceFile(const KURL& url, int lineNum, Embedding)
{
  PartController::getInstance()->editDocument(url, lineNum);
}


void Core::gotoExecutionPoint(const QString &fileName, int lineNum)
{
  PartController::getInstance()->editDocument(KURL(fileName), lineNum);
}


void Core::saveAllFiles()
{
  PartController::getInstance()->saveAllFiles();
}


void Core::revertAllFiles()
{
  PartController::getInstance()->revertAllFiles();
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
}


void Core::message(const QString &str)
{
  TopLevel::getInstance()->main()->statusBar()->message(str);
}


void Core::openProject(const QString& projectFileName)
{
  ProjectManager::getInstance()->loadProject(projectFileName);
}


QStatusBar *Core::statusBar() const
{
  return TopLevel::getInstance()->main()->statusBar();
}


#include "core.moc"
