/*
    SPDX-FileCopyrightText: 2017 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <interfaces/istatus.h>
#include <outputview/outputexecutejob.h>

class IExecutePlugin;
namespace KDevelop { class ILaunchConfiguration; }

namespace Heaptrack
{

class Job : public KDevelop::OutputExecuteJob, public KDevelop::IStatus
{
    Q_OBJECT
    Q_INTERFACES(KDevelop::IStatus)

public:
    Job(KDevelop::ILaunchConfiguration* launchConfig, IExecutePlugin* executePlugin);
    explicit Job(long int pid);
    ~Job() override;

    void start() override;

    QString statusName() const override;
    QString resultsFile() const;

Q_SIGNALS:
    void clearMessage(KDevelop::IStatus*) override;
    void hideProgress(KDevelop::IStatus*) override;
    void showErrorMessage(const QString& message, int timeout) override;
    void showMessage(KDevelop::IStatus*, const QString& message, int timeout = 0) override;
    void showProgress(KDevelop::IStatus*, int minimum, int maximum, int value) override;

private:
    void setup(const QString& targetName);
    void postProcessStdout(const QStringList& lines) override;

    QString m_resultsFile;
};

}
