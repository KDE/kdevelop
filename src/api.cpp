#include "core.h"
#include "classstore.h"
#include "partcontroller.h"
#include "toplevel.h"
#include "debugger.h"


#include "api.h"


API *API::s_instance = 0;


KDevMainWindow *API::mainWindow()
{
  return TopLevel::getInstance();
}

bool API::mainWindowValid()
{
  return TopLevel::mainWindowValid();
}

KDevPartController *API::partController()
{
  return PartController::getInstance();
}


KDevCore *API::core()
{
  return Core::getInstance();
}


ClassStore *API::classStore()
{
  return m_classStore;
}


ClassStore *API::ccClassStore()
{
  return m_ccClassStore;
}


KDevDebugger *API::debugger()
{
  return Debugger::getInstance();
}


API *API::getInstance()
{
  if (!s_instance)
    s_instance = new API;
  return s_instance;
}


API::API()
  : KDevApi()
{
  m_classStore = new ClassStore();
  m_ccClassStore = new ClassStore();
}


API::~API()
{
}

