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
#ifndef MAKEBUILDER_H
#define MAKEBUILDER_H

#include <iplugin.h>
#include "imakebuilder.h"
#include <QtCore/QList>
#include <QtCore/QPair>


class QStringList;
class MakeOutputModel;
class KDialog;
class QSignalMapper;
class QString;
class QStandardItem;
class KUrl;

namespace KDevelop {
class ProjectBaseItem;
class ProjectItem;
class CommandExecutor;
}

/**
@author Roberto Raggi
*/
class MakeBuilder: public KDevelop::IPlugin, public IMakeBuilder
{
    Q_OBJECT
    Q_INTERFACES( IMakeBuilder )
    Q_INTERFACES( KDevelop::IProjectBuilder )
public:
    explicit MakeBuilder(QObject *parent = 0, const QStringList &args = QStringList());
    virtual ~MakeBuilder();

    /**
     * If argument is ProjectItem, invoke "make" in IBuildSystemManager::buildDirectory(), with no target
     *
     * If argument is ProjectTargetItem, invoke "make" with targetname QStandardItem::text(). In this case,
     * it tries its best to fetch ProjectItem, which is the argument of IBuildSystemManager::buildDirectory()
     * If it fails to fetch ProjectItem, the top build directory is defaulted to project directory.
     * Based on top build directory, the actual build_dir is computed and handed to outputview
     *
     * If argument is ProjectBuildFolderItem, invoke "make" with no target. To determine the build directory,
     * first calculates rel_dir between top_project_dir(ProjectItem::url()) and ProjectBuildFolderItem::url().
     * Then invokes make in top_build_dir/rel_dir.
     * If this fails to fetch top_build_dir, just invoke "make" in ProjectBuildFolderItem::url().
     *
     * @TODO: Work on any project item, for fileitems you may find a target.
     */
    virtual bool build(KDevelop::ProjectBaseItem *dom);
    virtual bool clean(KDevelop::ProjectBaseItem *dom);

Q_SIGNALS:
    void built( KDevelop::ProjectBaseItem* );
    void failed( KDevelop::ProjectBaseItem* );

private Q_SLOTS:
    void commandFinished(const QString &id);
    void commandFailed(const QString &id);
    void cleanupModel( const QString& id );

private:
    QStringList computeBuildCommand(KDevelop::ProjectBaseItem *item);
    KUrl computeBuildDir( KDevelop::ProjectBaseItem* item );


private:
    QMap< KDevelop::IProject*, QString > m_ids;
    QMap< QString, KDevelop::CommandExecutor* > m_commands;
    QMap< QString, KDevelop::ProjectBaseItem* > m_items;
    QMap< QString, MakeOutputModel* > m_models;
    QSignalMapper* errorMapper;
    QSignalMapper* successMapper;
};

#endif // KDEVMAKEBUILDER_H

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;
