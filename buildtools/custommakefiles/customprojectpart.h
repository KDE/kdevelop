/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _CUSTOMPROJECTPART_H_
#define _CUSTOMPROJECTPART_H_

#include <qdict.h>
#include <qguardedptr.h>
#include <qmap.h>
#include <qdatetime.h>

#include <kdevproject.h>

class QListViewItem;
class QPopupMenu;
class QStringList;
class KDialogBase;
class CustomProjectWidget;
class Context;
class KSelectAction;

class CustomProjectPart : public KDevProject
{
    Q_OBJECT

public:
    CustomProjectPart( QObject *parent, const char *name, const QStringList & );
    ~CustomProjectPart();

    QStringList allMakeEnvironments() const;
    QString currentMakeEnvironment() const;

    bool isDirty();
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
    virtual void addFiles ( const QStringList& fileList );
    virtual void removeFile(const QString &fileName);
    virtual void removeFiles ( const QStringList &fileList );
    virtual QString buildDirectory() const;
    virtual QString runDirectory() const;
    virtual QString runArguments() const;
    virtual DomUtil::PairList runEnvironmentVars() const;


private slots:
    void projectConfigWidget(KDialogBase *dlg);
    void contextMenu(QPopupMenu *popup, const Context *context);
    void slotAddToProject();
    void slotRemoveFromProject();
    void slotChooseActiveDirectory();
    void slotBuild();
    void slotCompileFile();
    void slotClean();
    void slotExecute();
    void updateTargetMenu();
    void targetMenuActivated(int id);
    void updateMakeEnvironmentsMenu();
    void makeEnvironmentsMenuActivated(int id);
    void slotCommandFinished( const QString& command );
    void slotCommandFailed( const QString& command );

private:
    void populateProject();
    void saveProject();
    void startMakeCommand(const QString &dir, const QString &target);
    QString makeEnvironment() const;

    QString m_projectDirectory;
    QString m_projectName;
    QStringList m_sourceFiles;
    QPopupMenu *m_targetMenu;
    KSelectAction *m_makeEnvironmentsSelector;
    QStringList m_targets;
    QString m_contextFileName;
    QString m_contextDirName;

    QMap<QString, QDateTime> m_timestamp;
    bool m_executeAfterBuild;
    QString m_buildCommand;
	bool m_lastCompilationFailed;
};

#endif
