/*  This file is part of KDevelop
    Copyright 2009 Andreas Pakulat <apaku@gmx.de>
    Copyright 2009 Niko Sams <niko.sams@gmail.com>

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

#include "plasmoidexecutionjob.h"
#include "executeplasmoidplugin.h"

#include <QFileInfo>
#include <QDir>

#include <kprocess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kparts/mainwindow.h>

#include <interfaces/ilaunchconfiguration.h>
#include <outputview/outputmodel.h>
#include <outputview/outputdelegate.h>
#include <util/processlinemaker.h>
#include <util/environmentgrouplist.h>
#include <util/commandexecutor.h>

#include <kshell.h>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/iproject.h>
#include <project/projectmodel.h>

PlasmoidExecutionJob::PlasmoidExecutionJob(ExecutePlasmoidPlugin* iface, KDevelop::ILaunchConfiguration* cfg)
    : KDevelop::OutputJob( iface )
{
    QString identifier = cfg->config().readEntry("PlasmoidIdentifier", "");
    QStringList arguments = cfg->config().readEntry("Arguments", QStringList());
    
    Q_ASSERT(!identifier.isEmpty());
    setToolTitle(i18n("Plasmoid Viewer"));
    setCapabilities(Killable);
    setStandardToolView( KDevelop::IOutputView::RunView );
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll );
    setObjectName("plasmoidviewer "+identifier);
    setDelegate(new KDevelop::OutputDelegate);
 
    QString workingDirectory;
    KDevelop::IProject* p = cfg->project();
    if(p) {
        QString possiblePath = KUrl(p->folder(), identifier).toLocalFile();
        if(QFileInfo(possiblePath).isDir()) {
            workingDirectory = possiblePath;
        } else {
            workingDirectory = p->folder().toLocalFile();
            arguments += identifier;
        }
    } else {
        workingDirectory = QDir::tempPath();
        arguments += identifier;
    }
    
    m_process = new KDevelop::CommandExecutor("plasmoidviewer", this);
    m_process->setArguments( arguments );
    m_process->setWorkingDirectory(workingDirectory);
    
    KDevelop::OutputModel* model = new KDevelop::OutputModel(m_process->workingDirectory(), this);
    model->setFilteringStrategy(KDevelop::OutputModel::CompilerFilter);
    setModel( model );

    connect(m_process, SIGNAL(receivedStandardError(QStringList)), model,
            SLOT(appendLines(QStringList)) );
    connect(m_process, SIGNAL(receivedStandardOutput(QStringList)), model,
            SLOT(appendLines(QStringList)) );
    
    connect( m_process, SIGNAL(failed(QProcess::ProcessError)), SLOT(slotFailed(QProcess::ProcessError)) );
    connect( m_process, SIGNAL(completed(int)), SLOT(slotCompleted(int)) );
}


void PlasmoidExecutionJob::start()
{
    startOutput();
    model()->appendLine( m_process->workingDirectory() + "> " + m_process->command() + " " + m_process->arguments().join(" ") );
    m_process->start();
}

bool PlasmoidExecutionJob::doKill()
{
    m_process->kill();
    model()->appendLine( i18n("** Killed **") );
    return true;
}

KDevelop::OutputModel* PlasmoidExecutionJob::model()
{
    return qobject_cast<KDevelop::OutputModel*>( OutputJob::model() );
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
    setErrorText(i18n("Ninja failed to compile %1", m_process->workingDirectory()));
    model()->appendLine( i18n("*** Failed ***") );
    emitResult();
}
