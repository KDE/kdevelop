/* KDevelop QMake Support
 *
 * Copyright 2006-2007 Andreas Pakulat <apaku@gmx.de>
 * Copyright 2008 Hamish Rodda <rodda@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "qmakejob.h"

#include "../qmakeconfig.h"

#include "make/imakebuilder.h"

#include <interfaces/iproject.h>
#include <outputview/ioutputview.h>
#include <outputview/outputmodel.h>
#include <project/projectmodel.h>
#include <util/commandexecutor.h>

#include <kdebug.h>
#include <klocale.h>

QMakeJob::QMakeJob(QObject *parent)
    : OutputJob(parent)
    , m_killed(false)
{
    setCapabilities(Killable);
}

void QMakeJob::start()
{
    if( !m_project ) {
        setError(NoProjectError);
        setErrorText(i18n("No project specified."));
        return emitResult();
    }

    setStandardToolView(KDevelop::IOutputView::BuildView);
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll);
    setModel(new KDevelop::OutputModel);
    startOutput();

    QString cmd = QMakeConfig::qmakeBinary( m_project );
    m_cmd = new KDevelop::CommandExecutor(cmd, this);
    connect(m_cmd, SIGNAL(receivedStandardError(const QStringList&)),
            model(), SLOT(appendLines(const QStringList&) ) );
    connect(m_cmd, SIGNAL(receivedStandardOutput(const QStringList&)),
            model(), SLOT(appendLines(const QStringList&) ) );
    m_cmd->setWorkingDirectory( m_project->folder().toLocalFile() );
    connect( m_cmd, SIGNAL( failed(QProcess::ProcessError) ),
             this, SLOT( slotFailed(QProcess::ProcessError) ) );
    connect( m_cmd, SIGNAL( completed(int) ), this, SLOT( slotCompleted(int) ) );
    m_cmd->start();
}

void QMakeJob::setProject(KDevelop::IProject* project)
{
    m_project = project;
    
    if (m_project)
        setObjectName(i18n("QMake: %1", m_project->name()));
}

void QMakeJob::slotFailed(QProcess::ProcessError error)
{
    kDebug() << error;

    if (!m_killed) {
        setError(ConfigureError);
        // FIXME need more detail i guess
        setErrorText(i18n("Configure error"));
    }
    emitResult();
}

void QMakeJob::slotCompleted(int code)
{
    if( code != 0 ) {
        setError( FailedShownError );
    }
    emitResult();
}

bool QMakeJob::doKill()
{
    m_killed = true;
    m_cmd->kill();
    return true;
}

#include "qmakejob.moc"
