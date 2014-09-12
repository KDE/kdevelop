/************************************************************************
 * KDevelop4 Custom Buildsystem Support                                 *
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 or version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful, but  *
 * WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     *
 * General Public License for more details.                             *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program; if not, see <http://www.gnu.org/licenses/>. *
 ************************************************************************/

#include "custombuildjob.h"

#include <KLocalizedString>
#include <KProcess>
#include <KConfigGroup>
#include <KGlobal>
#include <KShell>

#include <outputview/outputmodel.h>
#include <outputview/outputdelegate.h>
#include <util/processlinemaker.h>
#include <util/environmentgrouplist.h>
#include <util/commandexecutor.h>
#include <project/projectmodel.h>

#include "custombuildsystemplugin.h"
#include "configconstants.h"

CustomBuildJob::CustomBuildJob( CustomBuildSystem* plugin, KDevelop::ProjectBaseItem* item, CustomBuildSystemTool::ActionType t )
    : OutputJob( plugin )
    , type( t )
    , exec(nullptr)
    , killed( false )
    , enabled( false )
{
    setCapabilities( Killable );
    QString subgrpname;
    QString title;
    switch( type ) {
        case CustomBuildSystemTool::Build:
            title = i18n( "Building:" );
            subgrpname = QString( "%1Build" ).arg( ConfigConstants::toolGroupPrefix );
            break;
        case CustomBuildSystemTool::Clean:
            title = i18n( "Cleaning:" );
            subgrpname = QString( "%1Clean" ).arg( ConfigConstants::toolGroupPrefix );
            break;
        case CustomBuildSystemTool::Install:
            title = i18n( "Installing:" );
            subgrpname = QString( "%1Install" ).arg( ConfigConstants::toolGroupPrefix );
            break;
        case CustomBuildSystemTool::Configure:
            title = i18n( "Configuring:" );
            subgrpname = QString( "%1Configure" ).arg( ConfigConstants::toolGroupPrefix );
            break;
        case CustomBuildSystemTool::Prune:
            title = i18n( "Pruning:" );
            subgrpname = QString( "%1Prune" ).arg( ConfigConstants::toolGroupPrefix );
            break;
        case CustomBuildSystemTool::Undefined:
            return;
    }
    setTitle( QString("%1 %2").arg( cmd ).arg( item->text() ) );
    setObjectName( QString("%1 %2").arg( cmd ).arg( item->text() ) );
    builddir = plugin->buildDirectory( item ).toLocalFile();
    KConfigGroup g = plugin->configuration( item->project() );
    if(g.isValid()) {
        KConfigGroup grp = g.group( subgrpname );
        enabled = grp.readEntry( ConfigConstants::toolEnabled, false );
        cmd = grp.readEntry( ConfigConstants::toolExecutable, QUrl() ).toLocalFile();
        environment = grp.readEntry( ConfigConstants::toolEnvironment, "" );
        arguments = grp.readEntry( ConfigConstants::toolArguments, "" );
    }
    setDelegate( new KDevelop::OutputDelegate );
}

void CustomBuildJob::start()
{
    if( type == CustomBuildSystemTool::Undefined ) {
        setError( UndefinedBuildType );
        setErrorText( i18n( "Undefined Build type" ) );
        emitResult();
    } else if( cmd.isEmpty() ) {
        setError( NoCommand );
        setErrorText( i18n( "No command given" ) );
        emitResult();
    } else if( !enabled ) {
        setError( ToolDisabled );
        setErrorText( i18n( "This command is disabled" ) );
        emitResult();
    } else {
        // prepend the command name to the argument string
        // so that splitArgs works correctly
        const QString allargv = KShell::quoteArg(cmd) + " " + arguments;

        KShell::Errors err;
        QStringList strargs = KShell::splitArgs( allargv, KShell::AbortOnMeta, &err );
        if( err != KShell::NoError ) {
            setError( WrongArgs );
            setErrorText( i18n( "The given arguments would need a real shell, this is not supported currently." ) );
            emitResult();
        }
        // and remove the command name back out of the split argument list
        Q_ASSERT(!strargs.isEmpty());
        strargs.removeFirst();

        setStandardToolView( KDevelop::IOutputView::BuildView );
        setBehaviours( KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll );
        KDevelop::OutputModel* model = new KDevelop::OutputModel( builddir );
        model->setFilteringStrategy( KDevelop::OutputModel::CompilerFilter );
        setModel( model );

        startOutput();

        exec = new KDevelop::CommandExecutor( cmd, this );

        exec->setArguments( strargs );
        exec->setEnvironment( KDevelop::EnvironmentGroupList( KSharedConfig::openConfig() ).createEnvironment( environment, KProcess::systemEnvironment() ) );
        exec->setWorkingDirectory( builddir );

        
        connect( exec, SIGNAL(completed(int)), SLOT(procFinished(int)) );
        connect( exec, SIGNAL(failed( QProcess::ProcessError )), SLOT(procError( QProcess::ProcessError )) );

        connect( exec, SIGNAL(receivedStandardError(QStringList)), model, SLOT(appendLines(QStringList)) );
        connect( exec, SIGNAL(receivedStandardOutput(QStringList)), model, SLOT(appendLines(QStringList)) );

        model->appendLine( QString("%1>%2 %3").arg( builddir ).arg( cmd ).arg( arguments ) );
        exec->start();
    }
}

bool CustomBuildJob::doKill()
{
    killed = true;
    exec->kill();
    return true;
}

void CustomBuildJob::procError( QProcess::ProcessError err )
{
    if( !killed ) {
        if( err == QProcess::FailedToStart ) {
            setError( FailedToStart );
            setErrorText( i18n( "Failed to start command." ) );
        } else if( err == QProcess::Crashed ) {
            setError( Crashed );
            setErrorText( i18n( "Command crashed." ) );
        } else {
            setError( UnknownExecError );
            setErrorText( i18n( "Unknown error executing command." ) );
        }
    }
    emitResult();
}

KDevelop::OutputModel* CustomBuildJob::model()
{
    return qobject_cast<KDevelop::OutputModel*>( OutputJob::model() );
}

void CustomBuildJob::procFinished(int code)
{
    //TODO: Make this configurable when the first report comes in from a tool
    //      where non-zero does not indicate error status
    if( code != 0 ) {
        setError( FailedShownError );
        model()->appendLine( i18n( "*** Failed ***" ) );
    } else {
        model()->appendLine( i18n( "*** Finished ***" ) );
    }
    emitResult();
}

