#ifndef __KONSOLEVIEWFACTORY_H__
#define __KONSOLEVIEWFACTORY_H__


#include "kdevfactory.h"


class KonsoleViewFactory : public KDevFactory
{
  Q_OBJECT

public:

  KonsoleViewFactory(QObject *parent=0, const char *name=0);
  ~KonsoleViewFactory();

  virtual KDevPart *createPartObject(KDevApi *api, QObject *parent, const QStringList &args);
    
  static KInstance *instance();

  
private:

  static KInstance *s_instance;
  
};


#endif
