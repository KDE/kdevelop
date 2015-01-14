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
#include "debug.h"
#include <util/environmentgrouplist.h>
#include <util/processlinemaker.h>
#include <KProcess>
#include <KLocalizedString>
#include <KShell>
#include <QFileInfo>
#include <QDir>

namespace KDevelop
{

class OutputExecuteJobPrivate
{
public:
    OutputExecuteJobPrivate( KDevelop::OutputExecuteJob* owner );

    void childProcessStdout();
    void childProcessStderr();

    QString joinCommandLine() const;
    QString getJobName();

    template< typename T >
    static void mergeEnvironment( QProcessEnvironment& dest, const T& src );
    QProcessEnvironment effectiveEnvironment() const;
    QStringList effectiveCommandLine() const;

    OutputExecuteJob* m_owner;

    KProcess* m_process;
    ProcessLineMaker* m_lineMaker;
    OutputExecuteJob::JobStatus m_status;
    OutputExecuteJob::JobProperties m_properties;
    OutputModel::OutputFilterStrategy m_filteringStrategy;
    QStringList m_arguments;
    QStringList m_privilegedExecutionCommand;
    QUrl m_workingDirectory;
    QString m_environmentProfile;
    QHash<QString, QString> m_environmentOverrides;
    QString m_jobName;
    bool m_outputStarted;

    QByteArray m_processStdout;
    QByteArray m_processStderr;
};

OutputExecuteJobPrivate::OutputExecuteJobPrivate( OutputExecuteJob* owner ) :
    m_owner( owner ),
    m_process( new KProcess( m_owner ) ),
    m_lineMaker( new ProcessLineMaker( m_owner ) ), // do not assign process to the line maker as we'll feed it data ourselves
    m_status( OutputExecuteJob::JobNotStarted ),
    m_properties( OutputExecuteJob::DisplayStdout ),
    m_filteringStrategy( OutputModel::NoFilter ),
    m_outputStarted( false )
{
}

OutputExecuteJob::OutputExecuteJob( QObject* parent, OutputJob::OutputJobVerbosity verbosity ):
    OutputJob( parent, verbosity ),
    d( new OutputExecuteJobPrivate( this ) )
{
    d->m_process->setOutputChannelMode( KProcess::SeparateChannels );

    connect( d->m_process, static_cast<void(KProcess::*)(int,QProcess::ExitStatus)>(&KProcess::finished),
             this, &OutputExecuteJob::childProcessExited );
    connect( d->m_process, static_cast<void(KProcess::*)(QProcess::ProcessError)>(&KProcess::error),
             this, &OutputExecuteJob::childProcessError );
    connect( d->m_process, &KProcess::readyReadStandardOutput,
             this, [=] { d->childProcessStdout(); } );
    connect( d->m_process, &KProcess::readyReadStandardError,
             this, [=] { d->childProcessStderr(); } );
}

OutputExecuteJob::~OutputExecuteJob()
{
    if( d->m_process->state() != QProcess::NotRunning ) {
        doKill();
    }
    Q_ASSERT( d->m_process->state() == QProcess::NotRunning );
    delete d;
}

OutputExecuteJob::JobStatus OutputExecuteJob::status() const
{
    return d->m_status;
}

OutputModel* OutputExecuteJob::model() const
{
    return dynamic_cast<OutputModel*> ( OutputJob::model() );
}

QStringList OutputExecuteJob::commandLine() const
{
    return d->m_arguments;
}

OutputExecuteJob& OutputExecuteJob::operator<<( const QString& argument )
{
    d->m_arguments << argument;
    return *this;
}

OutputExecuteJob& OutputExecuteJob::operator<<( const QStringList& arguments )
{
    d->m_arguments << arguments;
    return *this;
}

QStringList OutputExecuteJob::privilegedExecutionCommand() const
{
    return d->m_privilegedExecutionCommand;
}

void OutputExecuteJob::setPrivilegedExecutionCommand( const QStringList& command )
{
    d->m_privilegedExecutionCommand = command;
}

void OutputExecuteJob::setJobName( const QString& name )
{
    d->m_jobName = name;
    QString jobName = d->getJobName();
    setObjectName( jobName );
    setTitle( jobName );
}

QUrl OutputExecuteJob::workingDirectory() const
{
    return d->m_workingDirectory;
}

void OutputExecuteJob::setWorkingDirectory( const QUrl& url )
{
    d->m_workingDirectory = url;
}

void OutputExecuteJob::start()
{
    Q_ASSERT( d->m_status == JobNotStarted );
    d->m_status = JobRunning;

    const bool isBuilder = d->m_properties.testFlag( IsBuilderHint );

    const QUrl effectiveWorkingDirectory = workingDirectory();
    if( effectiveWorkingDirectory.isEmpty() ) {
        if( d->m_properties.testFlag( NeedWorkingDirectory ) ) {
            // A directory is not given, but we need it.
            setError( InvalidWorkingDirectoryError );
            if( isBuilder ) {
                setErrorText( i18n( "No build directory specified for a builder job." ) );
            } else {
                setErrorText( i18n( "No working directory specified for a process." ) );
            }
            return emitResult();
        }

        setModel( new OutputModel );
    } else {
        // Basic sanity checks.
        if( !effectiveWorkingDirectory.isValid() ) {
            setError( InvalidWorkingDirectoryError );
            if( isBuilder ) {
                setErrorText( i18n( "Invalid build directory '%1'", effectiveWorkingDirectory.toDisplayString(QUrl::PreferLocalFile) ) );
            } else {
                setErrorText( i18n( "Invalid working directory '%1'", effectiveWorkingDirectory.toDisplayString(QUrl::PreferLocalFile) ) );
            }
            return emitResult();
        } else if( !effectiveWorkingDirectory.isLocalFile() ) {
            setError( InvalidWorkingDirectoryError );
            if( isBuilder ) {
                setErrorText( i18n( "Build directory '%1' is not a local path", effectiveWorkingDirectory.toDisplayString(QUrl::PreferLocalFile) ) );
            } else {
                setErrorText( i18n( "Working directory '%1' is not a local path", effectiveWorkingDirectory.toDisplayString(QUrl::PreferLocalFile) ) );
            }
            return emitResult();
        }

        QFileInfo workingDirInfo( effectiveWorkingDirectory.toLocalFile() );
        if( !workingDirInfo.isDir() ) {
            // If a working directory does not actually exist, either bail out or create it empty,
            // depending on what we need by properties.
            // We use a dedicated bool variable since !isDir() may also mean that it exists,
            // but is not a directory, or a symlink to an inexistent object.
            bool successfullyCreated = false;
            if( !d->m_properties.testFlag( CheckWorkingDirectory ) ) {
                successfullyCreated = QDir().mkdir( effectiveWorkingDirectory.toLocalFile() );
            }
            if( !successfullyCreated ) {
                setError( InvalidWorkingDirectoryError );
                if( isBuilder ) {
                    setErrorText( i18n( "Build directory '%1' does not exist or is not a directory", effectiveWorkingDirectory.toDisplayString(QUrl::PreferLocalFile) ) );
                } else {
                    setErrorText( i18n( "Working directory '%1' does not exist or is not a directory", effectiveWorkingDirectory.toDisplayString(QUrl::PreferLocalFile) ) );
                }
                return emitResult();
            }
        }

        setModel( new OutputModel( effectiveWorkingDirectory ) );
    }
    Q_ASSERT( model() );

    model()->setFilteringStrategy( d->m_filteringStrategy );
    setDelegate( new OutputDelegate );

    // Slots hasRawStdout() and hasRawStderr() are responsible
    // for feeding raw data to the line maker; so property-based channel filtering is implemented there.
    if( d->m_properties.testFlag( PostProcessOutput ) ) {
        connect( d->m_lineMaker, &ProcessLineMaker::receivedStdoutLines,
                 this, &OutputExecuteJob::postProcessStdout );
        connect( d->m_lineMaker, &ProcessLineMaker::receivedStderrLines,
                 this, &OutputExecuteJob::postProcessStderr );
    } else {
        connect( d->m_lineMaker, &ProcessLineMaker::receivedStdoutLines, model(),
                 &OutputModel::appendLines );
        connect( d->m_lineMaker, &ProcessLineMaker::receivedStderrLines, model(),
                 &OutputModel::appendLines );
    }

    if( !d->m_properties.testFlag( NoSilentOutput ) || verbosity() != Silent ) {
        d->m_outputStarted = true;
        startOutput();
    }

    const QString joinedCommandLine = d->joinCommandLine();
    QString headerLine;
    if( !effectiveWorkingDirectory.isEmpty() ) {
        headerLine = effectiveWorkingDirectory.toString( QUrl::PreferLocalFile | QUrl::StripTrailingSlash ) + "> " + joinedCommandLine;
    } else {
        headerLine = joinedCommandLine;
    }
    model()->appendLine( headerLine );

    if( !effectiveWorkingDirectory.isEmpty() ) {
        d->m_process->setWorkingDirectory( effectiveWorkingDirectory.toLocalFile() );
    }
    d->m_process->setProcessEnvironment( d->effectiveEnvironment() );
    d->m_process->setProgram( d->effectiveCommandLine() );

    qCDebug(OUTPUTVIEW) << "Starting:" << d->m_process->program().join(" ") << "in" << d->m_process->workingDirectory();
    d->m_process->start();
}

bool OutputExecuteJob::doKill()
{
    const int terminateKillTimeout = 1000; // msecs

    if( d->m_status != JobRunning )
        return true;
    d->m_status = JobCanceled;

    d->m_process->terminate();
    bool terminated = d->m_process->waitForFinished( terminateKillTimeout );
    if( !terminated ) {
        d->m_process->kill();
        terminated = d->m_process->waitForFinished( terminateKillTimeout );
    }
    d->m_lineMaker->flushBuffers();
    if( terminated ) {
        model()->appendLine( i18n( "*** Aborted ***" ) );
    } else {
        // It survived SIGKILL, leave it alone...
        model()->appendLine( i18n( "*** Warning: could not kill the process ***" ) );
    }
    return true;
}

void OutputExecuteJob::childProcessError( QProcess::ProcessError processError )
{
    // This can be called twice: one time via an error() signal, and second - from childProcessExited().
    // Avoid doing things in second time.
    if( d->m_status != OutputExecuteJob::JobRunning )
        return;
    d->m_status = OutputExecuteJob::JobFailed;

    QString errorValue;
    switch( processError ) {
        case QProcess::FailedToStart:
            errorValue = i18n("%1 has failed to start", commandLine().first());
            break;

        case QProcess::Crashed:
            errorValue = i18n("%1 has crashed", commandLine().first());
            break;

        case QProcess::ReadError:
            errorValue = i18n("Read error");
            break;

        case QProcess::WriteError:
            errorValue = i18n("Write error");
            break;

        case QProcess::Timedout:
            errorValue = i18n("Waiting for the process has timed out");
            break;

        default:
        case QProcess::UnknownError:
            errorValue = i18n("Exit code %1", d->m_process->exitCode());
            break;
    }

    // Show the toolview if it's hidden for the user to be able to diagnose errors.
    if( !d->m_outputStarted ) {
        d->m_outputStarted = true;
        startOutput();
    }

    setError( FailedShownError );
    setErrorText( errorValue );
    d->m_lineMaker->flushBuffers();
    model()->appendLine( i18n("*** Failure: %1 ***", errorValue) );
    emitResult();
}

void OutputExecuteJob::childProcessExited( int exitCode, QProcess::ExitStatus exitStatus )
{
    if( d->m_status != JobRunning )
        return;

    if( exitStatus == QProcess::CrashExit ) {
        childProcessError( QProcess::Crashed );
    } else if ( exitCode != 0 ) {
        childProcessError( QProcess::UnknownError );
    } else {
        d->m_status = JobSucceeded;
        d->m_lineMaker->flushBuffers();
        model()->appendLine( i18n("*** Finished ***") );
        emitResult();
    }
}

void OutputExecuteJobPrivate::childProcessStdout()
{
    QByteArray out = m_process->readAllStandardOutput();
    if( m_properties.testFlag( OutputExecuteJob::AccumulateStdout ) ) {
        m_processStdout += out;
    }
    if( m_properties.testFlag( OutputExecuteJob::DisplayStdout ) ) {
        m_lineMaker->slotReceivedStdout( out );
    }
}

void OutputExecuteJobPrivate::childProcessStderr()
{
    QByteArray err = m_process->readAllStandardError();
    if( m_properties.testFlag( OutputExecuteJob::AccumulateStderr ) ) {
        m_processStderr += err;
    }
    if( m_properties.testFlag( OutputExecuteJob::DisplayStderr ) ) {
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
    d->m_filteringStrategy = strategy;
}

OutputExecuteJob::JobProperties OutputExecuteJob::properties() const
{
    return d->m_properties;
}

void OutputExecuteJob::setProperties( OutputExecuteJob::JobProperties properties, bool override )
{
    if( override ) {
        d->m_properties = properties;
    } else {
        d->m_properties |= properties;
    }
}

void OutputExecuteJob::unsetProperties( OutputExecuteJob::JobProperties properties )
{
    d->m_properties &= ~properties;
}

QString OutputExecuteJob::environmentProfile() const
{
    return d->m_environmentProfile;
}

void OutputExecuteJob::setEnvironmentProfile( const QString& profile )
{
    d->m_environmentProfile = profile;
}

void OutputExecuteJob::addEnvironmentOverride( const QString& name, const QString& value )
{
    d->m_environmentOverrides[name] = value;
}

void OutputExecuteJob::removeEnvironmentOverride( const QString& name )
{
    d->m_environmentOverrides.remove( name );
}

template< typename T >
void OutputExecuteJobPrivate::mergeEnvironment( QProcessEnvironment& dest, const T& src )
{
    for( typename T::const_iterator it = src.begin(); it != src.end(); ++it ) {
        dest.insert( it.key(), it.value() );
    }
}

QProcessEnvironment OutputExecuteJobPrivate::effectiveEnvironment() const
{
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    const EnvironmentGroupList environmentGroup( KSharedConfig::openConfig() );
    QString environmentProfile = m_owner->environmentProfile();
    if( environmentProfile.isEmpty() ) {
        environmentProfile = environmentGroup.defaultGroup();
    }
    OutputExecuteJobPrivate::mergeEnvironment( environment, environmentGroup.variables( environmentProfile ) );
    OutputExecuteJobPrivate::mergeEnvironment( environment, m_environmentOverrides );
    if( m_properties.testFlag( OutputExecuteJob::PortableMessages ) ) {
        environment.remove( "LC_ALL" );
        environment.insert( "LC_MESSAGES", "C" );
    }
    return environment;
}

QString OutputExecuteJobPrivate::joinCommandLine() const
{
    return KShell::joinArgs( effectiveCommandLine() );
}

QStringList OutputExecuteJobPrivate::effectiveCommandLine() const
{
    // If we need to use a su-like helper, invoke it as
    // "helper -- our command line".
    QStringList privilegedCommand = m_owner->privilegedExecutionCommand();
    if( !privilegedCommand.isEmpty() ) {
        return QStringList() << m_owner->privilegedExecutionCommand() << "--" << m_owner->commandLine();
    } else {
        return m_owner->commandLine();
    }
}

QString OutputExecuteJobPrivate::getJobName()
{
    const QString joinedCommandLine = joinCommandLine();
    if( m_properties.testFlag( OutputExecuteJob::AppendProcessString ) ) {
        if( !m_jobName.isEmpty() ) {
            return m_jobName + ": " + joinedCommandLine;
        } else {
            return joinedCommandLine;
        }
    } else {
        return m_jobName;
    }
}

} // namespace KDevelop

#include "moc_outputexecutejob.cpp"
