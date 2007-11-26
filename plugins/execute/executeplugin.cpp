/*
  * This file is part of KDevelop
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "executeplugin.h"

#include <klocale.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kdebug.h>

#include <environmentgrouplist.h>

using namespace KDevelop;

K_PLUGIN_FACTORY(KDevExecuteFactory, registerPlugin<ExecutePlugin>(); )
K_EXPORT_PLUGIN(KDevExecuteFactory("kdevexecute"))

ExecutePlugin::ExecutePlugin(QObject *parent, const QVariantList&)
    : KDevelop::IPlugin(KDevExecuteFactory::componentData(), parent)
{
  KDEV_USE_EXTENSION_INTERFACE( KDevelop::IRunProvider )
}

ExecutePlugin::~ExecutePlugin()
{
}

void ExecutePlugin::unload()
{
}

QStringList ExecutePlugin::instrumentorsProvided() const
{
    return QStringList() << "default";
}

bool ExecutePlugin::execute(const IRun & run, int serial)
{
    Q_ASSERT(instrumentorsProvided().contains(run.instrumentor()));

    QProcess* process = new QProcess(this);
    connect(process, SIGNAL(readyReadStandardOutput()), SLOT(readyReadStandardOutput()));
    connect(process, SIGNAL(readyReadStandardError()), SLOT(readyReadStandardError()));
    connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(processFinished(int, QProcess::ExitStatus)));

    m_runners.insert(serial, process);

    KDevelop::EnvironmentGroupList l(KGlobal::config());
    process->setProperty("serial", serial);
    process->setEnvironment(l.createEnvironment(run.environmentKey(), process->systemEnvironment()));
    process->setWorkingDirectory(run.workingDirectory().path());
    process->start(run.executable().path(), run.arguments());

    return true;
}

void ExecutePlugin::abort(int serial)
{
    if (m_runners.contains(serial))
        delete m_runners.take(serial);
}

void ExecutePlugin::readyReadStandardError()
{
    QProcess* process = qobject_cast<QProcess*>(sender());
    if (!process)
        return;

    readFrom(process, QProcess::StandardError);
}

void ExecutePlugin::readyReadStandardOutput()
{
    QProcess* process = qobject_cast<QProcess*>(sender());
    if (!process)
        return;

    readFrom(process, QProcess::StandardOutput);
}

void ExecutePlugin::readFrom(QProcess * process, QProcess::ProcessChannel channel)
{
    process->setReadChannel(channel);
    int serial = process->property("serial").toInt();

    while (process->canReadLine()) {
        QByteArray line = process->readLine();
        QString string = QString::fromLocal8Bit(line, line.length() - 2);
        emit output(serial, string, channel == QProcess::StandardOutput ? IRunProvider::StandardOutput : IRunProvider::StandardError);
    }
}

void ExecutePlugin::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess* process = qobject_cast<QProcess*>(sender());
    if (!process)
        return;

    int serial = process->property("serial").toInt();

    if (exitCode == 0 && exitStatus == QProcess::NormalExit)
        emit output(serial, i18n("*** Exited normally ***"), IRunProvider::RunProvider);
    else
        if (exitStatus == QProcess::NormalExit)
            emit output(serial, i18n("*** Exited with return code: %1 ***", QString::number(exitCode)), IRunProvider::RunProvider);
        else
            emit output(serial, i18n("*** Crashed with return code: %1 ***", QString::number(exitCode)), IRunProvider::RunProvider);

    emit finished(serial);
}

#include "executeplugin.moc"

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
