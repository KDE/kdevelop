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

#include <kprocess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kparts/mainwindow.h>

#include <interfaces/ilaunchconfiguration.h>
#include <outputview/outputmodel.h>
#include <util/processlinemaker.h>
#include <util/environmentgrouplist.h>

#include "executepluginconstants.h"
#include <kshell.h>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iprojectcontroller.h>
#include <project/projectmodel.h>

NativeAppJob::NativeAppJob(QObject* parent, KDevelop::ILaunchConfiguration* cfg) 
    : KDevelop::OutputJob( parent ), proc(0)
{
    kDebug() << "creating native app job";
    setCapabilities(Killable);
    
    KConfigGroup grp = cfg->config();
    KDevelop::EnvironmentGroupList l(KGlobal::config());

    QString executable = ExecutePlugin::executableFromConfig( grp );
    QString envgrp = grp.readEntry( ExecutePlugin::environmentGroupEntry, "" );
    
    if( executable.isEmpty() )
    {
        setError(-1);
        setErrorText( i18n("No executable specified") );
        kWarning() << "no executable set";
    } else
    {
        KShell::Errors err;
        if( KShell::splitArgs( executable, KShell::TildeExpand | KShell::AbortOnMeta, &err ).isEmpty() || err != KShell::NoError )
        {
            
            setError( -1 );
            if( err == KShell::BadQuoting ) 
            {
                setErrorText( i18n("There is a quoting error in the executable "
                                   "for the launch configuration '%1'. "
                                   "Aborting start.", cfg->name() ) );
            } else 
            {   
                setErrorText( i18n("A shell meta character was included in the "
                                   "executable for the launch configuration '%1', "
                                   "this is not supported currently. Aborting start.", cfg->name() ) );
            }
            kWarning() << "executable has meta characters";
        }
    }
    
    if( envgrp.isEmpty() )
    {
        kWarning() << i18n("No environment group specified, looks like a broken "
                       "configuration, please check run configuration '%1'. "
                       "Using default environment group.", cfg->name() );
        envgrp = l.defaultGroup();
    }
    
    KShell::Errors err;
    QStringList arguments = KShell::splitArgs( grp.readEntry( ExecutePlugin::argumentsEntry, "" ), KShell::TildeExpand | KShell::AbortOnMeta, &err );
    if( err != KShell::NoError )
    {
        
        setError( -1 );
        if( err == KShell::BadQuoting ) 
        {
            setErrorText( i18n("There is a quoting error in the arguments for "
                               "the launch configuration '%1'. Aborting start.", cfg->name() ) );
        } else 
        {   
            setErrorText( i18n("A shell meta character was included in the "
                               "arguments for the launch configuration '%1', "
                               "this is not supported currently. Aborting start.", cfg->name() ) );
        }
        kDebug() << "arguments have meta characters";
    }
    
    if( error() != 0 )
    {
        kDebug() << "oops, problem" << errorText();
        return;
    }
    
    proc = new KProcess( this );
    
    lineMaker = new KDevelop::ProcessLineMaker( proc, this );
    
    setStandardToolView(KDevelop::IOutputView::RunView);
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll);
    setModel( new KDevelop::OutputModel(), KDevelop::IOutputView::TakeOwnership );
    
    connect( lineMaker, SIGNAL(receivedStdoutLines(const QStringList&)), model(), SLOT(appendLines(QStringList)) );
    connect( proc, SIGNAL(error(QProcess::ProcessError)), SLOT(processError(QProcess::ProcessError)) );
    connect( proc, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(processFinished(int,QProcess::ExitStatus)) );

    // Now setup the process parameters
    
    proc->setEnvironment( l.createEnvironment( envgrp, proc->systemEnvironment()) );
    proc->setWorkingDirectory( grp.readEntry(ExecutePlugin::workingDirEntry, KUrl( QFileInfo( executable ).absolutePath() ) ).toLocalFile() );
    proc->setProperty( "executable", executable );
    
    kDebug() << "setting app:" << executable << arguments;
    
    proc->setOutputChannelMode(KProcess::MergedChannels);
    
    proc->setProperty("executable", executable);
    proc->setProgram( executable, arguments );
    
    setTitle(cfg->name());
}


void NativeAppJob::start() 
{
    kDebug() << "launching?" << proc;
    if( proc )
    {
        startOutput();
        model()->appendLine( i18n("Starting: %1", proc->property("executable").toString() ) );
        proc->start();
    } else
    {
        // No process means we've returned early on from the constructor, some bad error happened
        emitResult();
    }
}

bool NativeAppJob::doKill()
{
    if( proc ) {
        proc->kill();
        model()->appendLine( i18n( "*** Killed Application ***" ) );
    }
    return true;
}


void NativeAppJob::processFinished( int exitCode , QProcess::ExitStatus status ) 
{
    if (exitCode == 0 && status == QProcess::NormalExit)
        model()->appendLine( i18n("*** Exited normally ***") );
    else
        if (status == QProcess::NormalExit)
            model()->appendLine( i18n("*** Exited with return code: %1 ***", QString::number(exitCode)) );
        else 
            if (error() == KJob::KilledJobError)
                model()->appendLine( i18n("*** Process aborted ***") );
            else
                model()->appendLine( i18n("*** Crashed with return code: %1 ***", QString::number(exitCode)) );
    kDebug() << "Process done";
    emitResult();
}

void NativeAppJob::processError( QProcess::ProcessError error ) 
{
    if( error == QProcess::FailedToStart )
    {
        setError( -1 );
        QString errmsg =  i18n("Could not start program '%1'. Make sure that the "
                           "path is specified correctly.", proc->property("executable").toString() );
        KMessageBox::error( KDevelop::ICore::self()->uiController()->activeMainWindow(), errmsg, i18n("Could not start application") );
        setErrorText( errmsg );
        emitResult();
    }
    kDebug() << "Process error";
}

KDevelop::OutputModel* NativeAppJob::model()
{
    return dynamic_cast<KDevelop::OutputModel*>( OutputJob::model() );
}


#include "nativeappjob.moc"
