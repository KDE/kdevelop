#ifndef __DISTPART_FACTORY_H__
#define __DISTPART_FACTORY_H__


#include <kdevfactory.h>


class distpartFactory : public KDevFactory
{
  Q_OBJECT

public:

  distpartFactory(QObject *parent=0, const char *name=0);
  ~distpartFactory();

  virtual KDevPart *createPartObject(KDevApi *api, QObject *parent, const QStringList &args);
    
  static KInstance *instance();

  
private:

  static KInstance *s_instance;
  
};


#endif
