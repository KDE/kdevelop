#include "core.h"
#include "classstore.h"
#include "partcontroller.h"


#include "api.h"


API *API::s_instance = 0;


KDevPartController *API::getPartController()
{
  return PartController::getInstance();
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
  classStore = new ClassStore();
  ccClassStore = new ClassStore();
}


API::~API()
{
}

