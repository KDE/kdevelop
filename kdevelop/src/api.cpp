#include "core.h"
#include "codemodel.h"
#include "partcontroller.h"
#include "plugincontroller.h"
#include "toplevel.h"
#include "api.h"


API *API::s_instance = 0;


KDevMainWindow *API::mainWindow() const
{
  return TopLevel::getInstance();
}

KDevPartController *API::partController() const
{
  return PartController::getInstance();
}


KDevCore *API::core() const
{
  return Core::getInstance();
}


CodeModel *API::codeModel() const
{
  return m_classStore;
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
  m_classStore = new CodeModel();
}


API::~API()
{
  delete( m_classStore );
  m_classStore = 0;
}

KDevPluginController * API::pluginController() const
{
    return PluginController::getInstance();
}

