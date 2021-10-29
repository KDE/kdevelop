/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_NATIVEAPPJOB_H
#define KDEVPLATFORM_PLUGIN_NATIVEAPPJOB_H

#include <outputview/outputexecutejob.h>

#include <QMessageBox>

namespace KDevelop
{
class ILaunchConfiguration;
}

class NativeAppJob : public KDevelop::OutputExecuteJob
{
    Q_OBJECT

public:
    NativeAppJob( QObject* parent, KDevelop::ILaunchConfiguration* cfg );

    void start() override;

    // Actions for what to do when a job is already running
    static constexpr auto startAnother = QMessageBox::Yes;
    static constexpr auto killAllInstances = QMessageBox::No;
    static constexpr auto askIfRunning = QMessageBox::Cancel;

Q_SIGNALS:
    void killBeforeExecutingAgainChanged(int killBeforeExecutingAgainChanged);
private:
    QString m_name;
    int m_killBeforeExecutingAgain = startAnother;
};

#endif
