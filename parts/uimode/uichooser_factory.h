#ifndef __UICHOOSER_FACTORY_H__
#define __UICHOOSER_FACTORY_H__


#include <kdevfactory.h>


class UIChooserFactory : public KDevFactory
{
  Q_OBJECT

public:

  UIChooserFactory(QObject *parent=0, const char *name=0);
  ~UIChooserFactory();

  virtual KDevPart *createPartObject(KDevApi *api, QObject *parent, const QStringList &args);
    
  static KInstance *instance();

  
private:

  static KInstance *s_instance;
  
};


#endif
