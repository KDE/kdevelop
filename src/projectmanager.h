#ifndef __PROJECTMANAGER_H__
#define __PROJECTMANAGER_H__


#include <qstring.h>
#include <qobject.h>


#include <kxmlguiclient.h>
#include <kservice.h>

class KAction;
class KService;
class ProjectInfo;

class ProjectManager : public QObject, public KXMLGUIClient
{
  Q_OBJECT

public:

  ~ProjectManager();

  static ProjectManager *getInstance();

  void loadSettings();
  void saveSettings();
  void loadDefaultProject();

  bool projectLoaded() const;

  QString projectFile() const;

  void createActions( KActionCollection* ac );

public slots:

  void loadProject( const KURL& url);
  void loadRecentProject( const KURL& url);
  void closeProject();

private slots:

  void slotOpenProject();
  void slotProjectOptions();

private:

  ProjectManager();

  void setupActions();

  bool loadProjectFile(const QString &fileName);
  void getGeneralInfo();

  void loadProjectPart();
  void loadLanguageSupport();
  void loadLocalParts();
  void unloadLocalParts();
  void unloadLanguageSupport();
  void saveProjectFile();
  bool closeProjectSources();

  void initializeProjectSupport();

  void checkNewService(const KService::Ptr &service);

  bool loadService( const KService::Ptr &service );

  void integratePart(KXMLGUIClient *part);
  void removePart(KXMLGUIClient *part);

  ProjectInfo *m_info;

  class KAction *m_closeProjectAction, *m_projectOptionsAction;
  class KRecentFilesAction *m_openRecentProjectAction;

  static ProjectManager *s_instance;

};


#endif
