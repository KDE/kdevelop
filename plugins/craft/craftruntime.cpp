// SPDX-FileCopyrightText: 2022 Gleb Popov <arrowd@FreeBSD.org>
// SPDX-License-Identifier: BSD-3-Clause

#include "craftruntime.h"
#include "debug_craft.h"

#include <QFileInfo>
#include <QStandardPaths>
#include <QProcess>
#include <KProcess>

using namespace KDevelop;

namespace {
auto craftSetupHelperRelativePath()
{
    return QLatin1String{"/craft/bin/CraftSetupHelper.py"};
}
}

CraftRuntime::CraftRuntime(const QString& craftRoot, const QString& pythonExecutable)
    : m_craftRoot(craftRoot)
    , m_pythonExecutable(pythonExecutable)
{
    Q_ASSERT(!pythonExecutable.isEmpty());

    m_watcher.addPath(craftRoot + craftSetupHelperRelativePath());

    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, [this](const QString& path) {
        if (QFileInfo::exists(path)) {
            refreshEnvCache();
            if (!m_watcher.files().contains(path)) {
                m_watcher.addPath(path);
            }
        }
    });
    refreshEnvCache();
}

QString CraftRuntime::name() const
{
    return QStringLiteral("Craft [%1]").arg(m_craftRoot);
}

QString CraftRuntime::findCraftRoot(Path startingPoint)
{
    // CraftRuntime doesn't handle remote directories, because it needs
    // to check file existence in the findCraftRoot() function
    if (startingPoint.isRemote())
        return QString();

    QString craftRoot;
    while (true) {
        bool craftSettingsIniExists = QFileInfo::exists(startingPoint.path() + QLatin1String("/etc/CraftSettings.ini"));
        bool craftSetupHelperExists = QFileInfo::exists(startingPoint.path() + craftSetupHelperRelativePath());
        if (craftSettingsIniExists && craftSetupHelperExists) {
            craftRoot = startingPoint.path();
            break;
        }

        if (!startingPoint.hasParent())
            break;
        startingPoint = startingPoint.parent();
    }

    return QFileInfo(craftRoot).canonicalFilePath();
}

QString CraftRuntime::findPython()
{
    // Craft requires Python 3.6+, not any "python3", but
    // - If the user set up Craft already, there is a high probability that
    //   "python3" is a correct one
    // - We are running only CraftSetupHelper.py, not the whole Craft, so
    //   the 3.6+ requirement might be not relevant for this case.
    // So just search for "python3" and hope for the best.
    return QStandardPaths::findExecutable(QStringLiteral("python3"));
}

void CraftRuntime::setEnabled(bool enabled)
{
    if (enabled)
        qCDebug(CRAFT) << "Enabling Craft runtime at" << m_craftRoot << "with" << m_pythonExecutable;
}

void CraftRuntime::refreshEnvCache()
{
    QProcess python;
    python.start(m_pythonExecutable,
                 QStringList{m_craftRoot + craftSetupHelperRelativePath(), QStringLiteral("--getenv")});
    python.waitForFinished(5000);

    if (python.error() != QProcess::UnknownError) {
        if (python.error() == QProcess::Timedout)
            qCWarning(CRAFT) << "CraftSetupHelper.py execution timed out";
        else
            qCWarning(CRAFT) << "CraftSetupHelper.py execution failed:" << python.error() << python.errorString();
        return;
    }

    if (python.exitCode()) {
        qCWarning(CRAFT) << "CraftSetupHelper.py execution failed with code" << python.exitCode();
        return;
    }

    m_envCache.clear();

    const QList<QByteArray> output = python.readAllStandardOutput().split('\n');
    for (const auto& line : output) {
        // line contains things like "VAR=VALUE"
        int equalsSignIndex = line.indexOf('=');
        if (equalsSignIndex == -1)
            continue;

        QByteArray varName = line.left(equalsSignIndex);
        QByteArray value = line.mid(equalsSignIndex + 1);
        m_envCache.emplace_back(varName, value);
    }
}

QByteArray CraftRuntime::getenv(const QByteArray& varname) const
{
    auto it = std::find_if(m_envCache.begin(), m_envCache.end(), [&varname](const EnvironmentVariable& envVar) {
        return envVar.name == varname;
    });

    return it != m_envCache.end() ? it->value : QByteArray();
}

QString CraftRuntime::findExecutable(const QString& executableName) const
{
    auto runtimePaths = QString::fromLocal8Bit(getenv(QByteArrayLiteral("PATH"))).split(QLatin1Char(':'));

    return QStandardPaths::findExecutable(executableName, runtimePaths);
}

Path CraftRuntime::pathInHost(const Path& runtimePath) const
{
    return runtimePath;
}

Path CraftRuntime::pathInRuntime(const Path& localPath) const
{
    return localPath;
}

void CraftRuntime::startProcess(KProcess* process) const
{
    QStringList program = process->program();
    QString executableInRuntime = findExecutable(program.constFirst());
    if (executableInRuntime != program.constFirst()) {
        program.first() = std::move(executableInRuntime);
        process->setProgram(program);
    }
    setEnvironmentVariables(process);
    process->start();
}

void CraftRuntime::startProcess(QProcess* process) const
{
    QString executableInRuntime = findExecutable(process->program());
    process->setProgram(executableInRuntime);
    setEnvironmentVariables(process);
    process->start();
}

void CraftRuntime::setEnvironmentVariables(QProcess* process) const
{
    auto env = process->processEnvironment();

    for (const auto& envVar : m_envCache) {
        env.insert(QString::fromLocal8Bit(envVar.name), QString::fromLocal8Bit(envVar.value));
    }

    process->setProcessEnvironment(env);
}

EnvironmentVariable::EnvironmentVariable(const QByteArray& name, const QByteArray& value)
    : name(name.trimmed())
    , value(value)
{
}
