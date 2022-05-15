// SPDX-FileCopyrightText: 2022 Gleb Popov <arrowd@FreeBSD.org>
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CRAFTRUNTIME_H
#define CRAFTRUNTIME_H

#include <vector>

#include <QString>
#include <QFileSystemWatcher>
#include <interfaces/iruntime.h>
#include <util/path.h>

class QProcess;

namespace KDevelop {
class IProject;
}

// An auxiliary structure to hold normalized name and value of an env var
struct EnvironmentVariable
{
    EnvironmentVariable(const QByteArray& name, const QByteArray& value);

    QByteArray name;
    QByteArray value;
};
Q_DECLARE_TYPEINFO(EnvironmentVariable, Q_MOVABLE_TYPE);

class CraftRuntime : public KDevelop::IRuntime
{
    Q_OBJECT
public:
    CraftRuntime(const QString& craftRoot, const QString& pythonExecutable);

    QString name() const override;
    void setEnabled(bool enabled) override;
    void startProcess(KProcess* process) const override;
    void startProcess(QProcess* process) const override;
    KDevelop::Path pathInHost(const KDevelop::Path& runtimePath) const override;
    KDevelop::Path pathInRuntime(const KDevelop::Path& localPath) const override;
    QString findExecutable(const QString& executableName) const override;
    QByteArray getenv(const QByteArray& varname) const override;

    KDevelop::Path buildPath() const override
    {
        return {};
    }
    QString craftRoot() const
    {
        return m_craftRoot;
    }

    static QString findCraftRoot(KDevelop::Path startingPoint);
    static QString findPython();

private:
    void setEnvironmentVariables(QProcess* process) const;
    void refreshEnvCache();

    const QString m_craftRoot;
    const QString m_pythonExecutable;
    QFileSystemWatcher m_watcher;
    std::vector<EnvironmentVariable> m_envCache;
};

#endif // CRAFTRUNTIME_H
