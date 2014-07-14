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

#include <cmakebuilderconfig.h>

#include <QtCore/QFile>
#include <QtCore/QDir>

#include <project/projectmodel.h>

#include <interfaces/iproject.h>

#include <kshell.h>
#include <kdebug.h>
#include <kjob.h>
#include <klocalizedstring.h>

#include "cmakeutils.h"

using namespace KDevelop;

CMakeJob::CMakeJob(QObject* parent)
    : OutputExecuteJob(parent)
    , m_project(0)
{
    setCapabilities( Killable );
    setFilteringStrategy( OutputModel::CompilerFilter );
    setProperties( NeedWorkingDirectory | PortableMessages | DisplayStderr | IsBuilderHint );
    setToolTitle( i18n("CMake") );
    setStandardToolView( KDevelop::IOutputView::BuildView );
    setBehaviours(KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll );
}

void CMakeJob::start()
{
    kDebug(9037) << "Configuring cmake" << workingDirectory();

    if( !m_project ) {
        setError(NoProjectError);
        setErrorText("Internal error: no project specified to configure.");
        emitResult();
        return;
    }

    QDir::temp().mkpath(workingDirectory().toLocalFile());
    CMake::updateConfig( m_project, CMake::currentBuildDirIndex(m_project) );

    OutputExecuteJob::start();
}

QString CMakeJob::cmakeBinary( KDevelop::IProject* project )
{
    return CMake::currentCMakeBinary( project ).toLocalFile();
}

KUrl CMakeJob::workingDirectory() const
{
    KUrl url = CMake::currentBuildDir( m_project );
    kDebug(9042) << "builddir: " << url;
    Q_ASSERT(!url.isEmpty() && !url.isRelative()); //We cannot get the project folder as a build directory!
    return url;
}

QStringList CMakeJob::commandLine() const
{
    QStringList args;
    args << CMake::currentCMakeBinary( m_project ).toLocalFile();
    args << "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON";

    QString installDir = CMake::currentInstallDir( m_project ).toLocalFile();
    if( !installDir.isEmpty() )
    {
        args << QString("-DCMAKE_INSTALL_PREFIX=%1").arg(installDir);
    }
    QString buildType = CMake::currentBuildType( m_project );
    if( !buildType.isEmpty() )
    {
        args << QString("-DCMAKE_BUILD_TYPE=%1").arg(buildType);
    }
    
    //if we are creating a new build directory, we'll want to specify the generator
    QDir builddir(CMake::currentBuildDir( m_project ).toLocalFile());
    if(!builddir.exists() || builddir.count()==2) {
        CMakeBuilderSettings::self()->readConfig();
        args << QString("-G") << CMakeBuilderSettings::self()->generator();
    }
    QString cmakeargs = CMake::currentExtraArguments( m_project );
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
    args << CMake::projectRoot( m_project ).toLocalFile();

    return args;
}

QString CMakeJob::environmentProfile() const
{
    return CMake::currentEnvironment( m_project );
}

void CMakeJob::setProject(KDevelop::IProject* project)
{
    m_project = project;
    
    if (m_project)
        setJobName( i18n("CMake: %1", m_project->name()) );
}

