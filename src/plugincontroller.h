#ifndef __PLUGINCONTROLLER_H__
#define __PLUGINCONTROLLER_H__

#include <qhash.h>

#include <kservice.h>
#include <kservicetypetrader.h>
#include <kdevplugincontroller.h>

#include <kdevprofileengine.h>
#include "shellexport.h"

class KXMLGUIClient;
class KService;
class KDevPlugin;
class KDialog;
class ProjectInfo;

/**
Plugin controller implementation.
Loads and unloads plugins.
*/
class KDEVSHELL_EXPORT PluginController : public KDevPluginController
{
  Q_OBJECT

public:

  ~PluginController();

  static PluginController *getInstance();
  static QStringList argumentsFromService( const KService::Ptr &service );


  virtual KDevPlugin * loadPlugin( const QString & serviceType, const QString & constraint );
  virtual void unloadPlugin( const QString & plugin );

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

  const QList<KDevPlugin*> loadedPlugins( const QString& category = QString::null );

  ProfileEngine &engine() { return m_engine; }

  virtual KUrl::List profileResources(const QString &nameFilter);
  virtual KUrl::List profileResourcesRecursive(const QString &nameFilter);

  //returns the name of an old profile that was unloaded
  QString changeProfile(const QString &newProfile);

signals:
  void loadingPlugin(const QString &plugin);

protected:
  PluginController();

private slots:
  void loadCorePlugins();
  void loadPlugins( KService::List offers, const QStringList & ignorePlugins = QStringList() );
  void unloadPlugins();

private:
	static KDevPlugin *loadPlugin( const KService::Ptr &service );


  QHash<QString, KDevPlugin *> m_parts;
  QString m_profile;

  static PluginController *s_instance;

  ProfileEngine m_engine;

};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
