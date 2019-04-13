/* This file is part of KDevelop
    Copyright 2017 Aleix Pol Gonzalez <aleixpol@kde.org>
    Copyright 2018 Daniel Mensinger <daniel@mensinger-ka.de>

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

#pragma once

#include "mesonconfig.h"
#include "mesontests.h"

#include <project/abstractfilemanagerplugin.h>
#include <project/interfaces/ibuildsystemmanager.h>

#include <memory>

class MesonBuilder;
class MesonTarget;
class MesonTargets;
class MesonTargetSources;

class KDirWatch;

using MesonSourcePtr = std::shared_ptr<MesonTargetSources>;
using MesonTargetsPtr = std::shared_ptr<MesonTargets>;
using KDirWatchPtr = std::shared_ptr<KDirWatch>;

class MesonManager : public KDevelop::AbstractFileManagerPlugin, public KDevelop::IBuildSystemManager
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IBuildSystemManager)

public:
    explicit MesonManager(QObject* parent = nullptr, const QVariantList& args = QVariantList());
    ~MesonManager() override;

    // ********************
    // * Custom functions *
    // ********************

    /**
     * Create a new build directory and write it into the config.
     * @returns The configuration job on success or nullptr on error.
     */
    KJob* newBuildDirectory(KDevelop::IProject* project);

    /// Returns a list of all supported Meson backends (for now only ninja)
    QStringList supportedMesonBackends() const;
    QString defaultMesonBackend() const;

    KDevelop::Path findMeson() const;

    // *********************************
    // * AbstractFileManagerPlugin API *
    // *********************************

    KDevelop::IProjectFileManager::Features features() const override;
    KDevelop::ProjectFolderItem* createFolderItem(KDevelop::IProject* project, const KDevelop::Path& path,
                                                  KDevelop::ProjectBaseItem* parent = nullptr) override;

    bool reload(KDevelop::ProjectFolderItem* item) override;

    // ***********
    // * IPlugin *
    // ***********

    KDevelop::ConfigPage* perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options,
                                               QWidget* parent) override;
    int perProjectConfigPages() const override;

    // ***************************
    // * IBuildSystemManager API *
    // ***************************

    KJob* createImportJob(KDevelop::ProjectFolderItem* item) override;

    KDevelop::IProjectBuilder* builder() const override;

    KDevelop::Path::List includeDirectories(KDevelop::ProjectBaseItem* item) const override;
    KDevelop::Path::List frameworkDirectories(KDevelop::ProjectBaseItem* item) const override;
    QHash<QString, QString> defines(KDevelop::ProjectBaseItem* item) const override;
    QString extraArguments(KDevelop::ProjectBaseItem* item) const override;
    bool hasBuildInfo(KDevelop::ProjectBaseItem* item) const override;

    KDevelop::Path buildDirectory(KDevelop::ProjectBaseItem*) const override;
    QList<KDevelop::ProjectTargetItem*> targets(KDevelop::ProjectFolderItem*) const override;

    // Not sure when/if these will be implemented. This would require modifying meson files. Regardless: TODO
    KDevelop::ProjectTargetItem* createTarget(const QString& /*target*/,
                                              KDevelop::ProjectFolderItem* /*parent*/) override
    {
        return nullptr;
    }
    bool removeTarget(KDevelop::ProjectTargetItem* /*target*/) override { return false; }
    bool addFilesToTarget(const QList<KDevelop::ProjectFileItem*>& /*files*/,
                          KDevelop::ProjectTargetItem* /*target*/) override
    {
        return false;
    }
    bool removeFilesFromTargets(const QList<KDevelop::ProjectFileItem*>& /*files*/) override { return false; }

    KDevelop::Path compiler(KDevelop::ProjectTargetItem* p) const override;

private:
    void onMesonInfoChanged(QString path, QString projectName);

private:
    MesonBuilder* m_builder;
    QHash<KDevelop::IProject*, MesonTargetsPtr> m_projectTargets;
    QHash<KDevelop::IProject*, MesonTestSuitesPtr> m_projectTestSuites;
    QHash<KDevelop::IProject*, KDirWatchPtr> m_projectWatchers;

    MesonSourcePtr sourceFromItem(KDevelop::ProjectBaseItem* item) const;
    void populateTargets(KDevelop::ProjectFolderItem* item, QVector<MesonTarget*> targets);
};
