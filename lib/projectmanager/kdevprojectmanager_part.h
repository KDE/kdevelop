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

#include <QtCore/QPointer>
#include "kdevplugin.h"

class QTimer;
class QModelIndex;

class KUrl;

class KDevProjectItem;
class KDevProjectModel;
class KDevProjectManager;
class KDevProjectBuilder;
class KDevProjectImporter;
class KDevProjectFileItem;
class KDevProjectFolderItem;
class KDevProjectTargetItem;

/*
 Please read the README.dox file for more info about this part
 */
class KDevProjectManagerPart: public KDevPlugin
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

    KDevProjectFolderItem *activeFolder();
    KDevProjectTargetItem *activeTarget();
    KDevProjectFileItem *activeFile();

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
    void pressed( const QModelIndex & index );

protected:
    bool computeChanges(const QStringList &oldFileList, const QStringList &newFileList);

private:
    QPointer<QWidget> m_widget;
    KDevProjectManager *m_projectOverview;
    KDevProjectManager *m_projectDetails;
    QStringList m_cachedFileList;
};

#endif

// kate: space-indent on; indent-width 2; replace-tabs on;
