#ifndef __PLUGINCONTROLLER_H__
#define __PLUGINCONTROLLER_H__

#include <qdict.h>
#include <qvaluelist.h>

#include <kservice.h>

#include <kdevplugincontroller.h>

#include <profileengine.h>

class KXMLGUIClient;
class KService;
class KDevPlugin;
class KDialogBase;
class ProjectInfo;

/**
Plugin controller implementation.
Loads and unloads plugins.
*/
class PluginController : public KDevPluginController
{
  Q_OBJECT

public:

  ~PluginController();

  static PluginController *getInstance();
  static QStringList argumentsFromService( const KService::Ptr &service );

  QString currentProfile() const { return m_profile; }
  
  void loadInitialPlugins();
  
  void loadProjectPlugins( const QStringList & ignorePlugins );
  void unloadProjectPlugins();

  void loadGlobalPlugins( const QStringList & ignorePlugins = QStringList() );
    
  //  KDevPlugin * getPlugin( const KService::Ptr &service );
  
  virtual KDevPlugin *extension(const QString &serviceType, const QString &constraint = "");
  
  void unloadPlugins( QStringList const & );
  
  void integratePart(KXMLGUIClient *part);
  void integrateAndRememberPart(const QString &name, KDevPlugin *part);
  void removePart(KXMLGUIClient* part);
  void removeAndForgetPart(const QString &name, KDevPlugin* part);

  const QValueList<KDevPlugin*> loadedPlugins();
  
  ProfileEngine &engine() { return m_engine; }

  virtual KURL::List profileResources(const QString &nameFilter);
  virtual KURL::List profileResourcesRecursive(const QString &nameFilter);
  
  //returns the name of an old profile that was unloaded
  QString changeProfile(const QString &newProfile);
  
signals:
  void loadingPlugin(const QString &plugin);

protected:
  PluginController();

private slots:
  void slotConfigWidget( KDialogBase* );
  void loadCorePlugins();
  void loadPlugins( KTrader::OfferList offers, const QStringList & ignorePlugins = QStringList() );
  void unloadPlugins();
  
private:
	static KDevPlugin *loadPlugin( const KService::Ptr &service );


  QDict<KDevPlugin> m_parts;
  QString m_profile;
  
  static PluginController *s_instance;
  
  ProfileEngine m_engine;

};

#endif
