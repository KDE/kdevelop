/* This file is part of KDevelop
    Copyright 2004 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2007 Andreas Pakulat <apaku@gmx.de>

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
#ifndef KDEVPROJECTMANAGERVIEW_PART_H
#define KDEVPROJECTMANAGERVIEW_PART_H

#include <QtCore/QPointer>
#include "iplugin.h"
#include <QtCore/QVariant>

class KUrl;
class ProjectBuildSetModel;

namespace KDevelop
{
class ProjectBaseItem;
class ProjectBuilder;
class ProjectFileItem;
class ProjectFolderItem;
class ProjectTargetItem;
class IProjectBuilder;
class IProject;
class ContextMenuExtension;
}

class ProjectManagerView;

class ProjectManagerViewPlugin: public KDevelop::IPlugin
{
    Q_OBJECT
public:

public:
    ProjectManagerViewPlugin(QObject *parent, const QVariantList & = QVariantList() );
    virtual ~ProjectManagerViewPlugin();

    // Plugin methods
    virtual void unload();

    KDevelop::ContextMenuExtension contextMenuExtension( KDevelop::Context* );

    ProjectBuildSetModel* buildSet();

protected Q_SLOTS:
    void storeBuildset();
    void closeProjects();
    void buildItemsFromContextMenu();
    void installItemsFromContextMenu();
    void cleanItemsFromContextMenu();
    void buildProjectItems();
    void installProjectItems();
    void cleanProjectItems();
    void configureProjectItems();
    void pruneProjectItems();
    void buildAllProjects();
    void addItemsFromContextMenuToBuildset();
    void projectConfiguration();

private:
    void executeBuild( KDevelop::ProjectBaseItem* );
    void executeClean( KDevelop::ProjectBaseItem* );
    void executeInstall( KDevelop::ProjectBaseItem* );
    void executePrune( KDevelop::IProject* );
    void executeConfigure( KDevelop::IProject* );
    KDevelop::IProjectBuilder* getProjectBuilder( KDevelop::ProjectBaseItem* item );
    QList<KDevelop::ProjectBaseItem*> recurseAndFetchCheckedItems( KDevelop::ProjectBaseItem* item );
    class ProjectManagerViewPluginPrivate* const d;

};

#endif

