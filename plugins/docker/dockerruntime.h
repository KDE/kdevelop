/*
    SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef DOCKERRUNTIME_H
#define DOCKERRUNTIME_H

#include <interfaces/iruntime.h>
#include <util/path.h>
#include <QHash>
#include <QByteArray>

class KJob;
class DockerPreferencesSettings;

class DockerRuntime : public KDevelop::IRuntime
{
    Q_OBJECT
public:
    explicit DockerRuntime(const QString& tag);
    ~DockerRuntime() override;

    /**
     * @returns the docker tagname as a text identifier
     */
    QString name() const override { return m_tag; }

    /**
     * if @p enabled
     * Mounts the docker image's file system into a subdirectory the user can read.
     * if not @p enabled, it unmounts the image file system
     *
     * See GraphDriver.Data.UpperDir value in docker image inspect imagename
     *
     * Both require root privileges for now
     */
    void setEnabled(bool enabled) override;

    /**
     * Call processes using "docker run..." passing on the proper environment and volumes
     *
     * Volumes will include source and build directories that need to be exposed
     * into the container.
     */
    void startProcess(KProcess *process) const override;
    void startProcess(QProcess *process) const override;

    /**
     * Translates @p runtimePath from within the image into the host
     *
     * Takes into account the mounted upperDir and the different volumes set up
     */
    KDevelop::Path pathInHost(const KDevelop::Path & runtimePath) const override;

    /**
     * Translates @p localPath into a path that can be accessed by the runtime
     */
    KDevelop::Path pathInRuntime(const KDevelop::Path & localPath) const override;

    QString findExecutable(const QString& executableName) const override;

    /**
     * @returns the environment variable with @p varname set by the recipe (usually the Dockerfile)
     */
    QByteArray getenv(const QByteArray & varname) const override;

    KDevelop::Path buildPath() const override { return {}; }

    static DockerPreferencesSettings* s_settings;

private:
    void inspectContainer();
    QStringList workingDirArgs(QProcess* process) const;

    const QString m_tag;
    QString m_container;
    QHash<QByteArray,QByteArray> m_envs;
    KDevelop::Path m_mergedDir;
    KDevelop::Path m_userMergedDir;
};

#endif
