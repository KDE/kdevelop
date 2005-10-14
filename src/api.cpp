
#include <kdevcodemodel.h>

#include "core.h"
#include "documentcontroller.h"
#include "plugincontroller.h"
#include "toplevel.h"
#include "api.h"


API *API::s_instance = 0;


KDevMainWindow *API::mainWindow() const
{
  return TopLevel::getInstance();
}

KDevDocumentController *API::documentController() const
{
  return DocumentController::getInstance();
}


KDevCore *API::core() const
{
  return Core::getInstance();
}


KDevCodeModel *API::codeModel() const
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
  m_classStore = new KDevCodeModel();
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

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
