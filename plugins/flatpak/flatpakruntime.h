/*
   Copyright 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef FLATPAKRUNTIME_H
#define FLATPAKRUNTIME_H

#include <interfaces/iruntime.h>
#include <util/path.h>

class KJob;
class FlatpakPlugin;

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
    QByteArray getenv(const QByteArray &varname) const override;

    static KJob* createBuildDirectory(const KDevelop::Path &path, const KDevelop::Path &file, const QString &arch);

    KJob* rebuild();
    QList<KJob*> exportBundle(const QString &path) const;
    KJob* executeOnDevice(const QString &host, const QString &path) const;

    static QJsonObject config(const KDevelop::Path& path);
    KDevelop::Path buildPath() const override;

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
