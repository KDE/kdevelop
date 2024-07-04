/*
    SPDX-FileCopyrightText: 2004 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2007 Dukju Ahn <dukjuahn@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MAKEBUILDER_H
#define MAKEBUILDER_H

#include <interfaces/iplugin.h>

#include <QVariantList>

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
    explicit MakeBuilder(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args = QVariantList());
    ~MakeBuilder() override;

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
    KJob* build(KDevelop::ProjectBaseItem *dom) override;
    KJob* clean(KDevelop::ProjectBaseItem *dom) override;
    KJob* install(KDevelop::ProjectBaseItem *dom, const QUrl &installPath) override;

    KJob* executeMakeTarget(KDevelop::ProjectBaseItem* item, const QString& targetname ) override;
    KJob* executeMakeTargets(KDevelop::ProjectBaseItem* item, const QStringList& targetnames,
                                     const MakeVariables& variables = MakeVariables() ) override;
    KJob* runMake( KDevelop::ProjectBaseItem*, MakeJob::CommandType, const QStringList& = QStringList(),
                   const MakeVariables& variables = MakeVariables() );

    int perProjectConfigPages() const override;
    KDevelop::ConfigPage* perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent) override;

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

