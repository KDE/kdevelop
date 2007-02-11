/* This file is part of KDevelop
    Copyright (C) 2004 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>

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
#ifndef KDEVPROJECTMANAGER_PART_H
#define KDEVPROJECTMANAGER_PART_H

#include <QtCore/QPointer>
#include "iplugin.h"

class QTimer;
class QModelIndex;

class KUrl;

namespace KDevelop
{

class ProjectBaseItem;
class ProjectModel;
class ProjectManager;
class ProjectBuilder;
class ProjectImporter;
class ProjectFileItem;
class ProjectFolderItem;
class ProjectTargetItem;

class ProjectManagerPart: public IPlugin
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
    ProjectManagerPart(QObject *parent, const QStringList &);
    virtual ~ProjectManagerPart();

//     ProjectFolderItem *activeFolder();
//     ProjectTargetItem *activeTarget();
//     ProjectFileItem *activeFile();

    // Plugin methods
    virtual void unload();

Q_SIGNALS:
    void refresh();
    void addedProjectItem(ProjectBaseItem *dom);
    void aboutToRemoveProjectItem(ProjectBaseItem *dom);

public Q_SLOTS:
    void openURL(const KUrl &url);
    void updateDetails(ProjectBaseItem *item);

protected:
    bool computeChanges(const QStringList &oldFileList, const QStringList &newFileList);

private:
    class ProjectManagerPartPrivate* const d;

};

}
#endif

//kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
