#ifndef __PROJECTMANAGER_H__
#define __PROJECTMANAGER_H__


#include <qstring.h>
#include <qobject.h>


#include <kxmlguiclient.h>
#include <kservice.h>

class KAction;
class ProjectInfo;

class ProjectManager : public QObject, public KXMLGUIClient
{
  Q_OBJECT

public:

  ~ProjectManager();

  static ProjectManager *getInstance();

  void loadProject(const QString &fileName);
  void closeProject();

  bool projectLoaded() const;

  QString projectFile() const;


private:

  ProjectManager();

  void setupActions();

  bool loadProjectFile(const QString &fileName);
  void getGeneralInfo();

  void loadProjectPart();
  void loadLanguageSupport();
  void loadLocalParts();
  void storePartInfo();
  void unloadLocalParts();
  void unloadLanguageSupport();
  void saveProjectFile();
  bool closeProjectSources();

  void initializeProjectSupport();

  bool loadService( const KService::Ptr &service );
  void checkNewService(const KService::Ptr &service);

  void integratePart(KXMLGUIClient *part);
  void removePart(KXMLGUIClient *part);

  ProjectInfo *m_info;

  static ProjectManager *s_instance;

};


#endif
