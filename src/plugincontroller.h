#ifndef __PLUGINCONTROLLER_H__
#define __PLUGINCONTROLLER_H__

#include <kservice.h>

class KXMLGUIClient;
class KService;
class KDevPlugin;


class PluginController
{

public:

  ~PluginController();

  static void createInstance();
  static PluginController *getInstance();

  static KService::List pluginServices( const QString &scope = QString::null );

  static KDevPlugin *loadPlugin( const KService::Ptr &service );

  static QStringList argumentsFromService( const KService::Ptr &service );

protected:

  PluginController();


private:

  void loadGlobalPlugins();
  void loadDefaultParts();

  void integratePart(KXMLGUIClient *part);

  static PluginController *s_instance;

};


#endif
