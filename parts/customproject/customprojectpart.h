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


class CustomProjectPart : public KDevProject
{
    Q_OBJECT

public:
    CustomProjectPart( QObject *parent, const char *name, const QStringList & );
    ~CustomProjectPart();

    bool isDirty();

protected:
    virtual void openProject(const QString &dirName, const QString &projectName);
    virtual void closeProject();

    virtual QString projectDirectory();
    virtual QString projectName();
    virtual QString mainProgram(bool relative = false);
    virtual QString activeDirectory();
    virtual QStringList allFiles();
    virtual void addFile(const QString &fileName);
    virtual void addFiles ( const QStringList& fileList );
    virtual void removeFile(const QString &fileName);
    virtual void removeFiles ( const QStringList &fileList );
    virtual QString buildDirectory();
    virtual QString runDirectory();
    virtual QString runArguments();
    virtual DomUtil::PairList runEnvironmentVars();

private slots:
    void projectConfigWidget(KDialogBase *dlg);
    void contextMenu(QPopupMenu *popup, const Context *context);
    void slotAddToProject();
    void slotRemoveFromProject();
    void slotBuild();
    void slotCompileFile();
    void slotClean();
    void slotExecute();
    void updateTargetMenu();
    void targetMenuActivated(int id);
    void slotCommandFinished( const QString& command );
    void slotCommandFailed( const QString& command );

private:
    void populateProject();
    void startMakeCommand(const QString &dir, const QString &target);
    QString makeEnvironment();

    QString m_projectDirectory;
    QString m_projectName;
    QStringList m_sourceFiles;
    QPopupMenu *m_targetMenu;
    QStringList m_targets;
    QString m_contextFileName;

    QMap<QString, QDateTime> m_timestamp;
    bool m_executeAfterBuild;
    QString m_buildCommand;
	bool m_lastCompilationFailed;
};

#endif
