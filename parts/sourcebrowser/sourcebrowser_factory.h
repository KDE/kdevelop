#ifndef __SOURCEBROWSER_FACTORY_H__
#define __SOURCEBROWSER_FACTORY_H__


#include <kdevfactory.h>


class SourceBrowserFactory : public KDevFactory
{
  Q_OBJECT

public:

  SourceBrowserFactory(QObject *parent=0, const char *name=0);
  ~SourceBrowserFactory();

  virtual KDevPart *createPartObject(KDevApi *api, QObject *parent, const QStringList &args);
    
  static KInstance *instance();

  
private:

  static KInstance *s_instance;
  
};


#endif
