#ifndef _API_H_
#define _API_H_


#include "kdevapi.h"


class CodeModel;


class API : public KDevApi
{
public:

  virtual KDevMainWindow *mainWindow() const;
  virtual bool mainWindowValid() const;
  virtual KDevPartController *partController() const;
  virtual KDevCore *core() const;
  virtual CodeModel *codeModel() const;
  virtual KDevDebugger *debugger() const;


  static API *getInstance();

  ~API();

    KDevPlugin *queryForExtension(const QString &serviceType);

protected:

  API();

    QMap<QString, KDevPlugin*> extensions;

private:

  static API *s_instance;

  CodeModel *m_classStore;

  
};


#endif
