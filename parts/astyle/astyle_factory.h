#ifndef __ASTYLE_FACTORY_H__
#define __ASTYLE_FACTORY_H__


#include <kdevfactory.h>


class AStyleFactory : public KDevFactory
{
  Q_OBJECT

public:

  AStyleFactory(QObject *parent=0, const char *name=0);
  ~AStyleFactory();

  virtual KDevPart *createPartObject(KDevApi *api, QObject *parent, const QStringList &args);
    
  static KInstance *instance();

  
private:

  static KInstance *s_instance;
  
};


#endif
