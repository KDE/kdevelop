#ifndef __PARTSELECTOR_FACTORY_H__
#define __PARTSELECTOR_FACTORY_H__


#include <kdevfactory.h>


class PartSelectorFactory : public KDevFactory
{
  Q_OBJECT

public:

  PartSelectorFactory(QObject *parent=0, const char *name=0);
  ~PartSelectorFactory();

  virtual KDevPart *createPartObject(KDevApi *api, QObject *parent, const QStringList &args);
    
  static KInstance *instance();

  
private:

  static KInstance *s_instance;
  
};


#endif
