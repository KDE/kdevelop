#ifndef __PLUGINCONTROLLER_H__
#define __PLUGINCONTROLLER_H__

#include <kservice.h>

class KXMLGUIClient;
class KDevPart;
class KService;
class KDevPlugin;


class PluginController
{

public:

  ~PluginController();

  static void createInstance();
  static PluginController *getInstance();

  static KService::List pluginServices( const QString &scope = QString::null );

  static KDevPart *loadPlugin(const KService::Ptr &service, const char *className, QObject *parent);

  static KDevPlugin *loadPlugin( const KService::Ptr &service );

protected:

  PluginController();


private:

  void loadGlobalPlugins();
  void loadDefaultParts();

  void integratePart(KXMLGUIClient *part);

  static QStringList argumentsFromService( const KService::Ptr &service );

  static PluginController *s_instance;

};


#endif
