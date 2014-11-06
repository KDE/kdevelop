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
#include <QDebug>

#include <kprocess.h>
#include <KLocalizedString>
#include <kmessagebox.h>
#include <kconfiggroup.h>
#include <kparts/mainwindow.h>

#include <interfaces/ilaunchconfiguration.h>
#include <outputview/outputmodel.h>
#include <outputview/outputdelegate.h>
#include <util/processlinemaker.h>
#include <util/environmentgrouplist.h>

#include <kshell.h>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iplugincontroller.h>
#include <project/projectmodel.h>

#include "iexecuteplugin.h"
#include "debug.h"
#include <KConfigCore/ksharedconfig.h>

using namespace KDevelop;

NativeAppJob::NativeAppJob(QObject* parent, KDevelop::ILaunchConfiguration* cfg)
    : KDevelop::OutputJob( parent ), proc(0)
{
    setCapabilities(Killable);

    IExecutePlugin* iface = KDevelop::ICore::self()->pluginController()->pluginForExtension("org.kdevelop.IExecutePlugin", "kdevexecute")->extension<IExecutePlugin>();
    Q_ASSERT(iface);

    KDevelop::EnvironmentGroupList l(KSharedConfig::openConfig());
    QString envgrp = iface->environmentGroup(cfg);

    QString err;
    QUrl executable = iface->executable( cfg, err );

    if( !err.isEmpty() )
    {
        setError( -1 );
        setErrorText( err );
        return;
    }

    if( envgrp.isEmpty() )
    {
        qWarning() << "Launch Configuration:" << cfg->name() << i18n("No environment group specified, looks like a broken "
                       "configuration, please check run configuration '%1'. "
                       "Using default environment group.", cfg->name() );
        envgrp = l.defaultGroup();
    }

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

    proc = new KProcess( this );

    lineMaker = new KDevelop::ProcessLineMaker( proc, this );

    setStandardToolView(KDevelop::IOutputView::RunView);
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll);
    OutputModel* m = new OutputModel;
    m->setFilteringStrategy(OutputModel::NativeAppErrorFilter);
    setModel(m);
    setDelegate( new KDevelop::OutputDelegate );

    connect( lineMaker, &ProcessLineMaker::receivedStdoutLines, model(), &OutputModel::appendLines );
    connect( proc, static_cast<void(KProcess::*)(QProcess::ProcessError)>(&KProcess::error), this, &NativeAppJob::processError );
    connect( proc, static_cast<void(KProcess::*)(int,QProcess::ExitStatus)>(&KProcess::finished), this, &NativeAppJob::processFinished );

    // Now setup the process parameters

    proc->setEnvironment( l.createEnvironment( envgrp, proc->systemEnvironment()) );
    QUrl wc = iface->workingDirectory( cfg );
    if( !wc.isValid() || wc.isEmpty() )
    {
        wc = QUrl::fromLocalFile( QFileInfo( executable.toLocalFile() ).absolutePath() );
    }
    proc->setWorkingDirectory( wc.toLocalFile() );
    proc->setProperty( "executable", executable );

    qCDebug(PLUGIN_EXECUTE) << "setting app:" << executable << arguments;

    proc->setOutputChannelMode(KProcess::MergedChannels);

    if (iface->useTerminal(cfg)) {
        QStringList args = KShell::splitArgs(iface->terminal(cfg));
        for (QStringList::iterator it = args.begin(); it != args.end(); ++it) {
            if (*it == "%exe") {
                *it = KShell::quoteArg(executable.toLocalFile());
            } else if (*it == "%workdir") {
                *it = KShell::quoteArg(wc.toLocalFile());
            }
        }
        args.append( arguments );
        proc->setProgram( args );
    } else {
        proc->setProgram( executable.toLocalFile(), arguments );
    }

    setObjectName(cfg->name());
}


void NativeAppJob::start()
{
    qCDebug(PLUGIN_EXECUTE) << "launching?" << proc;
    if( proc )
    {
        startOutput();
        appendLine( i18n("Starting: %1", proc->program().join(" ") ) );
        proc->start();
    } else
    {
        qWarning() << "No process, something went wrong when creating the job";
        // No process means we've returned early on from the constructor, some bad error happened
        emitResult();
    }
}

bool NativeAppJob::doKill()
{
    if( proc ) {
        proc->kill();
        appendLine( i18n( "*** Killed Application ***" ) );
    }
    return true;
}

void NativeAppJob::processFinished( int exitCode , QProcess::ExitStatus status )
{
    if (!model()) {
        outputDone();
        return;
    }

    connect(model(), &OutputModel::allDone, this, &NativeAppJob::outputDone);
    lineMaker->flushBuffers();

    if (exitCode == 0 && status == QProcess::NormalExit) {
        appendLine( i18n("*** Exited normally ***") );
    } else if (status == QProcess::NormalExit) {
        appendLine( i18n("*** Exited with return code: %1 ***", QString::number(exitCode)) );
        setError(OutputJob::FailedShownError);
    } else if (error() == KJob::KilledJobError) {
        appendLine( i18n("*** Process aborted ***") );
        setError(KJob::KilledJobError);
    } else {
        appendLine( i18n("*** Crashed with return code: %1 ***", QString::number(exitCode)) );
        setError(OutputJob::FailedShownError);
    }

    model()->ensureAllDone();
}

void NativeAppJob::outputDone()
{
    emitResult();
}

void NativeAppJob::processError( QProcess::ProcessError error )
{
    if( error == QProcess::FailedToStart )
    {
        setError( FailedShownError );
        QString errmsg =  i18n("*** Could not start program '%1'. Make sure that the "
                           "path is specified correctly ***", proc->program().join(" ") );
        appendLine( errmsg );
        setErrorText( errmsg );
        emitResult();
    }
    qCDebug(PLUGIN_EXECUTE) << "Process error";
}

void NativeAppJob::appendLine(const QString& l)
{
    if (KDevelop::OutputModel* m = model()) {
        m->appendLine(l);
    }
}

KDevelop::OutputModel* NativeAppJob::model()
{
    return dynamic_cast<KDevelop::OutputModel*>( OutputJob::model() );
}


