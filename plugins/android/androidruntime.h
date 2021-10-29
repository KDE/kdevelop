/*
    SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef ANDROIDRUNTIME_H
#define ANDROIDRUNTIME_H

#include <interfaces/iruntime.h>
#include <util/path.h>

class KJob;
class AndroidPreferencesSettings;

class AndroidRuntime : public KDevelop::IRuntime
{
    Q_OBJECT
public:
    AndroidRuntime();
    ~AndroidRuntime() override;

    QString name() const override { return QStringLiteral("Android"); }

    void setEnabled(bool enabled) override;

    void startProcess(KProcess *process) const override;
    void startProcess(QProcess *process) const override;
    KDevelop::Path pathInHost(const KDevelop::Path & runtimePath) const override { return runtimePath; }
    KDevelop::Path pathInRuntime(const KDevelop::Path & localPath) const override { return localPath; }
    QByteArray getenv(const QByteArray &varname) const override;
    QString findExecutable(const QString& executableName) const override;
    KDevelop::Path buildPath() const override { return {}; }

    static AndroidPreferencesSettings* s_settings;
};

#endif
