#ifndef __TIPOFDAY_FACTORY_H__
#define __TIPOFDAY_FACTORY_H__


#include <kdevfactory.h>


class TipOfDayFactory : public KDevFactory
{
  Q_OBJECT

public:

  TipOfDayFactory(QObject *parent=0, const char *name=0);
  ~TipOfDayFactory();

  virtual KDevPart *createPartObject(KDevApi *api, QObject *parent, const QStringList &args);
    
  static KInstance *instance();

  
private:

  static KInstance *s_instance;
  
};


#endif
