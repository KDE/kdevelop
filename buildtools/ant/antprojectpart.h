#ifndef _ANTPROJECTPART_H_
#define _ANTPROJECTPART_H_


#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
//Added by qt3to4:
#include <Q3PopupMenu>


class Q3PopupMenu;


class KAction;
class KDialogBase;


#include "kdevbuildtool.h"


class AntOptionsWidget;


class AntOptions
{
public:

  AntOptions();

  enum Verbosity { Quiet, Verbose, Debug };

  QString m_buildXML;
  QString m_defaultTarget;
  QStringList m_targets;
  QMap<QString,QString> m_properties;
  QMap<QString,bool> m_defineProperties;
  Verbosity m_verbosity;
  
};


class AntProjectPart : public KDevBuildTool
{
  Q_OBJECT

public:
	  
  AntProjectPart(QObject *parent, const char *name, const QStringList &args);
  ~AntProjectPart();
    QStringList distFiles() const;


protected:

  virtual void openProject(const QString &dirName, const QString &projectName);
  virtual void closeProject();

  virtual QString projectDirectory() const;
  virtual QString projectName() const;
  virtual QString mainProgram(bool relative = false) const;
  virtual QString activeDirectory() const;
  virtual QStringList allFiles() const;
  virtual QString buildDirectory() const;
  virtual QString runDirectory() const;
  virtual QString runArguments() const;
  virtual DomUtil::PairList runEnvironmentVars() const;

  virtual void addFile(const QString &fileName);
  virtual void addFiles ( const QStringList &fileList );
  virtual void removeFile(const QString &fileName);
  virtual void removeFiles ( const QStringList& fileList );


private slots:

  void slotBuild();
  void slotTargetMenuActivated(int id);

  void projectConfigWidget(KDialogBase *dlg);
  void contextMenu(Q3PopupMenu *popup, const Context *context);

  void optionsAccepted();

  void slotAddToProject();
  void slotRemoveFromProject();


private:

  void parseBuildXML();
  void fillMenu();
  void populateProject();

  void ant(const QString &target);

  QString m_projectDirectory, m_projectName;
  QStringList m_classPath;

  QStringList m_sourceFiles;

  AntOptions m_antOptions;

  KAction *m_buildProjectAction;

  Q3PopupMenu *m_targetMenu;

  AntOptionsWidget *m_antOptionsWidget;
  ClassPathWidget *m_classPathWidget;

  QString m_contextFileName;

};


#endif
