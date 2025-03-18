/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "scriptappjob.h"
#include "executescriptplugin.h"

#include <QFileInfo>

#include <KLocalizedString>
#include <KProcess>
#include <KSharedConfig>
#include <KShell>

#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/iruntimecontroller.h>
#include <interfaces/iruntime.h>
#include <outputview/outputmodel.h>
#include <outputview/outputdelegate.h>
#include <util/processlinemaker.h>
#include <util/environmentprofilelist.h>

#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <project/projectmodel.h>
#include <util/path.h>

#include "iexecutescriptplugin.h"
#include "debug.h"

using namespace KDevelop;

namespace {
void setProcessEnvironment(const QString& launchConfigurationName, QProcess& process, QString environmentProfileName)
{
    const EnvironmentProfileList environmentProfiles(KSharedConfig::openConfig());
    if (environmentProfileName.isEmpty()) {
        qCWarning(PLUGIN_EXECUTESCRIPT).noquote() << i18n(
            "No environment profile specified, looks like a broken configuration, please check run configuration '%1'. "
            "Using default environment profile.",
            launchConfigurationName);
        environmentProfileName = environmentProfiles.defaultProfileName();
    }
    process.setEnvironment(environmentProfiles.createEnvironment(environmentProfileName, process.systemEnvironment()));
}
} // unnamed namespace

ScriptAppJob::ScriptAppJob(ExecuteScriptPlugin* parent, KDevelop::ILaunchConfiguration* cfg)
    : OutputJob(parent)
    , proc(nullptr)
    , lineMaker(nullptr)
{
    qCDebug(PLUGIN_EXECUTESCRIPT) << "creating script app job";
    setCapabilities(Killable);

    auto* iface = KDevelop::ICore::self()->pluginController()->pluginForExtension(QStringLiteral("org.kdevelop.IExecuteScriptPlugin"))->extension<IExecuteScriptPlugin>();
    Q_ASSERT(iface);

    QString err;
    const auto detectError = [&err, this](int errorCode) {
        if (err.isEmpty()) {
            return false;
        }
        setError(errorCode);
        setErrorText(err);
        return true;
    };

    const auto interpreter = iface->interpreter(cfg, err);
    if (detectError(-2)) {
        return;
    }

    QUrl script;
    if( !iface->runCurrentFile( cfg ) )
    {
        script = iface->script( cfg, err );
        if (detectError(-3)) {
            return;
        }
    } else {
        KDevelop::IDocument* document = KDevelop::ICore::self()->documentController()->activeDocument();
        if( !document )
        {
            setError( -1 );
            setErrorText( i18n( "There is no active document to launch." ) );
            return;
        }
        script = ICore::self()->runtimeController()->currentRuntime()->pathInRuntime(KDevelop::Path(document->url())).toUrl();
    }
    const auto scriptPath = script.toLocalFile();

    QString remoteHost = iface->remoteHost( cfg, err );
    if (detectError(-4)) {
        return;
    }

    QStringList arguments = iface->arguments( cfg, err );
    if (detectError(-5)) {
        return;
    }

    auto currentFilterMode = static_cast<KDevelop::OutputModel::OutputFilterStrategy>( iface->outputFilterModeId( cfg ) );

    proc = new KProcess(this);
    lineMaker = new ProcessLineMaker(proc, this);

    setStandardToolView(KDevelop::IOutputView::RunView);
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll);
    auto* m = new KDevelop::OutputModel;
    m->setFilteringStrategy(currentFilterMode);
    setModel( m );
    setDelegate( new KDevelop::OutputDelegate );

    connect( lineMaker, &ProcessLineMaker::receivedStdoutLines, model(), &OutputModel::appendLines );
    connect(proc, &QProcess::errorOccurred, this, &ScriptAppJob::processError);
    connect( proc, QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished), this, &ScriptAppJob::processFinished );

    // Now setup the process parameters

    const auto launchConfigurationName = cfg->name();
    setProcessEnvironment(launchConfigurationName, *proc, iface->environmentProfileName(cfg));

    QUrl wc = iface->workingDirectory( cfg );
    if( !wc.isValid() || wc.isEmpty() )
    {
        wc = QUrl::fromLocalFile(QFileInfo{scriptPath}.absolutePath());
    }
    proc->setWorkingDirectory( ICore::self()->runtimeController()->currentRuntime()->pathInRuntime(KDevelop::Path(wc)).toLocalFile() );

    QStringList program;
    if (!remoteHost.isEmpty()) {
        program << QStringLiteral("ssh");
        QStringList parts = remoteHost.split(QLatin1Char(':'));
        program << parts.first();
        if (parts.length() > 1) {
            program << QLatin1String("-p ") + parts.at(1);
        }
    }
    program << interpreter;
    program << scriptPath;
    program << arguments;

    qCDebug(PLUGIN_EXECUTESCRIPT) << "setting app:" << program;

    proc->setOutputChannelMode(KProcess::MergedChannels);

    proc->setProgram( program );

    setObjectName(launchConfigurationName);
}


void ScriptAppJob::start()
{
    qCDebug(PLUGIN_EXECUTESCRIPT) << "launching?" << proc;
    if( proc )
    {
        Q_ASSERT(error() == NoError);
        startOutput();
        appendLine( i18n("Starting: %1", proc->program().join(QLatin1Char( ' ' ) ) ) );
        ICore::self()->runtimeController()->currentRuntime()->startProcess(proc);
    } else
    {
        // No process means we've returned early on from the constructor, some bad error happened
        Q_ASSERT(error() != NoError);
        emitResult();
    }
}

bool ScriptAppJob::doKill()
{
    if( proc ) {
        proc->kill();
        appendLine( i18n( "*** Killed Application ***" ) );
    }
    return true;
}


void ScriptAppJob::processFinished( int exitCode , QProcess::ExitStatus status )
{
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
    qCDebug(PLUGIN_EXECUTESCRIPT) << "Process done";
    emitResult();
}

void ScriptAppJob::processError( QProcess::ProcessError error )
{
    qCDebug(PLUGIN_EXECUTESCRIPT) << proc->readAllStandardError();
    qCDebug(PLUGIN_EXECUTESCRIPT) << proc->readAllStandardOutput();
    qCDebug(PLUGIN_EXECUTESCRIPT) << proc->errorString();
    if( error == QProcess::FailedToStart )
    {
        setError( FailedShownError );
        QString errmsg =  i18n("*** Could not start program '%1'. Make sure that the "
                           "path is specified correctly ***", proc->program().join(QLatin1Char( ' ' ) ) );
        appendLine( errmsg );
        setErrorText( errmsg );
        emitResult();
    }
    qCDebug(PLUGIN_EXECUTESCRIPT) << "Process error";
}

void ScriptAppJob::appendLine(const QString& l)
{
    if (KDevelop::OutputModel* m = model()) {
        m->appendLine(l);
    }
}

KDevelop::OutputModel* ScriptAppJob::model()
{
    return qobject_cast<KDevelop::OutputModel*>(OutputJob::model());
}

#include "moc_scriptappjob.cpp"
