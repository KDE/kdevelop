#include "core.h"
#include "classstore.h"
#include "partcontroller.h"


#include "api.h"


API *API::s_instance = 0;


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


void API::createInstance()
{
  if (!s_instance)
    s_instance = new API;
}



API *API::getInstance()
{
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

