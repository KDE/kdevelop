/* This file is part of KDevelop
Copyright 2012 Ivan Shapovalov <intelfx100@gmail.com>

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

#include "outputexecutejob.h"
#include "outputmodel.h"
#include "outputdelegate.h"
#include <util/environmentgrouplist.h>
#include <util/processlinemaker.h>
#include <KProcess>
#include <KGlobal>
#include <KLocalizedString>
#include <KShell>
#include <QFileInfo>

namespace KDevelop
{

OutputExecuteJob::OutputExecuteJob( QObject* parent, OutputJob::OutputJobVerbosity verbosity ):
    OutputJob( parent, verbosity ),
    m_process( new KProcess( this ) ),
    m_lineMaker( new ProcessLineMaker( this ) ), // do not assign process to the line maker as we'll feed it data ourselves
    m_status( JobNotStarted ),
    m_properties( CheckWorkingDirectory | DisplayStdout ),
    m_filteringStrategy( OutputModel::NoFilter ),
    m_arguments(),
    m_privilegedExecutionCommand(),
    m_workingDirectory(),
    m_environmentProfile(),
    m_environmentOverrides(),
    m_jobName(),
    m_outputStarted( false ),
    m_processStdout(),
    m_processStderr()
{
    m_process->setOutputChannelMode( KProcess::SeparateChannels );
    m_process->setTextModeEnabled( true );

    connect( m_process, SIGNAL(finished(int,QProcess::ExitStatus)),
             SLOT(childProcessExited(int,QProcess::ExitStatus)) );
    connect( m_process, SIGNAL(error(QProcess::ProcessError)),
             SLOT(childProcessError(QProcess::ProcessError)) );
    connect( m_process, SIGNAL(readyReadStandardOutput()),
             SLOT(childProcessStdout()) );
    connect( m_process, SIGNAL(readyReadStandardError()),
             SLOT(childProcessStderr()) );
}

OutputExecuteJob::~OutputExecuteJob()
{
    if( m_process->state() != QProcess::NotRunning ) {
        doKill();
    }
    Q_ASSERT( m_process->state() == QProcess::NotRunning );
}

OutputExecuteJob::JobStatus OutputExecuteJob::status() const
{
    return m_status;
}

OutputModel* OutputExecuteJob::model() const
{
    return dynamic_cast<OutputModel*> ( OutputJob::model() );
}

QStringList OutputExecuteJob::commandLine() const
{
    return m_arguments;
}

OutputExecuteJob& OutputExecuteJob::operator<<( const QString& argument )
{
    m_arguments << argument;
    updateCommandLine();
    updateJobName();
    return *this;
}

OutputExecuteJob& OutputExecuteJob::operator<<( const QStringList& arguments )
{
    m_arguments << arguments;
    updateCommandLine();
    updateJobName();
    return *this;
}

QStringList OutputExecuteJob::privilegedExecutionCommand() const
{
    return m_privilegedExecutionCommand;
}

void OutputExecuteJob::setPrivilegedExecutionCommand( const QStringList& command )
{
    m_privilegedExecutionCommand = command;
}

void OutputExecuteJob::setJobName( const QString& name )
{
    m_jobName = name;
    updateJobName();
}

KUrl OutputExecuteJob::workingDirectory() const
{
    return m_workingDirectory;
}

void OutputExecuteJob::setWorkingDirectory( const KUrl& url )
{
    m_workingDirectory = url;
}

void OutputExecuteJob::start()
{
    Q_ASSERT( m_status == JobNotStarted );
    m_status = JobRunning;

    const bool isBuilder = m_properties.testFlag( IsBuilderHint );

    const KUrl effectiveWorkingDirectory = workingDirectory();
    if( effectiveWorkingDirectory.isEmpty() ) {
        if( m_properties.testFlag( NeedWorkingDirectory ) ) { // a directory is not given, but we need it
            setError( InvalidWorkingDirectoryError );
            if( isBuilder ) {
                setErrorText( i18n( "No build directory specified for a builder job." ) );
            } else {
                setErrorText( i18n( "No working directory specified for a process." ) );
            }
            return emitResult();
        }

        setModel( new OutputModel );
    } else if( m_properties.testFlag( CheckWorkingDirectory ) ) { // a directory is given and we need to check it
        if( !effectiveWorkingDirectory.isValid() ) {
            setError( InvalidWorkingDirectoryError );
            if( isBuilder ) {
                setErrorText( i18n( "Invalid build directory '%1'", effectiveWorkingDirectory.prettyUrl() ) );
            } else {
                setErrorText( i18n( "Invalid working directory '%1'", effectiveWorkingDirectory.prettyUrl() ) );
            }
            return emitResult();
        } else if( !effectiveWorkingDirectory.isLocalFile() ) {
            setError( InvalidWorkingDirectoryError );
            if( isBuilder ) {
                setErrorText( i18n( "Build directory '%1' is not a local path", effectiveWorkingDirectory.prettyUrl() ) );
            } else {
                setErrorText( i18n( "Working directory '%1' is not a local path", effectiveWorkingDirectory.prettyUrl() ) );
            }
            return emitResult();
        } else if( !QFileInfo( effectiveWorkingDirectory.toLocalFile() ).isDir() ) {
            setError( InvalidWorkingDirectoryError );
            if( isBuilder ) {
                setErrorText( i18n( "Build directory '%1' is not a directory", effectiveWorkingDirectory.prettyUrl() ) );
            } else {
                setErrorText( i18n( "Working directory '%1' is not a directory", effectiveWorkingDirectory.prettyUrl() ) );
            }
            return emitResult();
        }

        setModel( new OutputModel( effectiveWorkingDirectory ) );
    }
    Q_ASSERT( model() );

    model()->setFilteringStrategy( m_filteringStrategy );
    setDelegate( new OutputDelegate );

    // Slots hasRawStdout() and hasRawStderr() are responsible
    // for feeding raw data to the line maker; so property-based channel filtering is implemented there.
    if( m_properties.testFlag( PostProcessOutput ) ) {
        connect( m_lineMaker, SIGNAL( receivedStdoutLines( QStringList ) ),
                 SLOT( postProcessStdout( QStringList ) ) );
        connect( m_lineMaker, SIGNAL( receivedStderrLines( QStringList ) ),
                 SLOT( postProcessStderr( QStringList ) ) );
    } else {
        connect( m_lineMaker, SIGNAL( receivedStdoutLines( QStringList ) ), model(),
                 SLOT(appendLines(QStringList)) );
        connect( m_lineMaker, SIGNAL( receivedStderrLines( QStringList ) ), model(),
                 SLOT(appendLines(QStringList)) );
    }

    if( !m_properties.testFlag( NoSilentOutput ) || verbosity() != Silent ) {
        m_outputStarted = true;
        startOutput();
    }

    QString headerLine;
    if( !effectiveWorkingDirectory.isEmpty() ) {
        headerLine = effectiveWorkingDirectory.toLocalFile( KUrl::RemoveTrailingSlash ) + "> " + m_joinedCommandLine;
    } else {
        headerLine = m_joinedCommandLine;
    }
    model()->appendLine( headerLine );

    if( !effectiveWorkingDirectory.isEmpty() ) {
        m_process->setWorkingDirectory( effectiveWorkingDirectory.toLocalFile() );
    }
    m_process->setProcessEnvironment( effectiveEnvironment() );
    m_process->setProgram( effectiveCommandLine() );
    m_process->start();
}

bool OutputExecuteJob::doKill()
{
    if( m_status != JobRunning )
        return true;
    m_status = JobCanceled;

    m_process->terminate();
    bool terminated = m_process->waitForFinished( 3000 );
    if( !terminated ) {
        m_process->kill();
        terminated = m_process->waitForFinished();
    }
    m_lineMaker->flushBuffers();
    model()->appendLine( i18n("*** Aborted ***") );
    if( !terminated ) {
        // It survived SIGKILL, leave it alone...
        model()->appendLine( i18n( "*** Warning: could not kill the process ***" ) );
    }
    return true;
}

void OutputExecuteJob::childProcessError( QProcess::ProcessError processError )
{
    // This can be called twice: one time via an error() signal, and second - from childProcessExited().
    // Avoid doing things in second time.
    if( m_status != JobRunning )
        return;
    m_status = JobFailed;

    QString errorValue;
    switch( processError ) {
        case QProcess::FailedToStart:
            errorValue = i18n("Process had failed to start");
            break;

        case QProcess::Crashed:
            errorValue = i18n("Process had crashed");
            break;

        case QProcess::ReadError:
            errorValue = i18n("Read error");
            break;

        case QProcess::WriteError:
            errorValue = i18n("Write error");
            break;

        case QProcess::Timedout:
            errorValue = i18n("Waiting for the process had timed out");
            break;

        default:
        case QProcess::UnknownError:
            errorValue = i18n("Exit code %1", m_process->exitCode());
            break;
    }

    // Show the toolview if it's hidden for the user to be able to diagnose errors.
    if( !m_outputStarted ) {
        m_outputStarted = true;
        startOutput();
    }

    setError( FailedShownError );
    setErrorText( errorValue );
    m_lineMaker->flushBuffers();
    model()->appendLine( i18n("*** Failure: %1 ***", errorValue) );
    emitResult();
}

void OutputExecuteJob::childProcessExited( int exitCode, QProcess::ExitStatus exitStatus )
{
    if( m_status != JobRunning )
        return;

    if( exitStatus == QProcess::CrashExit ) {
        childProcessError( QProcess::Crashed );
    } else if ( exitCode != 0 ) {
        childProcessError( QProcess::UnknownError );
    } else {
        m_status = JobSucceeded;
        m_lineMaker->flushBuffers();
        model()->appendLine( i18n("*** Finished ***") );
        emitResult();
    }
}

void OutputExecuteJob::childProcessStdout()
{
    QByteArray out = m_process->readAllStandardOutput();
    if( m_properties.testFlag( AccumulateStdout ) ) {
        m_processStdout += out;
    }
    if( m_properties.testFlag( DisplayStdout ) ) {
        m_lineMaker->slotReceivedStdout( out );
    }
}

void OutputExecuteJob::childProcessStderr()
{
    QByteArray err = m_process->readAllStandardError();
    if( m_properties.testFlag( AccumulateStderr ) ) {
        m_processStderr += err;
    }
    if( m_properties.testFlag( DisplayStderr ) ) {
        m_lineMaker->slotReceivedStderr( err );
    }
}

void OutputExecuteJob::postProcessStdout( const QStringList& lines )
{
    model()->appendLines( lines );
}

void OutputExecuteJob::postProcessStderr( const QStringList& lines )
{
    model()->appendLines( lines );
}

void OutputExecuteJob::setFilteringStrategy( OutputModel::OutputFilterStrategy strategy )
{
    m_filteringStrategy = strategy;
}

OutputExecuteJob::JobProperties OutputExecuteJob::properties() const
{
    return m_properties;
}

void OutputExecuteJob::setProperties( OutputExecuteJob::JobProperties properties, bool override )
{
    if( override ) {
        m_properties = properties;
    } else {
        m_properties |= properties;
    }
}

void OutputExecuteJob::unsetProperties( OutputExecuteJob::JobProperties properties )
{
    m_properties &= ~properties;
}

QString OutputExecuteJob::environmentProfile() const
{
    return m_environmentProfile;
}

void OutputExecuteJob::setEnvironmentProfile( const QString& profile )
{
    m_environmentProfile = profile;
}

void OutputExecuteJob::addEnvironmentOverride( const QString& name, const QString& value )
{
    m_environmentOverrides[name] = value;
}

void OutputExecuteJob::removeEnvironmentOverride( const QString& name )
{
    m_environmentOverrides.remove( name );
}

void OutputExecuteJob::mergeEnvironment( QProcessEnvironment& dest, const QMap<QString, QString>& src )
{
    for( QMap<QString, QString>::const_iterator it = src.begin(); it != src.end(); ++it ) {
        dest.insert( it.key(), it.value() );
    }
}

QProcessEnvironment OutputExecuteJob::effectiveEnvironment() const
{
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    const EnvironmentGroupList environmentGroup( KGlobal::config() );
    mergeEnvironment( environment, environmentGroup.variables( environmentProfile() ) );
    mergeEnvironment( environment, m_environmentOverrides );
    if( m_properties.testFlag( PortableMessages ) ) {
        environment.insert( "LC_MESSAGES", "C" );
    }
    return environment;
}

void OutputExecuteJob::updateCommandLine()
{
    m_joinedCommandLine = KShell::joinArgs( effectiveCommandLine() );
}

QStringList OutputExecuteJob::effectiveCommandLine() const
{
    // If we need to use a su-like helper, invoke it as
    // "helper -- our command line".
    QStringList privilegedCommand = privilegedExecutionCommand();
    if( !privilegedCommand.isEmpty() ) {
        return QStringList() << privilegedExecutionCommand() << "--" << commandLine();
    } else {
        return commandLine();
    }
}

void OutputExecuteJob::updateJobName()
{
    QString jobName;
    if( m_properties.testFlag( AppendProcessString ) ) {
        if( !m_jobName.isEmpty() ) {
            jobName = m_jobName + ": " + m_joinedCommandLine;
        } else {
            jobName = m_joinedCommandLine;
        }
    } else {
        jobName = m_jobName;
    }
    setObjectName( jobName );
    setTitle( jobName );
}

} // namespace KDevelop

#include "outputexecutejob.moc"