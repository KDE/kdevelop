/* This file is part of KDevelop
    Copyright 2019 Daniel Mensinger <daniel@mensinger-ka.de>
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

#include <interfaces/iproject.h>

#include <KJob>

#include <QFutureWatcher>

#include <QJsonObject>
#include <memory>
#include <path.h>

class KDirWatch;
using KDirWatchPtr = std::shared_ptr<KDirWatch>;

class GNBuildSettings
{
public:
    GNBuildSettings() = default;
    explicit GNBuildSettings(const QJsonObject& buildSettings);
    const QString &rootPath() const;
    const QString &defaultToolchain() const;
    const QVector<QString>& projectFiles() const;

private:
    QString m_rootPath;
    QString m_defaultToolchain;
    QVector<QString> m_projectFiles;
};

using GNTargetDataInfoPtr = std::shared_ptr<struct GNTargetDataInfo>;

class GNTargetData
{
public:
    const KDevelop::Path::List &includeDirectories() const;
    const KDevelop::Path::List &frameworkDirectories() const;
    const QHash<QString,QString> &defines() const;
    const QString &extraArguments() const;
    const KDevelop::Path& compiler() const;

protected:
    explicit GNTargetData(const GNTargetDataInfoPtr &targetDataInfo);

private:
    GNTargetDataInfoPtr m_targetDataInfo;
};


class GNImportProjectJob : public KJob
{
    Q_OBJECT

public:
    explicit GNImportProjectJob(KDevelop::IProject* project, QObject* parent);
    explicit GNImportProjectJob(KDevelop::IProject* project, KDevelop::Path gn, QObject* parent);
    explicit GNImportProjectJob(KDevelop::IProject* project, GN::BuildDir buildDir, QObject* parent);

    void start() override;
    bool doKill() override;

    void importProject(const KDirWatchPtr &watcher);
    const GNBuildSettings& buildSettings();

private:
    QString import(GN::BuildDir buildDir);
    void finished();

    QFutureWatcher<QString> m_futureWatcher;

    // The commands to execute
    GN::BuildDir m_buildDir;
    KDevelop::Path m_projectPath;
    KDevelop::IProject* m_project = nullptr;

    QJsonObject m_projectInfo;
    GNBuildSettings m_buildSettings;
};
