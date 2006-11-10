/***************************************************************************
 *   Copyright (C) 2003 by Thomas Hasart                                   *
 *   thasart@gmx.de                                                        *
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2002 by Jakob Simon-Gaarde                              *
 *   jakob@jsg.dk                                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _TROLLPROJECTPART_H_
#define _TROLLPROJECTPART_H_

#include <qdict.h>
#include <qguardedptr.h>
#include <qmap.h>
#include <qdatetime.h>

#include "kdevbuildtool.h"

class KDialogBase;
class TrollProjectWidget;
class KDirWatch;
class QMakeDefaultOpts;

class TrollProjectPart : public KDevBuildTool
{
    Q_OBJECT

public:
    TrollProjectPart( QObject *parent, const char *name, const QStringList &args );
    ~TrollProjectPart();

    bool isTMakeProject() const { return m_tmakeProject; }
    bool isQt4Project() const;
    bool isDirty();
    KDirWatch* dirWatch();
    virtual Options options() const;
    QStringList distFiles() const;

protected:
    virtual void openProject(const QString &dirName, const QString &projectName);
    virtual void closeProject();

    virtual QString projectDirectory() const;
    virtual QString projectName() const;
    virtual QString mainProgram(bool relative = false) const;
    virtual QString activeDirectory() const;
    virtual QStringList allFiles() const;
    virtual void addFile(const QString &fileName);
    virtual void addFiles ( const QStringList &fileList );
    virtual void removeFile(const QString &fileName);
    virtual void removeFiles ( const QStringList &fileList );
    virtual QString& getQMakeHeader() {return m_qmakeHeader;}
    virtual QString buildDirectory() const;
    virtual QString runDirectory() const;
    virtual QString runArguments() const;
    virtual DomUtil::PairList runEnvironmentVars() const;

private slots:
    void projectConfigWidget(KDialogBase *dlg);
    void slotBuild();
//    void slotClean();
//    void slotExecute();
    void slotCommandFinished( const QString& command );
    void slotBuildAndExecuteProject();
    void slotBuildAndExecuteTarget();

private:
//    void startMakeCommand(const QString &dir, const QString &target);
    void startQMakeCommand(const QString &dir);
    void execute(const QString &directory, const QString &command);
    void queueCmd(const QString &dir, const QString &cmd);
    QString makeEnvironment();

    bool isValidQtDir( const QString& path ) const;
    QStringList availableQtDirList() const;

    QGuardedPtr<TrollProjectWidget> m_widget;
    QString m_qmakeHeader;
    QString m_projectName;
    bool m_tmakeProject;

    QMap<QString, QDateTime> m_timestamp;
    bool m_executeAfterBuild;
    QString m_buildCommand;
    QString m_defaultQtDir;
    QStringList m_availableQtDirList;

    KDirWatch* m_dirWatch;

    friend class TrollProjectWidget;
    friend class ProjectRunOptionsDlg;
    friend class QMakeDefaultOpts;
};

#endif

