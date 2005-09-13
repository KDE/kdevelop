#ifndef _API_H_
#define _API_H_


#include "kdevapi.h"

/**
API implementation.
*/
class API : public KDevApi
{
public:

  virtual KDevMainWindow *mainWindow() const;
  virtual KDevPartController *partController() const;
  virtual KDevPluginController *pluginController() const;
  virtual KDevCore *core() const;
  virtual KDevCodeModel *codeModel() const;

  static API *getInstance();

  ~API();

protected:

  API();

private:

  static API *s_instance;

  KDevCodeModel *m_classStore;


};


#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
