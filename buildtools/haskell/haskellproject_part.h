/***************************************************************************
                          haskellproject_part.h  -  description
                             -------------------
    begin                : Mon Aug 11 2003
    copyright            : (C) 2003 Peter Robinson
    email                : listener@thaldyron.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef HASKELLPROJECT_PART_H
#define HASKELLPROJECT_PART_H

#include <qwidget.h>
#include <qguardedptr.h>
#include <kaction.h>

#include "kdevbuildtool.h"
#include "domutil.h"
#include "kdevcore.h"
#include "kdevmainwindow.h"
#include "kdevmakefrontend.h"
#include "kdevappfrontend.h"
#include "kdevpartcontroller.h"
#include "kdevlanguagesupport.h"
#include "kdevcompileroptions.h"
#include "runoptionswidget.h"

/**
  *@author KDevelop Authors
  */
class HaskellProjectWidget;


class HaskellProjectPart : public KDevBuildTool
{
  Q_OBJECT

public:

	HaskellProjectPart(QObject *parent, const char *name, const QStringList &);
 	~HaskellProjectPart();
 	virtual void openProject(const QString &dirName, const QString &projectName);
 	virtual void closeProject();

 	/**Returns the name of the main source */
 	virtual QString mainProgram();
 	virtual QString mainProgram(bool relative = false) const;
 	/**Main source file (like src/main.pp)*/
 	virtual QString mainSource() const;
 	virtual void setMainSource(QString fullPath);

 	virtual QString projectDirectory() const;
 	virtual QString projectName() const;
 	virtual QString activeDirectory() const;
 	/**The location of the main source file*/
 	virtual QString buildDirectory() const;
	virtual QString runDirectory() const;
 	virtual QString runArguments() const;
 	virtual DomUtil::PairList runEnvironmentVars() const;

  	/**Returns everything in the project directory*/
  	virtual QStringList allFiles() const;
  	/**This does absolutelly nothing*/
  	virtual void addFile(const QString &fileName);
  	/**This does absolutelly nothing*/
  	virtual void addFiles(const QStringList &fileList);
  	/**This does absolutelly nothing*/
  	virtual void removeFile(const QString &fileName);
  	/**This does absolutelly nothing*/
  	virtual void removeFiles(const QStringList &fileList);

  	virtual void changedFiles( const QStringList & fileList );
  	virtual void changedFile( const QString & fileName );

  	KDevCompilerOptions *createCompilerOptions(const QString &name);

  	virtual QString defaultOptions(const QString compiler);
    QStringList distFiles() const;

public slots:
    /**loads config from project file*/
    void loadProjectConfig();

private slots:
    void slotBuild();
    void slotExecute();
    void projectConfigWidget(KDialogBase *dlg);
    void configWidget(KDialogBase *dlg);

private:
  	KAction *_buildAction, *_runAction;
  	QGuardedPtr<HaskellProjectWidget> m_widget;

  	void listOfFiles(QStringList &result, QString path);
  	QString createCmdLine( QString srcFile );
  	QString createPackageString();
  
  	QString _buildDir;
  	QString _projectDir;
  	QString _projectName;

  	QString _mainProg;
  	QString _mainSource;
  	QString _compilerExec;
  	QString _compilerOpts;

  	QStringList _sourceFiles;
};


#endif
