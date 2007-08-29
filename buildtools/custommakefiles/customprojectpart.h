/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2007 by Andreas Pakulat                                 *
 *   apaku@gmx.de                                                          *
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
#include <qvaluestack.h>

#include <kdevbuildtool.h>

class QListViewItem;
class QPopupMenu;
class QStringList;
class KDialogBase;
class CustomProjectWidget;
class Context;
class KSelectAction;

class CustomProjectPart : public KDevBuildTool
{
    Q_OBJECT

public:
    CustomProjectPart( QObject *parent, const char *name, const QStringList & );
    ~CustomProjectPart();

    QStringList allMakeEnvironments() const;
    QString currentMakeEnvironment() const;

    bool isDirty();
    QStringList distFiles() const;

    virtual void openProject( const QString &dirName, const QString &projectName );
    virtual void closeProject();

    virtual QString projectDirectory() const;
    virtual QString projectName() const;
    virtual QString mainProgram() const;
    virtual QString activeDirectory() const;
    virtual QStringList allFiles() const;
    virtual void addFile( const QString &fileName );
    virtual void addFiles( const QStringList& fileList );
    virtual void removeFile( const QString &fileName );
    virtual void removeFiles( const QStringList &fileList );
    virtual QString buildDirectory() const;
    virtual QString runDirectory() const;
    virtual QString debugArguments() const;
    virtual QString runArguments() const;
    virtual DomUtil::PairList runEnvironmentVars() const;
    QString relativeToProject( const QString& ) const;


private slots:
    void populateProject();
    void projectConfigWidget( KDialogBase *dlg );
    void contextMenu( QPopupMenu *popup, const Context *context );
    void slotAddToProject();
    void slotRemoveFromProject();
    void slotAddToProjectRecursive();
    void slotRemoveFromProjectRecursive();
    void addNewFilesToProject( const QStringList& );
    void slotChangeBlacklist();
    void slotChooseActiveDirectory();
    void slotBuild();
    void slotBuildActiveDir();
    void slotCompileFile();
    void slotInstall();
    void slotInstallActiveDir();
    void slotInstallWithKdesu();
    void slotClean();
    void slotExecute();
    void updateTargetMenu();
    void targetMenuActivated( int id );
    void targetObjectFilesMenuActivated( int id );
    void targetOtherFilesMenuActivated( int id );
    void updateMakeEnvironmentsMenu();
    void makeEnvironmentsMenuActivated( int id );
    void slotCommandFinished( const QString& command );
    void slotCommandFailed( const QString& command );

private:
    bool containsNonProjectFiles( const QString& url );
    QStringList projectFilesInDir( const QString& dir );
    bool containsProjectFiles( const QString& url );
    bool isProjectFileType( const QString& absFile ) const;
    bool isInBlacklist( const QString& ) const;
    void cleanFileList();
    void setFiletypes( const QStringList& );
    void findNewFiles( const QString& dir, QStringList& list) const;

    QStringList filetypes() const;
    QStringList blacklist() const;
    void updateBlacklist( const QStringList& );
    void saveProject();
    void startMakeCommand( const QString &dir, const QString &target, bool withKdesu = false );
    void parseMakefile( const QString& file );
    QString makeEnvironment() const;
    void putEnvVarsInVarMap();
    void switchBlacklistEntry(const QString& );

    bool isInProject( const QString& fileName ) const;
    void addToProject( const QString& fileName );
    void removeFromProject( const QString& fileName );

    /**
     * @brief Set of all the project's files.
     *
     * @bug
     * Due to deficiency in QT3,
     * we have to use a map with next-to-useless element value,
     * keyed by the file name,
     * as a set-container replacement.
     */
    typedef QMap<QString, bool> ProjectFilesSet;

    QString m_projectDirectory;
    QString m_projectName;
    QString m_filelistDir;
    /** All the sources (files and directories) of this project. */
    ProjectFilesSet m_sourceFilesSet;
    QPopupMenu *m_targetMenu;
    QPopupMenu *m_targetObjectFilesMenu;
    QPopupMenu *m_targetOtherFilesMenu;
    KSelectAction *m_makeEnvironmentsSelector;
    QStringList m_targets;
    QStringList m_targetsObjectFiles;
    QStringList m_targetsOtherFiles;
    QStringList m_contextAddFiles;
    QStringList m_contextRemoveFiles;
    QString m_contextDirName;

    QMap<QString, QDateTime> m_timestamp;
    bool m_executeAfterBuild;
    QString m_buildCommand;
    bool m_lastCompilationFailed;
    QMap<QString, int> m_parsedMakefiles;
    QValueStack<QString> m_makefilesToParse;
    QMap<QString, QString> m_makefileVars;
    bool m_recursive;
    bool m_first_recursive;
};

#endif
// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
