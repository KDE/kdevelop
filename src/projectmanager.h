#ifndef __PROJECTMANAGER_H__
#define __PROJECTMANAGER_H__


#include <qstring.h>
#include <qobject.h>


#include <kxmlguiclient.h>
#include <kservice.h>

class KAction;
class KService;
class ProjectInfo;
class ProjectSession;

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
  bool closeProject();

private slots:
  void slotOpenProject();
  void slotProjectOptions();

  void loadLocalParts();
  void unloadLocalParts();

private:
  ProjectManager();

  void setupActions();
  void getGeneralInfo();
  bool closeProjectSources();
  bool checkNewService(const KService::Ptr &service);

  bool loadProjectFile();
  bool saveProjectFile();

  bool loadProjectPart();
  void unloadProjectPart();

  bool loadLanguageSupport();
  void unloadLanguageSupport();

  bool loadCreateFileSupport();
  void unloadCreateFileSupport();

  ProjectInfo *m_info;

  class KAction *m_closeProjectAction, *m_projectOptionsAction;
  class KRecentFilesAction *m_openRecentProjectAction;

  static ProjectManager *s_instance;

  ProjectSession* m_pProjectSession;
};


#endif
