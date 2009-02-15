/* KDevelop CMake Support
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


#include "cmakejob.h"

#include "imakebuilder.h"

#include <config.h>

#include <QtCore/QStringList>
#include <QtCore/QSignalMapper>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

#include <project/projectmodel.h>

#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iplugincontroller.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <outputview/ioutputview.h>
#include <outputview/outputmodel.h>
#include <util/commandexecutor.h>
#include <QtDesigner/QExtensionFactory>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <KProcess>
#include <kjob.h>

#include "cmakeutils.h"

using namespace KDevelop;

CMakeJob::CMakeJob(QObject* parent)
    : OutputJob(parent)
    , m_project(0)
    , m_executor(0)
    , m_killed(false)
{
    setCapabilities(Killable);
}

void CMakeJob::start()
{
    kDebug(9037) << "Configuring cmake";

    if( !m_project ) {
        setError(NoProjectError);
        setErrorText("Internal error: no project specified to configure.");
        return emitResult();
    }

    setStandardToolView( KDevelop::IOutputView::BuildView );
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll );

    setModel( new KDevelop::OutputModel(this), KDevelop::IOutputView::TakeOwnership );
    startOutput();

    QString cmd = cmakeBinary( m_project );
    m_executor = new KDevelop::CommandExecutor(cmd, this);
    connect(m_executor, SIGNAL(receivedStandardError(const QStringList&)),
            model(), SLOT(appendLines(const QStringList&) ) );
    connect(m_executor, SIGNAL(receivedStandardOutput(const QStringList&)),
            model(), SLOT(appendLines(const QStringList&) ) );
    KUrl buildDirUrl = KUrl(QFileInfo(buildDir( m_project ).toLocalFile()).absoluteFilePath());
    if( !QFileInfo(buildDirUrl.toLocalFile()).exists() )
    {
        kDebug() << "creating" << buildDirUrl.fileName() << "in" << buildDirUrl.directory();
        QDir d(buildDirUrl.directory());
        d.mkdir( buildDirUrl.fileName() );
    }
    m_executor->setWorkingDirectory( buildDirUrl.toLocalFile() );
    m_executor->setArguments( cmakeArguments( m_project ) );
    connect( m_executor, SIGNAL( failed() ), this, SLOT( slotFailed() ) );
    connect( m_executor, SIGNAL( completed() ), this, SLOT( slotCompleted() ) );
    kDebug() << "Executing" << cmakeBinary( m_project ) << buildDirUrl.toLocalFile() << cmakeArguments( m_project );
    m_executor->start();
}

void CMakeJob::slotFailed()
{
    kDebug() << "job failed!";
    if (!m_killed) {
        setError(FailedError);
        // FIXME need more detail
        setErrorText(i18n("Job failed"));
    }
    emitResult();
}

void CMakeJob::slotCompleted()
{
    kDebug() << "job completed";
    emitResult();
}

bool CMakeJob::doKill()
{
    m_killed = true;
    m_executor->kill();
    return true;
}

QString CMakeJob::cmakeBinary( KDevelop::IProject* project )
{
    return CMake::currentCMakeBinary( project ).toLocalFile();
}

KUrl CMakeJob::buildDir( KDevelop::IProject* project )
{
    KUrl url = CMake::currentBuildDir( project );
    if( url.isEmpty() ) {
        url = project->folder();
    }
    return url;
}

QStringList CMakeJob::cmakeArguments( KDevelop::IProject* project )
{
    QStringList args;
    KSharedConfig::Ptr cfg = project->projectConfiguration();
    KConfigGroup group(cfg.data(), "CMake");
    args << QString("-DCMAKE_INSTALL_PREFIX=%1").arg(CMake::currentInstallDir(project).toLocalFile());
    args << QString("-DCMAKE_BUILD_TYPE=%1").arg(CMake::currentBuildType(project));
    args << project->folder().toLocalFile();
    return args;
}

void CMakeJob::setProject(KDevelop::IProject* project)
{
    m_project = project;
    
    if (m_project)
        setObjectName(i18n("CMake: %1", m_project->name()));
}

#include "cmakejob.moc"
