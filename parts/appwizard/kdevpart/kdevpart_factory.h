#ifndef __$APPNAMEUC$_FACTORY_H__
#define __$APPNAMEUC$_FACTORY_H__


#include <kdevfactory.h>


class $APPNAME$Factory : public KDevFactory
{
  Q_OBJECT

public:

  $APPNAME$Factory(QObject *parent=0, const char *name=0);
  ~$APPNAME$Factory();

  virtual KDevPart *createPartObject(KDevApi *api, QObject *parent, const QStringList &args);
    
  static KInstance *instance();

  
private:

  static KInstance *s_instance;
  
};


#endif
