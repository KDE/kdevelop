/* This file is part of KDevelop
   Copyright 2017 Anton Anikin <anton.anikin@htower.ru>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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
    void showErrorMessage(const QString& message, int timeout = 0) override;
    void showMessage(KDevelop::IStatus*, const QString& message, int timeout = 0) override;
    void showProgress(KDevelop::IStatus*, int minimum, int maximum, int value) override;

protected:
    void setup();
    void postProcessStdout(const QStringList& lines) override;

    long int m_pid;
    QString m_analyzedExecutable;
    QString m_resultsFile;
};

}
