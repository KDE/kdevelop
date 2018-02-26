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

#include <QFileInfo>
#include <QMessageBox>

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
            m_name = cfgGroup.readEntry(ExecutePlugin::executableEntry, cfg->name()).section('/', -1);
        }
    }
    setCapabilities(Killable);

    IExecutePlugin* iface = KDevelop::ICore::self()->pluginController()->pluginForExtension(QStringLiteral("org.kdevelop.IExecutePlugin"), QStringLiteral("kdevexecute"))->extension<IExecutePlugin>();
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
    NativeAppJob* job = qobject_cast<NativeAppJob*>(j);
    if (!job) {
        const QList<NativeAppJob*> jobs = j->findChildren<NativeAppJob*>();
        if (!jobs.isEmpty())
            job = jobs.first();
    }
    return job;
}

void NativeAppJob::start()
{
    // we kill any execution of the configuration
    auto currentJobs = ICore::self()->runController()->currentJobs();
    for (auto it = currentJobs.begin(); it != currentJobs.end();) {
        NativeAppJob* job = findNativeJob(*it);
        if (job && job != this && job->m_name == m_name) {
            QMessageBox::StandardButton button = QMessageBox::question(nullptr, i18n("Job already running"), i18n("'%1' is already being executed. Should we kill the previous instance?", m_name));
            if (button != QMessageBox::No && ICore::self()->runController()->currentJobs().contains(*it)) {
                (*it)->kill(EmitResult);
            }
            currentJobs = ICore::self()->runController()->currentJobs();
            it = currentJobs.begin();
        } else {
            ++it;
        }
    }

    OutputExecuteJob::start();
}
