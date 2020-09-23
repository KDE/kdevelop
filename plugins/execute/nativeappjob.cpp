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

#include "nativeappjob.h"

#include <QAbstractButton>
#include <QFileInfo>
#include <QMessageBox>
#include <QPointer>
#include <QCheckBox>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KShell>
#include <KSharedConfig>

#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/iruncontroller.h>
#include <outputview/outputmodel.h>
#include <util/environmentprofilelist.h>

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <project/projectmodel.h>

#include "executeplugin.h"
#include "debug.h"

using namespace KDevelop;

NativeAppJob::NativeAppJob(QObject* parent, KDevelop::ILaunchConfiguration* cfg)
    : KDevelop::OutputExecuteJob( parent )
    , m_name(cfg->name())
{
    {
        auto cfgGroup = cfg->config();
        if (cfgGroup.readEntry(ExecutePlugin::isExecutableEntry, false)) {
            m_name = cfgGroup.readEntry(ExecutePlugin::executableEntry, cfg->name()).section(QLatin1Char('/'), -1);
        }
        m_killBeforeExecutingAgain = cfgGroup.readEntry<int>(ExecutePlugin::killBeforeExecutingAgain, QMessageBox::Cancel);
    }
    setCapabilities(Killable);

    auto* iface = KDevelop::ICore::self()->pluginController()->pluginForExtension(QStringLiteral("org.kdevelop.IExecutePlugin"), QStringLiteral("kdevexecute"))->extension<IExecutePlugin>();
    Q_ASSERT(iface);

    const KDevelop::EnvironmentProfileList environmentProfiles(KSharedConfig::openConfig());
    QString envProfileName = iface->environmentProfileName(cfg);

    QString err;
    QUrl executable = iface->executable( cfg, err );

    if( !err.isEmpty() )
    {
        setError( -1 );
        setErrorText( err );
        return;
    }

    if (envProfileName.isEmpty()) {
        qCWarning(PLUGIN_EXECUTE) << "Launch Configuration:" << cfg->name() << i18n("No environment profile specified, looks like a broken "
                       "configuration, please check run configuration '%1'. "
                       "Using default environment profile.", cfg->name() );
        envProfileName = environmentProfiles.defaultProfileName();
    }
    setEnvironmentProfile(envProfileName);

    QStringList arguments = iface->arguments( cfg, err );
    if( !err.isEmpty() )
    {
        setError( -2 );
        setErrorText( err );
    }

    if( error() != 0 )
    {
        qCWarning(PLUGIN_EXECUTE) << "Launch Configuration:" << cfg->name() << "oops, problem" << errorText();
        return;
    }

    setStandardToolView(KDevelop::IOutputView::RunView);
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll);
    setFilteringStrategy(OutputModel::NativeAppErrorFilter);
    setProperties(DisplayStdout | DisplayStderr);

    // Now setup the process parameters

    QUrl wc = iface->workingDirectory( cfg );
    if( !wc.isValid() || wc.isEmpty() ) {
        wc = QUrl::fromLocalFile( QFileInfo( executable.toLocalFile() ).absolutePath() );
    }
    setWorkingDirectory( wc );

    qCDebug(PLUGIN_EXECUTE) << "setting app:" << executable << arguments;

    if (iface->useTerminal(cfg)) {
        QString terminalCommand = iface->terminal(cfg);
        terminalCommand.replace(QLatin1String("%exe"), KShell::quoteArg( executable.toLocalFile()) );
        terminalCommand.replace(QLatin1String("%workdir"), KShell::quoteArg( wc.toLocalFile()) );
        QStringList args = KShell::splitArgs(terminalCommand);
        args.append( arguments );
        *this << args;
    } else {
        *this << executable.toLocalFile();
        *this << arguments;
    }

    setJobName(m_name);
}

NativeAppJob* findNativeJob(KJob* j)
{
    auto* job = qobject_cast<NativeAppJob*>(j);
    if (!job) {
        const QList<NativeAppJob*> jobs = j->findChildren<NativeAppJob*>();
        if (!jobs.isEmpty())
            job = jobs.first();
    }
    return job;
}

void NativeAppJob::start()
{
    QVector<QPointer<NativeAppJob> > currentJobs;
    // collect running instances of the same type
    const auto& allCurrentJobs = ICore::self()->runController()->currentJobs();
    for (auto j : allCurrentJobs) {
        NativeAppJob* njob = findNativeJob(j);
        if (njob && njob != this && njob->m_name == m_name)
            currentJobs << njob;
    }

    if (!currentJobs.isEmpty()) {
        int killAllInstances = m_killBeforeExecutingAgain;
        if (killAllInstances == QMessageBox::Cancel) {
            QMessageBox msgBox(QMessageBox::Question,
                        i18nc("@title:window", "Job Already Running"),
                        i18n("'%1' is already being executed.", m_name),
                        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            msgBox.button(QMessageBox::No)->setText(i18nc("@action:button", "Kill All Instances"));
            msgBox.button(QMessageBox::Yes)->setText(i18nc("@action:button", "Start Another"));
            msgBox.setDefaultButton(QMessageBox::Cancel);

            QCheckBox* remember = new QCheckBox(i18nc("@option:check", "Remember choice"));
            msgBox.setCheckBox(remember);

            killAllInstances = msgBox.exec();
            if (remember->isChecked() && killAllInstances != QMessageBox::Cancel) {
                Q_EMIT killBeforeExecutingAgainChanged(killAllInstances);
            }
        }

        switch (killAllInstances) {
            case QMessageBox::Yes: // simply start another job
                break;
            case QMessageBox::No: // kill the running instance
                for (auto & job : currentJobs) {
                    if (job)
                        job->kill(EmitResult);
                }
                break;
            default: // cancel starting a new job
                kill(EmitResult);
                return;
        }
    }

    OutputExecuteJob::start();
}
