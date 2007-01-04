/***************************************************************************
 *   Copyright (C) 2003 Alexander Dymo                                     *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef __KDEVPART_PASCALPROJECT_H__
#define __KDEVPART_PASCALPROJECT_H__

#include <qguardedptr.h>

#include "kdevbuildtool.h"

class PascalProjectWidget;
class KDialogBase;
class KDevCompilerOptions;

class PascalProjectPart : public KDevBuildTool
{
    Q_OBJECT
public:
    PascalProjectPart(QObject *parent, const char *name, const QStringList &);
    ~PascalProjectPart();

    virtual void openProject(const QString &dirName, const QString &projectName);
    virtual void closeProject();

    /**Returns the name of the main source file without extension.
    All pascal compilers call the binary by that way*/
    virtual QString mainProgram() const;
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

    virtual QString defaultOptions(const QString compiler) const;
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
    QGuardedPtr<PascalProjectWidget> m_widget;

    void listOfFiles(QStringList &result, QString path) const;

    QString m_buildDir;
    QString m_projectDir;
    QString m_projectName;

    QString m_mainProg;
    QString m_mainSource;
    QString m_compilerExec;
    QString m_compilerOpts;

    QStringList m_sourceFiles;
};

#endif
