/* This file is part of KDevelop
    Copyright (C) 2004 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef __KDEVPART_KDEVPROJECTMANAGER_H__
#define __KDEVPART_KDEVPROJECTMANAGER_H__

#include "kdevprojectmodel.h"

#include <qguardedptr.h>
#include <kdevproject.h>

class KDevProjectManagerWidget;
class KDevProjectImporter;
class KDevProjectBuilder;
class ProjectModel;

/*
 Please read the README.dox file for more info about this part
 */
class KDevProjectManagerPart: public KDevProject
{
    Q_OBJECT
public:
    enum RefreshPolicy
    {
        Refresh,
        NoRefresh,
        ForceRefresh
    };
    
    enum BuildPolicy
    {
        Build,
        BuildWithSU
    };
    
public:
    KDevProjectManagerPart(QObject *parent, const char *name, const QStringList &);
    virtual ~KDevProjectManagerPart();

    inline ProjectModel *projectModel() const
    { return m_projectModel; }  
    
    inline bool isDirty() const
    { return m_dirty; }

    KDevProjectImporter *defaultImporter() const;
    KDevProjectBuilder *defaultBuilder() const;
    
//
// KDevProject interface
//    
    virtual void openProject(const QString &dirName, const QString &projectName);
    virtual void closeProject();
    virtual Options options() const;
    virtual QString projectDirectory() const;
    virtual QString projectName() const;
    virtual DomUtil::PairList runEnvironmentVars() const;
    virtual QString mainProgram(bool relative = false) const;
    virtual QString runDirectory() const;
    virtual QString runArguments() const;
    virtual QString activeDirectory() const;
    virtual QString buildDirectory() const;
    virtual QStringList allFiles() const;
    virtual QStringList distFiles() const;
    virtual void addFiles(const QStringList &fileList);
    virtual void addFile(const QString &fileName);
    virtual void removeFiles (const QStringList &fileList);
    virtual void removeFile(const QString &fileName);
    
    void import(RefreshPolicy policy = Refresh);
    void build(const QString &directory, const QString &target, BuildPolicy buildPolicy = Build);

signals:    
    void refresh();
    void addedProjectItem(ProjectItemDom dom);
    void aboutToRemoveProjectItem(ProjectItemDom dom);
        
private slots:
    void fileDirty(const QString &fileName);
    void fileCreated(const QString &fileName);
    void fileDeleted(const QString &fileName);
    
protected:
    bool computeChanges(const QStringList &oldFileList, const QStringList &newFileList);
    QStringList fileList(ProjectItemDom item);
    QStringList allFiles();
    
private:
    ProjectModel *m_projectModel;
    ProjectFolderDom m_workspace;
    QGuardedPtr<KDevProjectManagerWidget> m_widget;
    QMap<QString, KDevProjectImporter*> m_importers;
    QMap<QString, KDevProjectBuilder*> m_builders;
    QStringList m_cachedFileList;
    
    QString m_projectDirectory;
    QString m_projectName;
    
    bool m_dirty;
};

#endif
