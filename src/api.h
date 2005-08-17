#ifndef _API_H_
#define _API_H_


#include "kdevapi.h"


class CodeModel;

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
  virtual CodeModel *codeModel() const;

  static API *getInstance();

  ~API();

protected:

  API();

private:

  static API *s_instance;

  CodeModel *m_classStore;


};


#endif
