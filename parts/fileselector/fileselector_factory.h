#ifndef __FILESELECTOR_FACTORY_H__
#define __FILESELECTOR_FACTORY_H__


#include <kdevfactory.h>


class FileSelectorFactory : public KDevFactory
{
  Q_OBJECT

public:

  FileSelectorFactory(QObject *parent=0, const char *name=0);
  ~FileSelectorFactory();

  virtual KDevPart *createPartObject(KDevApi *api, QObject *parent, const QStringList &args);

  static KInstance *instance();


private:

  static KInstance *s_instance;

};


#endif
