#ifndef __PROJECTMANAGER_H__
#define __PROJECTMANAGER_H__


#include <qstring.h>
#include <qobject.h>


#include <kxmlguiclient.h>

class KAction;


#include "kdevpart.h"

class KService;


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

  void checkNewService(KService *service);

  void integratePart(KXMLGUIClient *part);
  void removePart(KDevPart *part);

  ProjectInfo *m_info;

  static ProjectManager *s_instance;

};


#endif
