/*
    SPDX-FileCopyrightText: 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "mesonintrospectjob.h"

#include "mesonconfig.h"
#include "mesonmanager.h"
#include "mesonoptions.h"
#include <debug.h>

#include <KLocalizedString>
#include <KProcess>

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtConcurrentRun>

using namespace Meson;
using namespace KDevelop;

MesonIntrospectJob::MesonIntrospectJob(KDevelop::IProject* project, QVector<MesonIntrospectJob::Type> types,
                                       MesonIntrospectJob::Mode mode, QObject* parent)
    : KJob(parent)
    , m_types(types)
    , m_mode(mode)
    , m_project(project)
{
    Q_ASSERT(project);

    if (mode == MESON_FILE) {
        // Since we are parsing the meson file in this mode, no build directory
        // is required and we have to fake a build directory
        m_buildDir.buildDir = project->path();
        auto* bsm = project->buildSystemManager();
        auto* manager = dynamic_cast<MesonManager*>(bsm);
        if (manager) {
            m_buildDir.mesonExecutable = manager->findMeson();
        }
    } else {
        m_buildDir = Meson::currentBuildDir(project);
    }

    m_projectPath = project->path();
    connect(&m_futureWatcher, &QFutureWatcher<QString>::finished, this, &MesonIntrospectJob::finished);
}

MesonIntrospectJob::MesonIntrospectJob(KDevelop::IProject* project, KDevelop::Path meson,
                                       QVector<MesonIntrospectJob::Type> types, QObject* parent)
    : KJob(parent)
    , m_types(types)
    , m_mode(MESON_FILE)
    , m_project(project)
{
    Q_ASSERT(project);

    // Since we are parsing the meson file in this mode, no build directory
    // is required and we have to fake a build directory
    m_projectPath = project->path();
    m_buildDir.buildDir = m_projectPath;
    m_buildDir.mesonExecutable = meson;
    connect(&m_futureWatcher, &QFutureWatcher<QString>::finished, this, &MesonIntrospectJob::finished);
}

MesonIntrospectJob::MesonIntrospectJob(KDevelop::IProject* project, Meson::BuildDir buildDir,
                                       QVector<MesonIntrospectJob::Type> types, MesonIntrospectJob::Mode mode,
                                       QObject* parent)
    : KJob(parent)
    , m_types(types)
    , m_mode(mode)
    , m_buildDir(buildDir)
    , m_project(project)
{
    Q_ASSERT(project);
    m_projectPath = project->path();
    connect(&m_futureWatcher, &QFutureWatcher<QString>::finished, this, &MesonIntrospectJob::finished);
}

QString MesonIntrospectJob::getTypeString(MesonIntrospectJob::Type type) const
{
    switch (type) {
    case BENCHMARKS:
        return QStringLiteral("benchmarks");
    case BUILDOPTIONS:
        return QStringLiteral("buildoptions");
    case BUILDSYSTEM_FILES:
        return QStringLiteral("buildsystem_files");
    case DEPENDENCIES:
        return QStringLiteral("dependencies");
    case INSTALLED:
        return QStringLiteral("installed");
    case PROJECTINFO:
        return QStringLiteral("projectinfo");
    case TARGETS:
        return QStringLiteral("targets");
    case TESTS:
        return QStringLiteral("tests");
    }

    return QStringLiteral("error");
}

QString MesonIntrospectJob::importJSONFile(const BuildDir& buildDir, MesonIntrospectJob::Type type, QJsonObject* out)
{
    QString typeStr = getTypeString(type);
    QString fileName = QStringLiteral("intro-") + typeStr + QStringLiteral(".json");
    QString infoDir = buildDir.buildDir.toLocalFile() + QStringLiteral("/") + QStringLiteral("meson-info");
    QFile introFile(infoDir + QStringLiteral("/") + fileName);

    if (!introFile.exists()) {
        return i18n("Introspection file '%1' does not exist", QFileInfo(introFile).canonicalFilePath());
    }

    if (!introFile.open(QFile::ReadOnly | QFile::Text)) {
        return i18n("Failed to open introspection file '%1'", QFileInfo(introFile).canonicalFilePath());
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(introFile.readAll(), &error);
    if (error.error) {
        return i18n("In %1:%2: %3", QFileInfo(introFile).canonicalFilePath(), error.offset, error.errorString());
    }

    if (doc.isArray()) {
        (*out)[typeStr] = doc.array();
    } else if (doc.isObject()) {
        (*out)[typeStr] = doc.object();
    } else {
        return i18n("The introspection file '%1' contains neither an array nor an object",
                    QFileInfo(introFile).canonicalFilePath());
    }

    return QString();
}

QString MesonIntrospectJob::importMesonAPI(const BuildDir& buildDir, MesonIntrospectJob::Type type, QJsonObject* out)
{
    QString typeStr = getTypeString(type);
    QString option = QStringLiteral("--") + typeStr;
    option.replace(QLatin1Char('_'), QLatin1Char('-'));

    KProcess proc;
    proc.setWorkingDirectory(m_projectPath.toLocalFile());
    proc.setOutputChannelMode(KProcess::SeparateChannels);
    proc.setProgram(buildDir.mesonExecutable.toLocalFile());
    proc << QStringLiteral("introspect") << option << QStringLiteral("meson.build");

    int ret = proc.execute();
    if (ret != 0) {
        return i18n("%1 returned %2", proc.program().join(QLatin1Char(' ')), ret);
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(proc.readAll(), &error);
    if (error.error) {
        return i18n("JSON parser error: %1", error.errorString());
    }

    if (doc.isArray()) {
        (*out)[typeStr] = doc.array();
    } else if (doc.isObject()) {
        (*out)[typeStr] = doc.object();
    } else {
        return i18n("The introspection output of '%1' contains neither an array nor an object",
                    proc.program().join(QLatin1Char(' ')));
    }

    return QString();
}

QString MesonIntrospectJob::import(BuildDir buildDir)
{
    QJsonObject rawData;

    // First load the complete JSON data
    for (auto i : m_types) {
        QString err;
        switch (m_mode) {
        case BUILD_DIR:
            err = importJSONFile(buildDir, i, &rawData);
            break;
        case MESON_FILE:
            err = importMesonAPI(buildDir, i, &rawData);
            break;
        }

        if (!err.isEmpty()) {
            qCWarning(KDEV_Meson) << "MINTRO: " << err;
            setError(true);
            setErrorText(err);
            return err;
        }
    }

    auto buildOptionsJSON = rawData[QStringLiteral("buildoptions")];
    if (buildOptionsJSON.isArray()) {
        m_res_options = std::make_shared<MesonOptions>(buildOptionsJSON.toArray());
        if (m_res_options) {
            qCDebug(KDEV_Meson) << "MINTRO: Imported " << m_res_options->options().size() << " buildoptions";
        } else {
            qCWarning(KDEV_Meson) << "MINTRO: Failed to parse buildoptions";
        }
    }

    auto projectInfoJSON = rawData[QStringLiteral("projectinfo")];
    if (projectInfoJSON.isObject()) {
        m_res_projectInfo = std::make_shared<MesonProjectInfo>(projectInfoJSON.toObject());
        if (!m_res_projectInfo) {
            qCWarning(KDEV_Meson) << "MINTRO: Failed to parse projectinfo";
        }
    }

    auto targetsJSON = rawData[QStringLiteral("targets")];
    if (targetsJSON.isArray()) {
        m_res_targets = std::make_shared<MesonTargets>(targetsJSON.toArray());
    }

    auto testsJSON = rawData[QStringLiteral("tests")];
    if (testsJSON.isArray()) {
        m_res_tests = std::make_shared<MesonTestSuites>(testsJSON.toArray(), m_project);
        if (m_res_tests) {
            qCDebug(KDEV_Meson) << "MINTRO: Imported " << m_res_tests->testSuites().size() << " test suites";
        } else {
            qCWarning(KDEV_Meson) << "MINTRO: Failed to parse tests";
        }
    }

    return QString();
}

void MesonIntrospectJob::start()
{
    qCDebug(KDEV_Meson) << "MINTRO: Starting meson introspection job";
    if (!m_buildDir.isValid()) {
        qCWarning(KDEV_Meson) << "The current build directory is invalid";
        setError(true);
        setErrorText(i18n("The current build directory is invalid"));
        emitResult();
        return;
    }

    auto future = QtConcurrent::run([this] {
        return import(m_buildDir);
    });
    m_futureWatcher.setFuture(future);
}

void MesonIntrospectJob::finished()
{
    qCDebug(KDEV_Meson) << "MINTRO: Meson introspection job finished";
    emitResult();
}

bool MesonIntrospectJob::doKill()
{
    if (m_futureWatcher.isRunning()) {
        m_futureWatcher.cancel();
    }
    return true;
}

MesonOptsPtr MesonIntrospectJob::buildOptions()
{
    return m_res_options;
}

MesonProjectInfoPtr MesonIntrospectJob::projectInfo()
{
    return m_res_projectInfo;
}

MesonTargetsPtr MesonIntrospectJob::targets()
{
    return m_res_targets;
}

MesonTestSuitesPtr MesonIntrospectJob::tests()
{
    return m_res_tests;
}

#include "moc_mesonintrospectjob.cpp"
