#ifndef __PROJECTMANAGER_H__
#define __PROJECTMANAGER_H__


#include <qstring.h>
#include <qobject.h>
#include <qdict.h>


#include <kxmlguiclient.h>
#include <kservice.h>

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
  KURL         m_projectURL;
  QDomDocument m_document;
  QString      m_projectPlugin, m_language, m_activeLanguage, m_vcsPlugin;
  QStringList  m_ignoreParts, m_loadParts, m_keywords, m_secondaryLanguages;

  QString sessionFile() const;
};

/**
Project manager.
Loads and unloads projects.
*/
class ProjectManager : public QObject, public KXMLGUIClient
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

  KURL projectFile() const;

  void createActions( KActionCollection* ac );

  ProjectSession* projectSession() const;

public slots:
  bool loadProject( const KURL& url);
  bool loadKDevelop2Project( const KURL& url);
  bool closeProject( bool exiting = false );

private slots:
  void slotOpenProject();
  void slotProjectOptions();
  
  void slotLoadProject();

  void loadLocalParts();

  void updateActiveLangMenu();
  void switchLanguage(const QString& lang);
  
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

  void loadVCSSupport();
  void unloadVCSSupport();
  
  ProjectInfo *m_info;

  KAction *m_closeProjectAction, *m_projectOptionsAction;
  KRecentFilesAction *m_openRecentProjectAction;
  KSelectAction *m_activeLanguage;

  static ProjectManager *s_instance;

  ProjectSession* m_pProjectSession;
  
  KDevPlugin *m_vcsPlugin;
  QString m_vcsName;
};


#endif
