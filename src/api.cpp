#include "core.h"
#include "codemodel.h"
#include "partcontroller.h"
#include "plugincontroller.h"
#include "toplevel.h"
#include "debugger.h"
#include "api.h"


API *API::s_instance = 0;


KDevMainWindow *API::mainWindow() const
{
  return TopLevel::getInstance();
}

bool API::mainWindowValid() const
{
  return TopLevel::mainWindowValid();
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


KDevDebugger *API::debugger() const
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
  m_classStore = new CodeModel();
}


API::~API()
{
  delete( m_classStore );
  m_classStore = 0;
}

KDevPlugin * API::queryForExtension( const QString & serviceType )
{
/*    if (!m_extensions.contains(name))
        m_extensions[name] = PluginController::getInstance()->extension(name);*/
    
    return PluginController::getInstance()->extension(serviceType);
}

