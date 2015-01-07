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
#include <kjob.h>
#include <klocalizedstring.h>

#include "cmakeutils.h"
#include "debug.h"

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
    qCDebug(CMAKEBUILDER) << "Configuring cmake" << workingDirectory();

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

QUrl CMakeJob::workingDirectory() const
{
    KDevelop::Path path = CMake::currentBuildDir( m_project );
    qCDebug(CMAKEBUILDER) << "builddir: " << path;
    Q_ASSERT(path.isValid()); //We cannot get the project folder as a build directory!
    return path.toUrl();
}

QStringList CMakeJob::commandLine() const
{
    QStringList args;
    args << CMake::currentCMakeBinary( m_project ).toLocalFile();
    args << "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON";

    QString installDir = CMake::currentInstallDir( m_project ).toLocalFile();
    if( !installDir.isEmpty() )
    {
        args << QStringLiteral("-DCMAKE_INSTALL_PREFIX=%1").arg(installDir);
    }
    QString buildType = CMake::currentBuildType( m_project );
    if( !buildType.isEmpty() )
    {
        args << QStringLiteral("-DCMAKE_BUILD_TYPE=%1").arg(buildType);
    }
    QVariantMap cacheArgs = property("extraCMakeCacheValues").toMap();
    for( auto it = cacheArgs.constBegin(), itEnd = cacheArgs.constEnd(); it!=itEnd; ++it) {
        args << QStringLiteral("-D%1=%2").arg(it.key()).arg(it.value().toString());
    }

    //if we are creating a new build directory, we'll want to specify the generator
    QDir builddir(CMake::currentBuildDir( m_project ).toLocalFile());
    if(!builddir.exists() || builddir.count()==2) {
        CMakeBuilderSettings::self()->load();
        args << QString("-G") << CMakeBuilderSettings::self()->generator();
    }
    QString cmakeargs = CMake::currentExtraArguments( m_project );
    if( !cmakeargs.isEmpty() ) {
        KShell::Errors err;
        QStringList tmp = KShell::splitArgs( cmakeargs, KShell::TildeExpand | KShell::AbortOnMeta, &err );
        if( err == KShell::NoError ) {
            args += tmp;
        } else {
            qWarning() << "Ignoring cmake Extra arguments";
            if( err == KShell::BadQuoting ) {
                qWarning() << "CMake arguments badly quoted:" << cmakeargs;
            } else {
                qWarning() << "CMake arguments had meta character:" << cmakeargs;
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

