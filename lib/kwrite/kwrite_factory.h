#ifndef __kwrite_factory_h__
#define __kwrite_factory_h__

#include <kparts/factory.h>

#define KWRITE_VERSION "2.0"

class KInstance;
class KAboutData;

class KWriteFactory : public KParts::Factory
{
  Q_OBJECT
public:
  KWriteFactory();
  virtual ~KWriteFactory();

  virtual KParts::Part *createPart( QWidget *parentWidget, const char *widgetName, QObject *parent, const char *name, const char *classname, const QStringList &args );

  static const KAboutData *aboutData();
  static KInstance *instance();

private:
  static KInstance *s_instance;
  static KAboutData *s_aboutData;
};

#endif
