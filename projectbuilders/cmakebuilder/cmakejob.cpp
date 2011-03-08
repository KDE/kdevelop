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
#include <kshell.h>
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
    KDevelop::OutputModel* m_model = new KDevelop::OutputModel(this);
    setModel( m_model, KDevelop::IOutputView::TakeOwnership );
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
        d.mkpath( buildDirUrl.fileName() );
    }
    m_executor->setWorkingDirectory( buildDirUrl.toLocalFile() );
    m_executor->setArguments( cmakeArguments( m_project ) );
    connect( m_executor, SIGNAL( failed(QProcess::ProcessError)), this, SLOT( slotFailed( QProcess::ProcessError ) ) );
    connect( m_executor, SIGNAL( completed() ), this, SLOT( slotCompleted() ) );
    kDebug() << "Executing" << cmakeBinary( m_project ) << buildDirUrl.toLocalFile() << cmakeArguments( m_project );
    m_model->appendLine( buildDirUrl.toLocalFile() + "> " + cmakeBinary( m_project ) + " " + cmakeArguments( m_project ).join(" ") );
    m_executor->start();
}

void CMakeJob::slotFailed( QProcess::ProcessError )
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
    kDebug(9042) << "builddir: " << url;
    Q_ASSERT(!url.isEmpty() && !url.isRelative()); //We cannot get the project folder as a build directory!
    return url;
}

QStringList CMakeJob::cmakeArguments( KDevelop::IProject* project )
{
    QStringList args;
    KUrl cmakecache = buildDir( project );
    cmakecache.addPath("CMakeCache.txt");
    if( !QFileInfo( cmakecache.toLocalFile() ).exists() )
    {
        if( !CMake::currentInstallDir(project).toLocalFile().isEmpty() ) 
        {
            args << QString("-DCMAKE_INSTALL_PREFIX=%1").arg(CMake::currentInstallDir(project).toLocalFile());
        }
        if( !CMake::currentBuildType(project).isEmpty() )
        {
            args << QString("-DCMAKE_BUILD_TYPE=%1").arg(CMake::currentBuildType(project));
        }
        if( !CMake::currentExtraArguments(project).isEmpty() ) {
            KShell::Errors err;
            QString cmakeargs = CMake::currentExtraArguments(project);
            QStringList tmp = KShell::splitArgs( cmakeargs, KShell::TildeExpand | KShell::AbortOnMeta, &err );
            if( err == KShell::NoError ) {
                args += tmp;
            } else {
                kWarning() << "Ignoring cmake Extra arguments";
                if( err == KShell::BadQuoting ) {
                    kWarning() << "CMake arguments badly quoted:" << cmakeargs;
                } else {
                    kWarning() << "CMake arguments had meta character:" << cmakeargs;
                }
            }
        }
        args << CMake::projectRoot(project).toLocalFile();
    } else 
    {
        args << ".";
    }

    return args;
}

void CMakeJob::setProject(KDevelop::IProject* project)
{
    m_project = project;
    
    if (m_project)
        setObjectName(i18n("CMake: %1", m_project->name()));
}

#include "cmakejob.moc"
