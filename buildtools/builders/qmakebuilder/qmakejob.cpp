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

#include "imakebuilder.h"

#include <config.h>

#include <QtCore/QStringList>
#include <QtCore/QSignalMapper>


#include <projectmodel.h>

#include <iproject.h>
#include <icore.h>
#include <iplugincontroller.h>
#include <ioutputview.h>
#include <outputmodel.h>
#include <commandexecutor.h>
#include <QtDesigner/QExtensionFactory>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

QMakeJob::QMakeJob(QObject *parent)
    : OutputJob(parent)
{
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
    setModel(new KDevelop::OutputModel(this), KDevelop::IOutputView::TakeOwnership);
    startOutput();

    m_item = m_project->projectItem();
    QString cmd = qmakeBinary( m_project );
    m_cmd = new KDevelop::CommandExecutor(cmd, this);
    connect(m_cmd, SIGNAL(receivedStandardError(const QStringList&)),
            m_model, SLOT(appendLines(const QStringList&) ) );
    connect(m_cmd, SIGNAL(receivedStandardOutput(const QStringList&)),
            m_model, SLOT(appendLines(const QStringList&) ) );
    m_cmd->setWorkingDirectory( m_project->folder().toLocalFile() );
    connect( m_cmd, SIGNAL( failed() ), this, SLOT( slotFailed() ) );
    connect( m_cmd, SIGNAL( completed() ), this, SLOT( slotCompleted() ) );
    m_cmd->start();
}

QString QMakeJob::qmakeBinary( KDevelop::IProject* project )
{
    KSharedConfig::Ptr cfg = project->projectConfiguration();
    KConfigGroup group(cfg.data(), "QMake Builder");
    KUrl v = group.readEntry("QMake Binary", KUrl( "file:///usr/bin/qmake" ) );
    return v.toLocalFile();
}

void QMakeJob::setProject(KDevelop::IProject* project)
{
    m_project = project;
    
    if (m_project)
        setObjectName(i18n("QMake: %1", m_project->name()));
}

void QMakeJob::slotFailed()
{
    setError(ConfigureError);
    // FIXME need more detail i guess
    setErrorText(i18n("Configure error"));
    emitResult();
}

void QMakeJob::slotCompleted()
{
    emitResult();
}

#include "qmakejob.moc"
