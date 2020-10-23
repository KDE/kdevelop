/*  This file is part of KDevelop
    Copyright 2009 Andreas Pakulat <apaku@gmx.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
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
