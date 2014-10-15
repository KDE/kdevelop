/* This file is part of KDevelop
    Copyright 2004 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2007 Andreas Pakulat <apaku@gmx.de>
    Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>

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

#include <interfaces/iplugin.h>

#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtCore/QVariant>

#include "imakebuilder.h"
#include "makejob.h"

#include <util/objectlist.h>

namespace KDevelop {
class ProjectBaseItem;
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
    explicit MakeBuilder(QObject *parent = 0, const QVariantList &args = QVariantList());
    virtual ~MakeBuilder();

    /**
     * If argument is ProjectItem, invoke "make" in IBuildSystemManager::buildDirectory(), with
     * specified target in project setting.
     *
     * If argument is ProjectTargetItem, invoke "make" with targetname QStandardItem::text(). In this case,
     * it tries its best to fetch ProjectItem, which is the argument of IBuildSystemManager::buildDirectory()
     * If it fails to fetch ProjectItem, the top build directory is defaulted to project directory.
     * Based on top build directory, the actual build_dir is computed and handed to outputview
     *
     * If argument is ProjectBuildFolderItem, invoke "make" with specified target in project setting.
     * To determine the build directory, first calculates rel_dir between
     * top_project_dir(ProjectItem::url()) and ProjectBuildFolderItem::url().
     * Then invokes make in top_build_dir/rel_dir.
     * If this fails to fetch top_build_dir, just invoke "make" in ProjectBuildFolderItem::url().
     *
     * @TODO: Work on any project item, for fileitems you may find a target.
     */
    virtual KJob* build(KDevelop::ProjectBaseItem *dom);
    virtual KJob* clean(KDevelop::ProjectBaseItem *dom);
    virtual KJob* install(KDevelop::ProjectBaseItem *dom);

    virtual KJob* executeMakeTarget(KDevelop::ProjectBaseItem* item, const QString& targetname );
    virtual KJob* executeMakeTargets(KDevelop::ProjectBaseItem* item, const QStringList& targetnames,
                                     const MakeVariables& variables = MakeVariables() );
    KJob* runMake( KDevelop::ProjectBaseItem*, MakeJob::CommandType, const QStringList& = QStringList(),
                   const MakeVariables& variables = MakeVariables() );

    virtual int perProjectConfigPages() const override;
    virtual KDevelop::ConfigPage* perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent) override;

Q_SIGNALS:
    void built( KDevelop::ProjectBaseItem* );
    void failed( KDevelop::ProjectBaseItem* );
    void installed(KDevelop::ProjectBaseItem*);
    void cleaned(KDevelop::ProjectBaseItem*);
    void makeTargetBuilt( KDevelop::ProjectBaseItem* item, const QString& targetname );

private Q_SLOTS:
    void jobFinished(KJob* job);

private:
    KDevelop::ObjectList<MakeJob> m_activeMakeJobs;
};

#endif // KDEVMAKEBUILDER_H

