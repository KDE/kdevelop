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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#ifndef __KDEVPART_KDEVPROJECTMANAGER_H__
#define __KDEVPART_KDEVPROJECTMANAGER_H__

#include "kdevprojectmodel.h"

#include <QtCore/QPointer>
#include <kdevproject.h>

class KDevProjectManager;
class KDevProjectImporter;
class KDevProjectBuilder;
class KDevProjectModel;
class QTimer;
class KUrl;

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

public:
    KDevProjectManagerPart(QObject *parent, const QStringList &);
    virtual ~KDevProjectManagerPart();

    inline KDevProjectModel *projectModel() const
    { return m_projectModel; }

    inline bool isDirty() const
    { return m_dirty; }

    KDevProjectFolderItem *activeFolder();
    KDevProjectTargetItem *activeTarget();
    KDevProjectFileItem *activeFile();

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

signals:
    void refresh();
    void addedProjectItem(KDevProjectItem *dom);
    void aboutToRemoveProjectItem(KDevProjectItem *dom);

public slots:
    void openURL(const KUrl &url);
    void updateDetails(KDevProjectItem *item);

private slots:
    void fileDirty(const QString &fileName);
    void fileCreated(const QString &fileName);
    void fileDeleted(const QString &fileName);
    void updateProjectTimeout();

protected:
    bool computeChanges(const QStringList &oldFileList, const QStringList &newFileList);
    QStringList fileList(KDevProjectItem *item);
    QStringList allFiles();

private:
    KDevProjectModel *m_projectModel;
    KDevProjectFolderItem *m_workspace;
    QPointer<QWidget> m_widget;
    KDevProjectManager *m_projectOverview;
    KDevProjectManager *m_projectDetails;
    QMap<QString, KDevProjectImporter*> m_importers;
    QMap<QString, KDevProjectBuilder*> m_builders;
    QStringList m_cachedFileList;

    QString m_projectDirectory;
    QString m_projectName;

    bool m_dirty;

    QTimer *m_updateProjectTimer;
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
