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

  QString currentProfile() const { return m_profile; }
  QString currentProfilePath() const { return m_profilePath; }
  
  void loadInitialPlugins();

  void integratePart(KXMLGUIClient *part);
  void removePart(KXMLGUIClient* part);

  const QDict<KDevPlugin>& globalParts() { return m_globalParts; }

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
  QString m_profile;
  QString m_profilePath;
  QString m_defaultProfile;
  QString m_defaultProfilePath;
  
  static PluginController *s_instance;

};

#endif
