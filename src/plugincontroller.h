#ifndef __PLUGINCONTROLLER_H__
#define __PLUGINCONTROLLER_H__

#include <qobject.h>
#include <qdict.h>

#include <kservice.h>

class KXMLGUIClient;
class KService;
class KDevPlugin;
class KDialogBase;

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

  void integratePart(KXMLGUIClient *part);
  void removePart(KXMLGUIClient* part);

signals:
  void loadingPlugin(const QString &plugin);

protected:
  PluginController();

private slots:
  void slotConfigWidget( KDialogBase* );
  void loadGlobalPlugins();
  void loadCorePlugins();
  void unloadGlobalPlugins();
  
private:
  void loadDefaultParts();

  QDict<KDevPlugin> m_globalParts;
  
  static PluginController *s_instance;

};

#endif
