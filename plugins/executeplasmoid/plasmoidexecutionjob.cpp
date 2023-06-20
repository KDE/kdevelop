/*
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2009 Niko Sams <niko.sams@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "plasmoidexecutionjob.h"
#include "executeplasmoidplugin.h"

#include <QFileInfo>
#include <QDir>

#include <KLocalizedString>
#include <KConfigGroup>

#include <interfaces/ilaunchconfiguration.h>
#include <outputview/outputmodel.h>
#include <outputview/outputdelegate.h>
#include <util/commandexecutor.h>
#include <util/path.h>

#include <interfaces/iproject.h>
#include <project/projectmodel.h>
#include <QStandardPaths>

using namespace KDevelop;

PlasmoidExecutionJob::PlasmoidExecutionJob(ExecutePlasmoidPlugin* iface, ILaunchConfiguration* cfg)
    : OutputJob( iface )
{
    QString identifier = cfg->config().readEntry("PlasmoidIdentifier", "");

    Q_ASSERT(!identifier.isEmpty());
    setToolTitle(i18n("Plasmoid Viewer"));
    setCapabilities(Killable);
    setStandardToolView( IOutputView::RunView );
    setBehaviours(IOutputView::AllowUserClose | IOutputView::AutoScroll );
    setObjectName(QLatin1String("plasmoidviewer ")+identifier);
    setDelegate(new OutputDelegate);

    m_process = new CommandExecutor(executable(cfg), this);
    m_process->setArguments( arguments(cfg) );
    m_process->setWorkingDirectory(workingDirectory(cfg));

    auto* model = new OutputModel(QUrl::fromLocalFile(m_process->workingDirectory()), this);
    model->setFilteringStrategy(OutputModel::CompilerFilter);
    setModel( model );

    connect(m_process, &CommandExecutor::receivedStandardError, model,
            &OutputModel::appendLines );
    connect(m_process, &CommandExecutor::receivedStandardOutput, model,
            &OutputModel::appendLines );

    connect( m_process, &CommandExecutor::failed, this, &PlasmoidExecutionJob::slotFailed );
    connect( m_process, &CommandExecutor::completed, this, &PlasmoidExecutionJob::slotCompleted );
}


void PlasmoidExecutionJob::start()
{
    startOutput();
    model()->appendLine(m_process->workingDirectory() + QLatin1String("> ") + m_process->command() + QLatin1Char(' ') + m_process->arguments().join(QLatin1Char(' ')));
    m_process->start();
}

bool PlasmoidExecutionJob::doKill()
{
    m_process->kill();
    model()->appendLine( i18n("** Killed **") );
    return true;
}

OutputModel* PlasmoidExecutionJob::model()
{
    return qobject_cast<OutputModel*>( OutputJob::model() );
}

void PlasmoidExecutionJob::slotCompleted(int code)
{
    if( code != 0 ) {
        setError( FailedShownError );
        model()->appendLine( i18n("*** Failed ***") );
    } else {
        model()->appendLine( i18n("*** Finished ***") );
    }
    emitResult();
}

void PlasmoidExecutionJob::slotFailed(QProcess::ProcessError error)
{
    setError(error);
    // FIXME need more detail
    setErrorText(i18n("Plasmoid failed to execute on %1", m_process->workingDirectory()));
    model()->appendLine( i18n("*** Failed ***") );
    emitResult();
}

QString PlasmoidExecutionJob::executable(ILaunchConfiguration*)
{
    return QStandardPaths::findExecutable(QStringLiteral("plasmoidviewer"));
}

QStringList PlasmoidExecutionJob::arguments(ILaunchConfiguration* cfg)
{
    QStringList arguments = cfg->config().readEntry("Arguments", QStringList());
    if(workingDirectory(cfg) == QDir::tempPath()) {
        QString identifier = cfg->config().readEntry("PlasmoidIdentifier", "");
        arguments += QStringLiteral("-a");
        arguments += identifier;
    } else {
        arguments += { QStringLiteral("-a"), QStringLiteral(".") };
    }
    return arguments;
}

QString PlasmoidExecutionJob::workingDirectory(ILaunchConfiguration* cfg)
{
    QString workingDirectory;
    IProject* p = cfg->project();
    if(p) {
        QString identifier = cfg->config().readEntry("PlasmoidIdentifier", "");
        QString possiblePath = Path(p->path(), identifier).toLocalFile();
        if(QFileInfo(possiblePath).isDir()) {
            workingDirectory = possiblePath;
        }
    }

    if(workingDirectory.isEmpty())
    {
        workingDirectory = QDir::tempPath();
    }
    return workingDirectory;
}

#include "moc_plasmoidexecutionjob.cpp"
