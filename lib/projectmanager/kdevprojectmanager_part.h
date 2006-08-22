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
#include "kdevproject.h"

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

    KDevFileManager *defaultImporter() const;

    //
    // KDevProject interface
    //
    virtual void openProject(const KUrl &dirName, const QString &projectName);
    virtual void closeProject();
    virtual KUrl projectDirectory() const;
    virtual QString projectName() const;
    virtual QList<KDevProjectFileItem*> allFiles();

    void import(RefreshPolicy policy = Refresh);

    // KDevPlugin methods
    virtual QWidget *pluginView() const;
    virtual Qt::DockWidgetArea dockWidgetAreaHint() const;

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
    void pressed( const QModelIndex & index );

protected:
    bool computeChanges(const QStringList &oldFileList, const QStringList &newFileList);
    QStringList fileList(KDevProjectItem *item);
    QStringList fileList();
    QList<KDevProjectFileItem*> recurseFiles(KDevProjectItem *item);

private:
    KDevProjectModel *m_projectModel;
    KDevProjectFolderItem *m_workspace;
    QPointer<QWidget> m_widget;
    KDevProjectManager *m_projectOverview;
    KDevProjectManager *m_projectDetails;
    KDevFileManager* m_manager;
    QStringList m_cachedFileList;

    KUrl m_projectDirectory;
    QString m_projectName;

    bool m_dirty;

    QTimer *m_updateProjectTimer;
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
