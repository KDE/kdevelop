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

#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>

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

#include "iexecuteplugin.h"
#include "debug.h"

using namespace KDevelop;

NativeAppJob::NativeAppJob(QObject* parent, KDevelop::ILaunchConfiguration* cfg)
    : KDevelop::OutputExecuteJob( parent )
    , m_cfgname(cfg->name())
{
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
        qWarning() << "Launch Configuration:" << cfg->name() << i18n("No environment profile specified, looks like a broken "
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
        qWarning() << "Launch Configuration:" << cfg->name() << "oops, problem" << errorText();
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
        QStringList args = KShell::splitArgs(iface->terminal(cfg));
        for (QStringList::iterator it = args.begin(); it != args.end(); ++it) {
            if (*it == QLatin1String("%exe")) {
                *it = KShell::quoteArg(executable.toLocalFile());
            } else if (*it == QLatin1String("%workdir")) {
                *it = KShell::quoteArg(wc.toLocalFile());
            }
        }
        args.append( arguments );
        *this << args;
    } else {
        *this << executable.toLocalFile();
        *this << arguments;
    }

    setJobName(cfg->name());
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
    foreach(KJob* j, ICore::self()->runController()->currentJobs()) {
        NativeAppJob* job = findNativeJob(j);
        if (job && job != this && job->m_cfgname == m_cfgname) {
            QMessageBox::StandardButton button = QMessageBox::question(nullptr, i18n("Job already running"), i18n("'%1' is already being executed. Should we kill the previous instance?", m_cfgname));
            if (button != QMessageBox::No)
                j->kill();
        }
    }

    OutputExecuteJob::start();
}
