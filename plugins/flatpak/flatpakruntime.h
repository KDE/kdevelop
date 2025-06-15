/*
    SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef FLATPAKRUNTIME_H
#define FLATPAKRUNTIME_H

#include <interfaces/iruntime.h>
#include <util/path.h>

class KJob;
class FlatpakPlugin;

inline constexpr QLatin1String kdeFlatpakManifestFileName(".flatpak-manifest.json");

class FlatpakRuntime : public KDevelop::IRuntime
{
    Q_OBJECT
public:
    FlatpakRuntime(const KDevelop::Path &buildDirectory, const KDevelop::Path &file, const QString &arch);
    ~FlatpakRuntime() override;

    QString name() const override;

    void setEnabled(bool enabled) override;

    void startProcess(KProcess *process) const override;
    void startProcess(QProcess *process) const override;
    KDevelop::Path pathInHost(const KDevelop::Path & runtimePath) const override;
    KDevelop::Path pathInRuntime(const KDevelop::Path & localPath) const override;
    QString findExecutable(const QString& executableName) const override;
    QByteArray getenv(const QByteArray &varname) const override;

    static KJob* createBuildDirectory(const KDevelop::Path &path, const KDevelop::Path &file, const QString &arch);

    KJob* rebuild();

    struct ExportBundle
    {
        QList<KJob*> jobs;
        QString applicationId;
    };
    ExportBundle exportBundle(const QString& path) const;

    KJob* executeOnDevice(const QString &host, const QString &path) const;

    static QJsonObject config(const KDevelop::Path& path);
    KDevelop::Path buildPath() const override;

    KDevelop::Path file() const
    {
        return m_file;
    }
    QString arch() const
    {
        return m_arch;
    }

private:
    void refreshJson();
    QJsonObject config() const;

    const KDevelop::Path m_file;
    const KDevelop::Path m_buildDirectory;
    const QString m_arch;
    QStringList m_finishArgs;
    KDevelop::Path m_sdkPath;
};

#endif
