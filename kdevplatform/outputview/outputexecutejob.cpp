/*
    SPDX-FileCopyrightText: 2012 Ivan Shapovalov <intelfx100@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "outputexecutejob.h"
#include "outputmodel.h"
#include "outputdelegate.h"
#include "debug.h"
#include <interfaces/icore.h>
#include <interfaces/iruntime.h>
#include <interfaces/iruntimecontroller.h>
#include <util/environmentprofilelist.h>
#include <util/processlinemaker.h>
#include <KProcess>
#include <KLocalizedString>
#include <KShell>
#include <QFileInfo>
#include <QDir>

#include <memory>

namespace KDevelop
{

class OutputExecuteJobPrivate
{
public:
    explicit OutputExecuteJobPrivate( KDevelop::OutputExecuteJob* owner );

    void childProcessStdout();
    void childProcessStderr();

    void emitProgress(const IFilterStrategy::Progress& progress);

    QString joinCommandLine() const;

    template< typename T >
    static void mergeEnvironment( QProcessEnvironment& dest, const T& src );
    QProcessEnvironment effectiveEnvironment(const QUrl& workingDirectory) const;
    QStringList effectiveCommandLine() const;

    OutputExecuteJob* m_owner;

    KProcess* m_process;
    ProcessLineMaker* m_lineMaker;
    OutputExecuteJob::JobStatus m_status;
    OutputExecuteJob::JobProperties m_properties;
    OutputModel::OutputFilterStrategy m_filteringStrategy;
    std::unique_ptr<IFilterStrategy> m_filteringStrategyPtr;
    QStringList m_arguments;
    QStringList m_privilegedExecutionCommand;
    QUrl m_workingDirectory;
    QString m_environmentProfile;
    QHash<QString, QString> m_environmentOverrides;
    bool m_executeOnHost = false;
    bool m_checkExitCode = true;
};

OutputExecuteJobPrivate::OutputExecuteJobPrivate( OutputExecuteJob* owner ) :
    m_owner( owner ),
    m_process( new KProcess( m_owner ) ),
    m_lineMaker( new ProcessLineMaker( m_owner ) ), // do not assign process to the line maker as we'll feed it data ourselves
    m_status( OutputExecuteJob::JobNotStarted ),
    m_properties( OutputExecuteJob::DisplayStdout ),
    m_filteringStrategy(OutputModel::NoFilter)
{
}

OutputExecuteJob::OutputExecuteJob( QObject* parent, OutputJob::OutputJobVerbosity verbosity ):
    OutputJob( parent, verbosity ),
    d_ptr(new OutputExecuteJobPrivate(this))
{
    Q_D(OutputExecuteJob);

    d->m_process->setOutputChannelMode( KProcess::SeparateChannels );

    connect( d->m_process, QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),
             this, &OutputExecuteJob::childProcessExited );
    connect( d->m_process, &QProcess::errorOccurred,
             this, &OutputExecuteJob::childProcessError );
    connect( d->m_process, &KProcess::readyReadStandardOutput,
             this, [this] { Q_D(OutputExecuteJob); d->childProcessStdout(); } );
    connect( d->m_process, &KProcess::readyReadStandardError,
             this, [this] { Q_D(OutputExecuteJob); d->childProcessStderr(); } );
}

OutputExecuteJob::~OutputExecuteJob()
{
    Q_D(OutputExecuteJob);

    // indicates if process is running and survives kill, then we cannot do anything
    bool killSuccessful = d->m_process->state() == QProcess::NotRunning;
    if( !killSuccessful ) {
        killSuccessful = doKill();
    }

    Q_ASSERT( d->m_process->state() == QProcess::NotRunning || !killSuccessful );
}

OutputExecuteJob::JobStatus OutputExecuteJob::status() const
{
    Q_D(const OutputExecuteJob);

    return d->m_status;
}

OutputModel* OutputExecuteJob::model() const
{
    return qobject_cast<OutputModel*>(OutputJob::model());
}

QStringList OutputExecuteJob::commandLine() const
{
    Q_D(const OutputExecuteJob);

    return d->m_arguments;
}

OutputExecuteJob& OutputExecuteJob::operator<<( const QString& argument )
{
    Q_D(OutputExecuteJob);

    d->m_arguments << argument;
    return *this;
}

OutputExecuteJob& OutputExecuteJob::operator<<( const QStringList& arguments )
{
    Q_D(OutputExecuteJob);

    d->m_arguments << arguments;
    return *this;
}

QStringList OutputExecuteJob::privilegedExecutionCommand() const
{
    Q_D(const OutputExecuteJob);

    return d->m_privilegedExecutionCommand;
}

void OutputExecuteJob::setPrivilegedExecutionCommand( const QStringList& command )
{
    Q_D(OutputExecuteJob);

    d->m_privilegedExecutionCommand = command;
}

void OutputExecuteJob::setJobName( const QString& name )
{
    setObjectName(name);
    setTitle(name);
    setToolTitle(name);
}

QUrl OutputExecuteJob::workingDirectory() const
{
    Q_D(const OutputExecuteJob);

    return d->m_workingDirectory;
}

void OutputExecuteJob::setWorkingDirectory( const QUrl& url )
{
    Q_D(OutputExecuteJob);

    d->m_workingDirectory = url;
}

void OutputExecuteJob::start()
{
    Q_D(OutputExecuteJob);

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
            emitResult();
            return;
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
            emitResult();
            return;
        } else if( !effectiveWorkingDirectory.isLocalFile() ) {
            setError( InvalidWorkingDirectoryError );
            if( isBuilder ) {
                setErrorText( i18n( "Build directory '%1' is not a local path", effectiveWorkingDirectory.toDisplayString(QUrl::PreferLocalFile) ) );
            } else {
                setErrorText( i18n( "Working directory '%1' is not a local path", effectiveWorkingDirectory.toDisplayString(QUrl::PreferLocalFile) ) );
            }
            emitResult();
            return;
        }

        QFileInfo workingDirInfo( effectiveWorkingDirectory.toLocalFile() );
        if( !workingDirInfo.isDir() ) {
            if (!QDir().mkdir(effectiveWorkingDirectory.toLocalFile())) {
                setError( InvalidWorkingDirectoryError );
                if( isBuilder ) {
                    setErrorText( i18n( "Build directory '%1' does not exist or is not a directory", effectiveWorkingDirectory.toDisplayString(QUrl::PreferLocalFile) ) );
                } else {
                    setErrorText( i18n( "Working directory '%1' does not exist or is not a directory", effectiveWorkingDirectory.toDisplayString(QUrl::PreferLocalFile) ) );
                }
                emitResult();
                return;
            }
        }

        setModel( new OutputModel( effectiveWorkingDirectory ) );
    }
    Q_ASSERT( model() );

    if (d->m_filteringStrategyPtr) {
        model()->setFilteringStrategy(d->m_filteringStrategyPtr.release());
    } else {
        model()->setFilteringStrategy(d->m_filteringStrategy);
    }

    setDelegate( new OutputDelegate );

    connect(model(), &OutputModel::progress, this, [this](const IFilterStrategy::Progress& progress) {
        Q_D(OutputExecuteJob);
        d->emitProgress(progress);
    });

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

    startOutput();

    if( !effectiveWorkingDirectory.isEmpty() ) {
        d->m_process->setWorkingDirectory( effectiveWorkingDirectory.toLocalFile() );
    }

    d->m_process->setProcessEnvironment( d->effectiveEnvironment(effectiveWorkingDirectory) );

    if (!d->effectiveCommandLine().isEmpty()) {
        d->m_process->setProgram( d->effectiveCommandLine() );
        // there is no way to input data in the output view so redirect stdin to the null device
        d->m_process->setStandardInputFile(QProcess::nullDevice());
        qCDebug(OUTPUTVIEW) << "Starting:" << d->effectiveCommandLine() << d->m_process->program() << "in" << d->m_process->workingDirectory();
        if (d->m_executeOnHost) {
            d->m_process->start();
        } else {
            KDevelop::ICore::self()->runtimeController()->currentRuntime()->startProcess(d->m_process);
        }
        model()->appendLine(d->m_process->workingDirectory() + QLatin1String("> ") + KShell::joinArgs(d->m_process->program()));
    } else {
        QString errorMessage = i18n("Failed to specify program to start: %1", d->joinCommandLine());
        model()->appendLine( i18n( "*** %1 ***", errorMessage) );
        setErrorText(errorMessage);
        setError( FailedShownError );
        emitResult();
    }
}

bool OutputExecuteJob::doKill()
{
    Q_D(OutputExecuteJob);

    const int terminateKillTimeout = 1000; // msecs

    if( d->m_status != JobRunning ) {
        return true;
    }
    d->m_status = JobCanceled;

    d->m_process->terminate();
    bool terminated = d->m_process->waitForFinished( terminateKillTimeout );
    if( !terminated ) {
        d->m_process->kill();
        terminated = d->m_process->waitForFinished( terminateKillTimeout );
    }
    d->m_lineMaker->flushBuffers();
    if( terminated ) {
        model()->appendLine( i18n( "*** Killed process ***" ) );
    } else {
        // It survived SIGKILL, leave it alone...
        qCWarning(OUTPUTVIEW) << "Could not kill the running process:" << d->m_process->error();
        model()->appendLine( i18n( "*** Warning: could not kill the process ***" ) );
        return false;
    }
    return true;
}

void OutputExecuteJob::childProcessError( QProcess::ProcessError processError )
{
    Q_D(OutputExecuteJob);

    // This can be called twice: one time via an error() signal, and second - from childProcessExited().
    // Avoid doing things in second time.
    if( d->m_status != OutputExecuteJob::JobRunning )
        return;
    d->m_status = OutputExecuteJob::JobFailed;

    qCWarning(OUTPUTVIEW) << "process error:" << processError << d->m_process->errorString()
                          << ", the command line:" << d->joinCommandLine();

    QString errorValue;
    switch( processError ) {
        case QProcess::FailedToStart:
            errorValue = i18n("%1 has failed to start", commandLine().at(0));
            break;

        case QProcess::Crashed:
            errorValue = i18n("%1 has crashed", commandLine().at(0));
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

        case QProcess::UnknownError:
            errorValue = i18n("Exit code %1", d->m_process->exitCode());
            break;
    }

    setError( FailedShownError );
    setErrorText( errorValue );
    d->m_lineMaker->flushBuffers();
    model()->appendLine( i18n("*** Failure: %1 ***", errorValue) );
    emitResult();
}

void OutputExecuteJob::childProcessExited( int exitCode, QProcess::ExitStatus exitStatus )
{
    Q_D(OutputExecuteJob);

    if( d->m_status != JobRunning )
        return;

    if( exitStatus == QProcess::CrashExit ) {
        childProcessError( QProcess::Crashed );
    } else if ( d->m_checkExitCode && exitCode != 0 ) {
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
    qCDebug(OUTPUTVIEW) << out;
    if( m_properties.testFlag( OutputExecuteJob::DisplayStdout ) ) {
        m_lineMaker->slotReceivedStdout( out );
    }
}

void OutputExecuteJobPrivate::childProcessStderr()
{
    QByteArray err = m_process->readAllStandardError();
    qCDebug(OUTPUTVIEW) << err;
    if( m_properties.testFlag( OutputExecuteJob::DisplayStderr ) ) {
        m_lineMaker->slotReceivedStderr( err );
    }
}

void OutputExecuteJobPrivate::emitProgress(const IFilterStrategy::Progress& progress)
{
    if (progress.percent != -1) {
        m_owner->emitPercent(progress.percent, 100);
    }
    if (!progress.status.isEmpty()) {
        emit m_owner->infoMessage(m_owner, progress.status);
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
    Q_D(OutputExecuteJob);

    d->m_filteringStrategy = strategy;

    // clear the other
    d->m_filteringStrategyPtr.reset();
}

void OutputExecuteJob::setFilteringStrategy(IFilterStrategy* filterStrategy)
{
    Q_D(OutputExecuteJob);

    d->m_filteringStrategyPtr.reset(filterStrategy);

    // clear the other
    d->m_filteringStrategy = OutputModel::NoFilter;
}

OutputExecuteJob::JobProperties OutputExecuteJob::properties() const
{
    Q_D(const OutputExecuteJob);

    return d->m_properties;
}

void OutputExecuteJob::setProperties( OutputExecuteJob::JobProperties properties, bool override )
{
    Q_D(OutputExecuteJob);

    if( override ) {
        d->m_properties = properties;
    } else {
        d->m_properties |= properties;
    }
}

void OutputExecuteJob::unsetProperties( OutputExecuteJob::JobProperties properties )
{
    Q_D(OutputExecuteJob);

    d->m_properties &= ~properties;
}

QString OutputExecuteJob::environmentProfile() const
{
    Q_D(const OutputExecuteJob);

    return d->m_environmentProfile;
}

void OutputExecuteJob::setEnvironmentProfile( const QString& profile )
{
    Q_D(OutputExecuteJob);

    d->m_environmentProfile = profile;
}

void OutputExecuteJob::addEnvironmentOverride( const QString& name, const QString& value )
{
    Q_D(OutputExecuteJob);

    d->m_environmentOverrides[name] = value;
}

void OutputExecuteJob::removeEnvironmentOverride( const QString& name )
{
    Q_D(OutputExecuteJob);

    d->m_environmentOverrides.remove( name );
}


void OutputExecuteJob::setExecuteOnHost(bool executeHost)
{
    Q_D(OutputExecuteJob);

    d->m_executeOnHost = executeHost;
}

bool OutputExecuteJob::executeOnHost() const
{
    Q_D(const OutputExecuteJob);

    return d->m_executeOnHost;
}

bool OutputExecuteJob::checkExitCode() const
{
    Q_D(const OutputExecuteJob);

    return d->m_checkExitCode;
}

void OutputExecuteJob::setCheckExitCode(bool check)
{
    Q_D(OutputExecuteJob);

    d->m_checkExitCode = check;
}

template< typename T >
void OutputExecuteJobPrivate::mergeEnvironment( QProcessEnvironment& dest, const T& src )
{
    for( typename T::const_iterator it = src.begin(); it != src.end(); ++it ) {
        dest.insert( it.key(), it.value() );
    }
}

QProcessEnvironment OutputExecuteJobPrivate::effectiveEnvironment(const QUrl& workingDirectory) const
{
    const EnvironmentProfileList environmentProfiles(KSharedConfig::openConfig());
    QString environmentProfile = m_owner->environmentProfile();
    if( environmentProfile.isEmpty() ) {
        environmentProfile = environmentProfiles.defaultProfileName();
    }
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    auto userEnv = environmentProfiles.variables(environmentProfile);
    expandVariables(userEnv, environment);

    OutputExecuteJobPrivate::mergeEnvironment( environment, userEnv );
    OutputExecuteJobPrivate::mergeEnvironment( environment, m_environmentOverrides );
    if( m_properties.testFlag( OutputExecuteJob::PortableMessages ) ) {
        environment.remove( QStringLiteral( "LC_ALL" ) );
        environment.insert( QStringLiteral( "LC_MESSAGES" ), QStringLiteral( "C" ) );
    }
    if (!workingDirectory.isEmpty() && environment.contains(QStringLiteral("PWD"))) {
        // also update the environment variable for the cwd, otherwise scripts can break easily
        environment.insert(QStringLiteral("PWD"), workingDirectory.toLocalFile());
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
        return QStringList() << m_owner->privilegedExecutionCommand() << QStringLiteral("--") << m_owner->commandLine();
    } else {
        return m_owner->commandLine();
    }
}

} // namespace KDevelop

#include "moc_outputexecutejob.cpp"
