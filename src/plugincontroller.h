#ifndef __PLUGINCONTROLLER_H__
#define __PLUGINCONTROLLER_H__


#include <qobject.h>


#include <kservice.h>

class KXMLGUIClient;
class KService;
class KDevPlugin;


class PluginController : public QObject
{
  Q_OBJECT
	
public:

  ~PluginController();

  static void createInstance();
  static PluginController *getInstance();

  static KService::List pluginServices( const QString &scope = QString::null );

  static KDevPlugin *loadPlugin( const KService::Ptr &service );

  static QStringList argumentsFromService( const KService::Ptr &service );

  void loadInitialPlugins();


signals:

  void loadingPlugin(const QString &plugin);
  

protected:

  PluginController();


private:

  void loadGlobalPlugins();
  void loadDefaultParts();

  void integratePart(KXMLGUIClient *part);

  static PluginController *s_instance;

};


#endif
