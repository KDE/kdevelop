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

#include "gnimportprojectjob.h"

#include "gnconfig.h"
#include "gnmanager.h"
#include <debug.h>

#include <KDirWatch>
#include <KLocalizedString>
#include <KProcess>

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtConcurrentRun>
#include <projectmodel.h>

using namespace GN;
using namespace KDevelop;

const GNBuildSettings &GNImportProjectJob::buildSettings()
{
    return m_buildSettings;
}

GNBuildSettings::GNBuildSettings(const QJsonObject &buildSettings)
{
    m_rootPath = buildSettings[QLatin1String("root_path")].toString();
    m_defaultToolchain = buildSettings[QLatin1String("default_toolchain")].toString();
    for (const auto &jv: buildSettings[QLatin1String("gen_input_files")].toArray())
        m_projectFiles << GN::relativeFilePath(jv.toString());
}

const QString &GNBuildSettings::rootPath() const
{
    return m_rootPath;
}

const QString &GNBuildSettings::defaultToolchain() const
{
    return m_defaultToolchain;
}

const QVector<QString> &GNBuildSettings::projectFiles() const
{
    return m_projectFiles;
}

struct GNTargetDataInfo
{
    KDevelop::Path::List includeDirectories;
    KDevelop::Path::List frameworkDirectories;
    QHash<QString,QString> defines;
    QString extraArguments;
    KDevelop::Path compiler;
};

const Path::List &GNTargetData::includeDirectories() const
{
    return m_targetDataInfo->includeDirectories;
}

const Path::List &GNTargetData::frameworkDirectories() const
{
    return m_targetDataInfo->frameworkDirectories;
}

const QHash<QString, QString> &GNTargetData::defines() const
{
    return m_targetDataInfo->defines;
}

const QString &GNTargetData::extraArguments() const
{
    return m_targetDataInfo->extraArguments;
}

const Path &GNTargetData::compiler() const
{
    return m_targetDataInfo->compiler;
}

GNTargetData::GNTargetData(const GNTargetDataInfoPtr &targetDataInfo)
    : m_targetDataInfo(targetDataInfo)
{}

// GN Specific Project Items
class GNProjectFileItem : public ProjectFileItem, public GNTargetData
{
public:
    GNProjectFileItem(IProject* project, const Path& path, ProjectBaseItem* parent, const GNTargetDataInfoPtr & targetData);
};

GNProjectFileItem::GNProjectFileItem(IProject *project, const Path &path, ProjectBaseItem *parent, const GNTargetDataInfoPtr &targetData)
    : ProjectFileItem(project, path, parent)
    , GNTargetData(targetData)
{}


class GNProjectExecutableTargetItem final : public ProjectExecutableTargetItem
{
public:
    GNProjectExecutableTargetItem(IProject* project, const QString& name, ProjectBaseItem* parent, Path buildPath)
        : ProjectExecutableTargetItem(project, name, parent)
        , m_buildPath(buildPath)
    {}

    QUrl builtUrl() const override { return m_buildPath.toUrl(); }
    QUrl installedUrl() const override { return {}; }

private:
    Path m_buildPath;
};
// GN Specific Project Items



GNImportProjectJob::GNImportProjectJob(KDevelop::IProject* project, QObject* parent)
    : KJob(parent)
    , m_project(project)
{
    Q_ASSERT(project);

    m_buildDir = GN::currentBuildDir(project);

    m_projectPath = project->path();
    connect(&m_futureWatcher, &QFutureWatcher<QString>::finished, this, &GNImportProjectJob::finished);
}

GNImportProjectJob::GNImportProjectJob(IProject *project, Path gn, QObject *parent)
     : KJob(parent)
     , m_project(project)
{
    Q_ASSERT(project);
    // Since we are parsing the gn file in this mode, no build directory
    // is required and we have to fake a build directory
    m_projectPath = project->path();
    m_buildDir.buildDir = m_projectPath;
    m_buildDir.gnExecutable = gn;
    connect(&m_futureWatcher, &QFutureWatcher<QString>::finished, this, &GNImportProjectJob::finished);
}

GNImportProjectJob::GNImportProjectJob(KDevelop::IProject* project, GN::BuildDir buildDir, QObject* parent)
    : KJob(parent)
    , m_buildDir(buildDir)
    , m_project(project)
{
    Q_ASSERT(project);
    m_projectPath = project->path();
    connect(&m_futureWatcher, &QFutureWatcher<QString>::finished, this, &GNImportProjectJob::finished);
}


static QString importJSONFile(const BuildDir& buildDir, QJsonObject* out)
{
    QString fileName = QStringLiteral("project.json");
    QFile introFile(buildDir.buildDir.toLocalFile() + QStringLiteral("/") + fileName);

    if (!introFile.exists()) {
        return i18n("Project info file '%1' does not exist", QFileInfo(introFile).canonicalFilePath());
    }

    if (!introFile.open(QFile::ReadOnly | QFile::Text)) {
        return i18n("Failed to open project info file '%1'", QFileInfo(introFile).canonicalFilePath());
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(introFile.readAll(), &error);
    if (error.error) {
        return i18n("In %1:%2: %3", QFileInfo(introFile).canonicalFilePath(), error.offset, error.errorString());
    }

    if (doc.isObject()) {
        *out = doc.object();
    } else {
        return i18n("Invalid project info file '%1'",
                    QFileInfo(introFile).canonicalFilePath());
    }
    return QString();
}


QString GNImportProjectJob::import(BuildDir buildDir)
{
    QString err = importJSONFile(buildDir, &m_projectInfo);
    if (!err.isEmpty()) {
        qCWarning(KDEV_GN) << "GN Import: " << err;
        setError(true);
        setErrorText(err);
        return err;
    }
    return err;
}

void GNImportProjectJob::start()
{
    qCDebug(KDEV_GN) << "GN Import: Starting gn introspection job";
    if (!m_buildDir.isValid()) {
        qCWarning(KDEV_GN) << "The current build directory is invalid";
        setError(true);
        setErrorText(i18n("The current build directory is invalid"));
        emitResult();
        return;
    }

    auto future = QtConcurrent::run(this, &GNImportProjectJob::import, m_buildDir);
    m_futureWatcher.setFuture(future);
}

void GNImportProjectJob::finished()
{
    qCDebug(KDEV_GN) << "GN Import: GN introspection job finished";
    emitResult();
}

bool GNImportProjectJob::doKill()
{
    if (m_futureWatcher.isRunning()) {
        m_futureWatcher.cancel();
    }
    return true;
}

using FolderHash = QHash<Path, ProjectFolderItem*>;
static ProjectFolderItem* getFolder(IProject* project, FolderHash *hash, const Path &folder)
{
    if (folder.isEmpty() || folder.path() == QLatin1Char('/')) {
        return project->projectItem();
    }
    auto it = hash->find(folder);
    if (it != hash->end())
        return *it;
    auto dir = new ProjectFolderItem(project, folder, getFolder(project, hash, folder.parent()));
    hash->insert(folder, dir);
    return dir;
}

void GNImportProjectJob::importProject(const KDirWatchPtr &watcher)
{
    auto item = m_project->projectItem();
    item->removeRows(0, item->rowCount());
    auto buildSettingsJSON = m_projectInfo[QStringLiteral("build_settings")];
    if (!buildSettingsJSON.isObject())
        return;
    m_buildSettings = GNBuildSettings{buildSettingsJSON.toObject()};

    FolderHash folderHash;
    for (const auto &file : m_buildSettings.projectFiles()) {
        Path path{QUrl::fromLocalFile(m_buildSettings.rootPath())};
        path.addPath(file);
        watcher->addFile(path.toLocalFile());
        new ProjectFileItem{m_project, path,
                    getFolder(m_project, &folderHash, Path{QUrl::fromLocalFile(file)}.parent())};
    }


    QHash<Path, ProjectFolderItem*> foldersCache;
    auto targetsJSON = m_projectInfo[QStringLiteral("targets")].toObject();
    for (auto targetIt = targetsJSON.begin(); targetIt != targetsJSON.end(); ++targetIt) {
        auto targetJsonData = targetIt->toObject();
        auto type  = targetJsonData[QLatin1String("type")].toString();
        if (type != QLatin1String("executable") &&
                type != QLatin1String("shared_library") &&
                type != QLatin1String("loadable_module") &&
                type != QLatin1String("static_library") &&
                type != QLatin1String("source_set")) {
            continue;
        }
        auto fullName = targetIt.key();
        // The target name has 3 parts splitted by ':'
        // first part is the path
        // second one is the target name
        // last (optional) one is the toolchain using to build it
        int splitPos = fullName.indexOf(QLatin1Char(':'));
        assert(splitPos != -1);
        auto path = GN::relativeFilePath(fullName.left(splitPos));
        auto name = fullName.mid(splitPos + 1);
        auto targetData = [&]() {
            auto targetData = std::make_shared<GNTargetDataInfo>();
            auto it = targetJsonData.find(QLatin1String("cflags"));
            QStringList args;
            if (it != targetJsonData.end()) {
                for (auto v : it->toArray())
                    args << v.toString();
            }
            it = targetJsonData.find(QLatin1String("cflags_cc"));
            if (it != targetJsonData.end()) {
                for (auto v : it->toArray())
                    args << v.toString();
            }
            targetData->extraArguments = args.join(QLatin1Char(' '));

            it = targetJsonData.find(QLatin1String("defines"));
            if (it != targetJsonData.end()) {
                for (auto v : it->toArray()) {
                    auto str = v.toString();
                    int pos = str.indexOf(QLatin1Char('='));
                    if (pos == -1)
                        targetData->defines.insert(str, {});
                    else
                        targetData->defines.insert(str.left(pos), str.mid(pos + 1));
                }
            }
            it = targetJsonData.find(QLatin1String("include_dirs"));
            if (it != targetJsonData.end()) {
                for (auto v : it->toArray()) {
                    targetData->includeDirectories
                            << Path{QUrl::fromLocalFile(GN::absoluteFilePath(m_buildSettings.rootPath(), v.toString()))};
                }
            }

            it = targetJsonData.find(QLatin1String("framework_dirs"));
            if (it != targetJsonData.end()) {
                for (auto v : it->toArray()) {
                    targetData->frameworkDirectories
                            << Path{QUrl::fromLocalFile(GN::absoluteFilePath(m_buildSettings.rootPath(), v.toString()))};
                }
            }
            return targetData;
        };


        ProjectBaseItem* parentTarget;
        if (type == QLatin1String("executable")) {
            Path builtPath;
            auto it = targetJsonData.find(QLatin1String("outputs"));
            if (it != targetJsonData.end()) {
                auto outputs = it->toArray();
                if (outputs.count() > 1) {
                    qCWarning(KDEV_GN) << "Target " << fullName << " has more than one output, we'll pick the first one";
                }
                builtPath = Path{QUrl::fromLocalFile(m_buildSettings.rootPath())}.addPath(outputs.at(0).toString());
            } else {
                qCWarning(KDEV_GN) << "Target " << fullName << " has no outputs";
            }
            parentTarget = new GNProjectExecutableTargetItem{m_project, name, getFolder(m_project, &folderHash, Path{QUrl::fromLocalFile(path)}), builtPath};
        } else {
            parentTarget = new ProjectLibraryTargetItem{m_project, name, getFolder(m_project, &folderHash, Path{QUrl::fromLocalFile(path)})};
        }
        auto it = targetJsonData.find(QLatin1String("sources"));
        if (it != targetJsonData.end()) {
            auto sources = it->toArray();
            auto tagetDataPtr = targetData();
            for (const auto &jv: sources) {
                new GNProjectFileItem{m_project, Path{QUrl::fromLocalFile(GN::absoluteFilePath(m_buildSettings.rootPath(), jv.toString()))},
                            parentTarget, tagetDataPtr};
            }
        }
        // TODO: create a testsuite here
//            it = targetData.find(QLatin1String("testonly"));
//            bool testOnly = it != targetData.end() && it->toBool();
    }

//    // Add test suites
//    for (auto& i : tests->testSuites()) {
//        ICore::self()->testController()->addTestSuite(i.get());
//    }
}
