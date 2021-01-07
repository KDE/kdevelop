/* This file is part of KDevelop
    Copyright 2017 Aleix Pol Gonzalez <aleixpol@kde.org>
    Copyright 2018 Daniel Mensinger <daniel@mensinger-ka.de>
    Copyright 2021 BogDan Vatra <bogdan@kde.org>

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

#include "gnconfig.h"

#include <project/abstractfilemanagerplugin.h>
#include <project/interfaces/ibuildsystemmanager.h>

#include <memory>

class GNBuilder;
class KDirWatch;

using KDirWatchPtr = std::shared_ptr<KDirWatch>;

class GNManager : public KDevelop::AbstractFileManagerPlugin, public KDevelop::IBuildSystemManager
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IBuildSystemManager)

public:
    explicit GNManager(QObject* parent = nullptr, const QVariantList& args = QVariantList());
    ~GNManager() override;

    // ********************
    // * Custom functions *
    // ********************

    /**
     * Create a new build directory and write it into the config.
     * @returns The configuration job on success or nullptr on error.
     */
    KJob* newBuildDirectory(KDevelop::IProject* project, GN::BuildDir* outBuildDir = nullptr);

    /// Returns a list of all supported GN backends (for now only ninja)
    QString defaultGNBackend() const;

    KDevelop::Path findGN() const;

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

    // Not sure when/if these will be implemented. This would require modifying gn files. Regardless: TODO
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
    void onGNInfoChanged(QString path, QString projectName);

private:
    GNBuilder* m_builder;
    QHash<KDevelop::IProject*, KDirWatchPtr> m_projectWatchers;
};
