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

#include <config.h>
#include <cmakebuilderconfig.h>

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
#include <util/environmentgrouplist.h>

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
        emitResult();
        return;
    }

    CMake::updateConfig( m_project, CMake::currentBuildDirIndex(m_project) );

    setStandardToolView( KDevelop::IOutputView::BuildView );
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll );
    KDevelop::OutputModel* model = new KDevelop::OutputModel;
    setModel( model );
    startOutput();

    QString cmd = cmakeBinary( m_project );
    m_executor = new KDevelop::CommandExecutor(cmd, this);
    connect(m_executor, SIGNAL(receivedStandardError(QStringList)),
            model, SLOT(appendLines(QStringList)) );
    connect(m_executor, SIGNAL(receivedStandardOutput(QStringList)),
            model, SLOT(appendLines(QStringList)) );
    KUrl buildDirUrl = KUrl(QFileInfo(buildDir( m_project ).toLocalFile()).absoluteFilePath());
    if( !QFileInfo(buildDirUrl.toLocalFile()).exists() )
    {
        kDebug() << "creating" << buildDirUrl.fileName() << "in" << buildDirUrl.directory();
        QDir d(buildDirUrl.directory());
        d.mkpath( buildDirUrl.fileName() );
    }
    m_executor->setWorkingDirectory( buildDirUrl.toLocalFile() );
    m_executor->setArguments( cmakeArguments( m_project ) );
    m_executor->setEnvironment(buildEnvironment());
    connect( m_executor, SIGNAL(failed(QProcess::ProcessError)), this, SLOT(slotFailed(QProcess::ProcessError)) );
    connect( m_executor, SIGNAL(completed()), this, SLOT(slotCompleted()) );
    kDebug() << "Executing" << m_executor->command() << buildDirUrl.toLocalFile() << m_executor->arguments();
    model->appendLine( buildDirUrl.toLocalFile() + "> " + m_executor->command() + " " + m_executor->arguments().join(" "));
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
    QString installDir = CMake::currentInstallDir(project).toLocalFile();
    if( !installDir.isEmpty() )
    {
        args << QString("-DCMAKE_INSTALL_PREFIX=%1").arg(installDir);
    }
    QString buildType = CMake::currentBuildType(project);
    if( !buildType.isEmpty() )
    {
        args << QString("-DCMAKE_BUILD_TYPE=%1").arg(buildType);
    }
    
    //if we are creating a new build directory, we'll want to specify the generator
    QDir builddir(CMake::currentBuildDir(project).toLocalFile());
    if(!builddir.exists() || builddir.count()==2) {
        CMakeBuilderSettings::self()->readConfig();
        args << QString("-G") << CMakeBuilderSettings::self()->generator();
    }
    QString cmakeargs = CMake::currentExtraArguments(project);
    if( !cmakeargs.isEmpty() ) {
        KShell::Errors err;
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

    return args;
}

void CMakeJob::setProject(KDevelop::IProject* project)
{
    m_project = project;
    
    if (m_project)
        setObjectName(i18n("CMake: %1", m_project->name()));
}

QStringList CMakeJob::buildEnvironment()
{
    QString profile = CMake::currentEnvironment(m_project);

    const KDevelop::EnvironmentGroupList l(KGlobal::config());
    QStringList env = QProcess::systemEnvironment();
    env.append( "LC_MESSAGES=C" );
    return l.createEnvironment( profile, env );
}

#include "cmakejob.moc"
