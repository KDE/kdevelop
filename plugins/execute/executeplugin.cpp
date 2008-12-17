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

#include <QApplication>

#include <klocale.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kdebug.h>
#include <kjob.h>
#include <kmessagebox.h>
#include <kaboutdata.h>

#include <util/environmentgrouplist.h>

using namespace KDevelop;

K_PLUGIN_FACTORY(KDevExecuteFactory, registerPlugin<ExecutePlugin>(); )
K_EXPORT_PLUGIN(KDevExecuteFactory(KAboutData("kdevexecute", "kdevexecute", ki18n("Execute support"), "0.1", ki18n("Allows running of native apps"), KAboutData::License_GPL)))

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
    return QStringList() << "default" << "konsole";
}

QString ExecutePlugin::translatedInstrumentor(const QString& instrumentor) const
{
    if (instrumentor == "default")
        return i18n("Run");

    if (instrumentor == "konsole")
        return i18n("Run in external konsole");

    return i18n("Unsupported instrumentor");
}

bool ExecutePlugin::execute(const IRun & run, KJob* job)
{
    Q_ASSERT(instrumentorsProvided().contains(run.instrumentor()));

    QProcess* process = new QProcess(this);
    connect(process, SIGNAL(readyReadStandardOutput()), SLOT(readyReadStandardOutput()));
    connect(process, SIGNAL(readyReadStandardError()), SLOT(readyReadStandardError()));
    connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(processFinished(int, QProcess::ExitStatus)));
    connect(process, SIGNAL(error(QProcess::ProcessError)), SLOT(error(QProcess::ProcessError)));

    m_runners.insert(job, process);

    KDevelop::EnvironmentGroupList l(KGlobal::config());
    process->setProperty("job", QVariant::fromValue(static_cast<void*>(job)));
    process->setEnvironment(l.createEnvironment(run.environmentKey(), process->systemEnvironment()));
    process->setWorkingDirectory(run.workingDirectory().path());

    process->setProperty("executable", run.executable().path());

    QString executable = run.executable().path();
    QStringList args;

    if (!run.runAsUser().isEmpty()) {
        args << "-u" << run.runAsUser();
        args << "-c" << executable;
        executable = "kdesudo";
    }

    if (run.instrumentor() == "konsole") {
        // Don't fork, so we can still kill it via our job system
        //args << "--nofork";
        // Provide the executable to run
        args << "-e" << executable;

        executable = "konsole";
    }

    args << run.arguments();

    process->start(executable, args);

    kDebug() << "Started process" << executable << "with arguments" << args;

    return true;
}

void ExecutePlugin::abort(KJob* job)
{
    if (m_runners.contains(job)) {
        QProcess* process = m_runners.take(job);
        process->close();
        delete process;
    }
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
    KJob* job = static_cast<KJob*>(qvariant_cast<void*>(process->property("job")));

    while (process->canReadLine()) {
        QByteArray line = process->readLine() + '\n';
        QString string = QString::fromLocal8Bit(line, line.length() - 2);
        emit output(job, string, channel == QProcess::StandardOutput ? IRunProvider::StandardOutput : IRunProvider::StandardError);
    }
}

void ExecutePlugin::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess* process = qobject_cast<QProcess*>(sender());
    if (!process)
        return;

    KJob* job = static_cast<KJob*>(qvariant_cast<void*>(process->property("job")));

    if (exitCode == 0 && exitStatus == QProcess::NormalExit)
        emit output(job, i18n("*** Exited normally ***"), IRunProvider::RunProvider);
    else
        if (exitStatus == QProcess::NormalExit)
            emit output(job, i18n("*** Exited with return code: %1 ***", QString::number(exitCode)), IRunProvider::RunProvider);
        else
            if (job->error() == KJob::KilledJobError)
                emit output(job, i18n("*** Process aborted ***"), IRunProvider::RunProvider);
            else
                emit output(job, i18n("*** Crashed with return code: %1 ***", QString::number(exitCode)), IRunProvider::RunProvider);

    emit finished(job);
}

void ExecutePlugin::error(QProcess::ProcessError error)
{
    const QProcess* process = qobject_cast<const QProcess*>(sender());
    Q_ASSERT(process);

    if( error == QProcess::FailedToStart )
    {
        KMessageBox::information(
            qApp->activeWindow(),
            i18n("<b>Could not start program.</b>"
                 "<p>Could not run '%1'. "
                 "Make sure that the path name is specified correctly.",
                 process->property("executable").toString()),
            i18n("Could not start program"));
    }

    KJob* job = static_cast<KJob*>(qvariant_cast<void*>(process->property("job")));
    Q_ASSERT(job);

    emit finished(job);
}

#include "executeplugin.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on; auto-insert-doxygen on
