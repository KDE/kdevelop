#include <kdebug.h>


#include "editorproxy.h"
#include "partcontroller.h"


#include "debugger.h"


Debugger *Debugger::s_instance = 0;

Debugger::Debugger()
{
}


Debugger::~Debugger()
{
}


Debugger *Debugger::getInstance()
{
  if (!s_instance)
    s_instance = new Debugger;

  return s_instance;
}


void Debugger::setBreakpoint(const QString &fileName, int lineNum, int id, bool enabled, bool pending)
{
  KParts::Part *part = PartController::getInstance()->partForURL(KURL(fileName));
  if (part)
    if (id == -1)
      EditorProxy::getInstance()->removeBreakpoint(part, lineNum);
    else
      EditorProxy::getInstance()->setBreakpoint(part, lineNum, enabled, pending);
}


void Debugger::clearExecutionPoint()
{
  EditorProxy::getInstance()->clearExecutionPoint();
}


void Debugger::gotoExecutionPoint(const KURL &url, int lineNum)
{
  PartController::getInstance()->editDocument(url, lineNum);
  
  KParts::Part *part = PartController::getInstance()->partForURL(url);
  if (part)
    EditorProxy::getInstance()->setExecutionPoint(part, lineNum);
}


#include "debugger.moc"
