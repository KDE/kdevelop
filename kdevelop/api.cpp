#include "core.h"
#include "classstore.h"
#include "partcontroller.h"
#include "ckdevelop.h"


#include "api.h"


API *API::s_instance = 0;


KDevTopLevel *API::topLevel()
{
  return CKDevelop::getInstance();
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

KDevDebugger *API::debugger()
{
  return 0; // VERY BIG TODO
}
