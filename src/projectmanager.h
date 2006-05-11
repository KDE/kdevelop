#ifndef __PROJECTMANAGER_H__
#define __PROJECTMANAGER_H__


#include <QString>
#include <QObject>


#include <kxmlguiclient.h>
#include <kservice.h>
#include "shellexport.h"

class KUrl;
class KAction;
class KSelectAction;
class KService;
class ProjectInfo;
class ProjectSession;
class KRecentFilesAction;
class KDevPlugin;


class ProjectInfo
{
public:
  KUrl         m_projectURL;
  QDomDocument m_document;
  QString      m_profileName;
  QString      m_projectPlugin, m_language, m_activeLanguage, m_vcsPlugin;
  QStringList  m_ignoreParts, m_keywords;

  QString sessionFile() const;
};

/**
Project manager.
Loads and unloads projects.
*/
class KDEVSHELL_EXPORT ProjectManager : public QObject, public KXMLGUIClient
{
  Q_OBJECT

public:

  ~ProjectManager();

  static QString projectDirectory( const QString& path, bool absolute );

  static ProjectManager *getInstance();

  void loadSettings();
  void saveSettings();
  void loadDefaultProject();

  bool projectLoaded() const;

  KUrl projectFile() const;

  void createActions( KActionCollection* ac );

  ProjectSession* projectSession() const;

public slots:
  bool loadProject( const KUrl& url);
  bool loadKDevelop2Project( const KUrl& url);
  bool closeProject( bool exiting = false );

private slots:
  void slotOpenProject();
  void slotProjectOptions();

  void slotLoadProject();

  void loadLocalParts();

private:
  ProjectManager();

  void setupActions();
  void getGeneralInfo();

  bool loadProjectFile();
  bool saveProjectFile();

  bool loadProjectPart();
  void unloadProjectPart();

  bool loadLanguageSupport(const QString& lang);
  void unloadLanguageSupport();

  QString profileByAttributes(const QString &language, const QStringList &keywords);

  ProjectInfo *m_info;

  KAction *m_closeProjectAction, *m_projectOptionsAction;
  KRecentFilesAction *m_openRecentProjectAction;

  static ProjectManager *s_instance;

  ProjectSession* m_pProjectSession;

  KDevPlugin *m_vcsPlugin;
  QString m_vcsName;

  QString m_oldProfileName;

  bool m_toolbarStatus[5];
};


#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on

