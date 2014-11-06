/* KDevelop CMake Support
 *
 * Copyright 2009 Andreas Pakulat <apaku@gmx.de>
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

#include "cmakeutils.h"

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QProcess>
#include <QTemporaryDir>

#include <kconfig.h>
#include <KLocalizedString>
#include <kconfiggroup.h>
#include <kparts/mainwindow.h>

#include <project/projectmodel.h>
#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentationcontroller.h>
#include <interfaces/iplugincontroller.h>

#include "icmakedocumentation.h"
#include "cmakebuilddirchooser.h"
#include "settings/cmakecachemodel.h"
#include "debug.h"

namespace Config
{
namespace Old
{
static const QString currentBuildDirKey = "CurrentBuildDir";
static const QString currentCMakeBinaryKey = "Current CMake Binary";
static const QString currentBuildTypeKey = "CurrentBuildType";
static const QString currentInstallDirKey = "CurrentInstallDir";
static const QString currentEnvironmentKey = "CurrentEnvironment";
static const QString currentExtraArgumentsKey = "Extra Arguments";
static const QString projectRootRelativeKey = "ProjectRootRelative";
static const QString projectBuildDirs = "BuildDirs";
}

static const QString buildDirIndexKey = "Current Build Directory Index";
static const QString buildDirOverrideIndexKey = "Temporary Build Directory Index";
static const QString buildDirCountKey = "Build Directory Count";

namespace Specific
{
static const QString buildDirPathKey = "Build Directory Path";
static const QString cmakeBinKey = "CMake Binary";
static const QString cmakeBuildTypeKey = "Build Type";
static const QString cmakeInstallDirKey = "Install Directory";
static const QString cmakeEnvironmentKey = "Environment Profile";
static const QString cmakeArgumentsKey = "Extra Arguments";
}

static const QString groupNameBuildDir = "CMake Build Directory %1";
static const QString groupName = "CMake";

} // namespace Config

namespace
{

KConfigGroup baseGroup( KDevelop::IProject* project )
{
    if (!project)
        return KConfigGroup();

    return project->projectConfiguration()->group( Config::groupName );
}

KConfigGroup buildDirGroup( KDevelop::IProject* project, int buildDirIndex )
{
    return baseGroup(project).group( Config::groupNameBuildDir.arg(buildDirIndex) );
}

bool buildDirGroupExists( KDevelop::IProject* project, int buildDirIndex )
{
    return baseGroup(project).hasGroup( Config::groupNameBuildDir.arg(buildDirIndex) );
}

int currentBuildDirIndex( KDevelop::IProject* project )
{
    KConfigGroup baseGrp = baseGroup(project);

    if ( baseGrp.hasKey( Config::buildDirOverrideIndexKey ) )
        return baseGrp.readEntry<int>( Config::buildDirOverrideIndexKey, 0 );

    else
        return baseGrp.readEntry<int>( Config::buildDirIndexKey, 0 ); // default is 0 because QString::number(0) apparently returns an empty string
}

QString readProjectParameter( KDevelop::IProject* project, const QString& key, const QString& aDefault )
{
    int buildDirIndex = currentBuildDirIndex(project);
    if (buildDirIndex >= 0)
        return buildDirGroup( project, buildDirIndex ).readEntry( key, aDefault );

    else
        return aDefault;
}

void writeProjectParameter( KDevelop::IProject* project, const QString& key, const QString& value )
{
    int buildDirIndex = currentBuildDirIndex(project);
    if (buildDirIndex >= 0)
    {
        KConfigGroup buildDirGrp = buildDirGroup( project, buildDirIndex );
        buildDirGrp.writeEntry( key, value );
    }

    else
    {
        qWarning() << "cannot write key" << key << "(" << value << ")" << "when no builddir is set!";
    }
}

void writeProjectBaseParameter( KDevelop::IProject* project, const QString& key, const QString& value )
{
    KConfigGroup baseGrp = baseGroup(project);
    baseGrp.writeEntry( key, value );
}

} // namespace

namespace CMake
{

KDevelop::Path::List resolveSystemDirs(KDevelop::IProject* project, const QStringList& dirs)
{
    const KDevelop::Path buildDir(CMake::currentBuildDir(project));
    const KDevelop::Path installDir(CMake::currentInstallDir(project));

    KDevelop::Path::List newList;
    newList.reserve(dirs.size());
    foreach(const QString& s, dirs)
    {
        KDevelop::Path dir;
        if(s.startsWith(QString::fromUtf8("#[bin_dir]")))
        {
            dir = KDevelop::Path(buildDir, s);
        }
        else if(s.startsWith(QString::fromUtf8("#[install_dir]")))
        {
            dir = KDevelop::Path(installDir, s);
        }
        else
        {
            dir = KDevelop::Path(s);
        }

//         qCDebug(CMAKE) << "resolved" << s << "to" << d;

        if (!newList.contains(dir))
        {
            newList.append(dir);
        }
    }
    return newList;
}

///NOTE: when you change this, update @c defaultConfigure in cmakemanagertest.cpp
bool checkForNeedingConfigure( KDevelop::IProject* project )
{
    const KDevelop::Path builddir = currentBuildDir(project);
    if( !builddir.isValid() )
    {
        CMakeBuildDirChooser bd;

        KDevelop::Path folder = project->path();
        QString relative=CMake::projectRootRelative(project);
        folder.cd(relative);

        bd.setSourceFolder( folder );
        bd.setAlreadyUsed( CMake::allBuildDirs(project) );
        bd.setCMakeBinary( currentCMakeBinary(project) );

        if( !bd.exec() )
        {
            return false;
        }

        QString newbuilddir = bd.buildFolder().toLocalFile();
        int addedBuildDirIndex = buildDirCount( project ); // old count is the new index

        // Initialize the kconfig items with the values from the dialog, this ensures the settings
        // end up in the config file once the changes are saved
        qCDebug(CMAKE) << "adding to cmake config: new builddir index" << addedBuildDirIndex;
        qCDebug(CMAKE) << "adding to cmake config: builddir path " << bd.buildFolder();
        qCDebug(CMAKE) << "adding to cmake config: installdir " << bd.installPrefix();
        qCDebug(CMAKE) << "adding to cmake config: extra args" << bd.extraArguments();
        qCDebug(CMAKE) << "adding to cmake config: build type " << bd.buildType();
        qCDebug(CMAKE) << "adding to cmake config: cmake binary " << bd.cmakeBinary();
        qCDebug(CMAKE) << "adding to cmake config: environment <null>";
        CMake::setBuildDirCount( project, addedBuildDirIndex + 1 );
        CMake::setCurrentBuildDirIndex( project, addedBuildDirIndex );
        CMake::setCurrentBuildDir( project, bd.buildFolder() );
        CMake::setCurrentInstallDir( project, bd.installPrefix() );
        CMake::setCurrentExtraArguments( project, bd.extraArguments() );
        CMake::setCurrentBuildType( project, bd.buildType() );
        CMake::setCurrentCMakeBinary( project, bd.cmakeBinary() );
        CMake::setCurrentEnvironment( project, QString() );

        return true;
    } else if( !QFile::exists( KDevelop::Path(builddir, "CMakeCache.txt").toLocalFile() ) ||
                //TODO: maybe we could use the builder for that?
               !(QFile::exists( KDevelop::Path(builddir, "Makefile").toLocalFile() ) ||
                    QFile::exists( KDevelop::Path(builddir, "build.ninja").toLocalFile() ) ) )
    {
        // User entered information already, but cmake hasn't actually been run yet.
        return true;
    }
    return false;
}

KDevelop::Path projectRoot(KDevelop::IProject* project)
{
    if (!project) {
        return {};
    }

    return project->path().cd(CMake::projectRootRelative(project));
}

KDevelop::Path currentBuildDir( KDevelop::IProject* project )
{
    return KDevelop::Path(readProjectParameter( project, Config::Specific::buildDirPathKey, QString() ));
}

KDevelop::Path commandsFile(KDevelop::IProject* project)
{
    auto currentBuildDir = CMake::currentBuildDir(project);
    if (currentBuildDir.isEmpty()) {
        return {};
    }

    return KDevelop::Path(currentBuildDir, QStringLiteral("compile_commands.json"));
}

KDevelop::Path projectTargetsFile(KDevelop::IProject* project)
{
    auto currentBuildDir = CMake::currentBuildDir(project);
    if (currentBuildDir.isEmpty()) {
        return {};
    }

    return KDevelop::Path(currentBuildDir, QStringLiteral("ProjectTargets.json"));
}

QString currentBuildType( KDevelop::IProject* project )
{
    return readProjectParameter( project, Config::Specific::cmakeBuildTypeKey, "Release" );
}

KDevelop::Path currentCMakeBinary( KDevelop::IProject* project )
{
    return KDevelop::Path(readProjectParameter( project, Config::Specific::cmakeBinKey,
                                                QStandardPaths::findExecutable( "cmake" ) ));
}

KDevelop::Path currentInstallDir( KDevelop::IProject* project )
{
    return KDevelop::Path(readProjectParameter( project, Config::Specific::cmakeInstallDirKey, "/usr/local" ));
}

QString projectRootRelative( KDevelop::IProject* project )
{
    return baseGroup(project).readEntry( Config::Old::projectRootRelativeKey, "." );
}

bool hasProjectRootRelative(KDevelop::IProject* project)
{
    return baseGroup(project).hasKey( Config::Old::projectRootRelativeKey );
}

QString currentExtraArguments( KDevelop::IProject* project )
{
    return readProjectParameter( project, Config::Specific::cmakeArgumentsKey, QString() );
}

void setCurrentInstallDir( KDevelop::IProject* project, const KDevelop::Path& path )
{
    writeProjectParameter( project, Config::Specific::cmakeInstallDirKey, path.toLocalFile() );
}

void setCurrentBuildType( KDevelop::IProject* project, const QString& type )
{
    writeProjectParameter( project, Config::Specific::cmakeBuildTypeKey, type );
}

void setCurrentCMakeBinary( KDevelop::IProject* project, const KDevelop::Path& path )
{
    writeProjectParameter( project, Config::Specific::cmakeBinKey, path.toLocalFile() );
}

void setCurrentBuildDir( KDevelop::IProject* project, const KDevelop::Path& path )
{
    writeProjectParameter( project, Config::Specific::buildDirPathKey, path.toLocalFile() );
}

void setProjectRootRelative( KDevelop::IProject* project, const QString& relative)
{
    writeProjectBaseParameter( project, Config::Old::projectRootRelativeKey, relative );
}

void setCurrentExtraArguments( KDevelop::IProject* project, const QString& string)
{
    writeProjectParameter( project, Config::Specific::cmakeArgumentsKey, string );
}

QString currentEnvironment(KDevelop::IProject* project)
{
    return readProjectParameter( project, Config::Specific::cmakeEnvironmentKey, QString() );
}


int currentBuildDirIndex( KDevelop::IProject* project )
{
    KConfigGroup baseGrp = baseGroup(project);

    if ( baseGrp.hasKey( Config::buildDirOverrideIndexKey ) )
        return baseGrp.readEntry<int>( Config::buildDirOverrideIndexKey, 0 );

    else
        return baseGrp.readEntry<int>( Config::buildDirIndexKey, 0 ); // default is 0 because QString::number(0) apparently returns an empty string
}

void setCurrentBuildDirIndex( KDevelop::IProject* project, int buildDirIndex )
{
    writeProjectBaseParameter( project, Config::buildDirIndexKey, QString::number (buildDirIndex) );
}

void setCurrentEnvironment( KDevelop::IProject* project, const QString& environment )
{
    writeProjectParameter( project, Config::Specific::cmakeEnvironmentKey, environment );
}

void initBuildDirConfig( KDevelop::IProject* project )
{
    int buildDirIndex = currentBuildDirIndex( project );
    if (buildDirCount(project) <= buildDirIndex )
        setBuildDirCount( project, buildDirIndex + 1 );
}

int buildDirCount( KDevelop::IProject* project )
{
    return baseGroup(project).readEntry<int>( Config::buildDirCountKey, 0 );
}

void setBuildDirCount( KDevelop::IProject* project, int count )
{
    writeProjectBaseParameter( project, Config::buildDirCountKey, QString::number(count) );
}

void removeBuildDirConfig( KDevelop::IProject* project )
{
    int buildDirIndex = currentBuildDirIndex( project );
    if ( !buildDirGroupExists( project, buildDirIndex ) )
    {
        qWarning() << "build directory config" << buildDirIndex << "to be removed but does not exist";
        return;
    }

    int bdCount = buildDirCount(project);
    setBuildDirCount( project, bdCount - 1 );
    removeOverrideBuildDirIndex( project );
    setCurrentBuildDirIndex( project, -1 );

    // move (rename) the upper config groups to keep the numbering
    // if there's nothing to move, just delete the group physically
    if (buildDirIndex + 1 == bdCount)
        buildDirGroup( project, buildDirIndex ).deleteGroup();

    else for (int i = buildDirIndex + 1; i < bdCount; ++i)
    {
        KConfigGroup src = buildDirGroup( project, i );
        KConfigGroup dest = buildDirGroup( project, i - 1 );
        dest.deleteGroup();
        src.copyTo(&dest);
        src.deleteGroup();
    }
}

void updateConfig( KDevelop::IProject* project, int buildDirIndex, CMakeCacheModel* model )
{
    if (buildDirIndex < 0)
        return;

    KConfigGroup buildDirGrp = buildDirGroup( project, buildDirIndex );
    bool deleteModel = false;
    if (!model)
    {
        KDevelop::Path cacheFilePath( buildDirGrp.readEntry( Config::Specific::buildDirPathKey, QString() ) );
        cacheFilePath.addPath("CMakeCache.txt");

        if( QFile::exists( cacheFilePath.toLocalFile() ) )
        {
            model = new CMakeCacheModel( 0, cacheFilePath );
            deleteModel = true;
        }
    }
    if (!model)
        return;

    buildDirGrp.writeEntry( Config::Specific::cmakeBinKey, model->value("CMAKE_COMMAND") );
    buildDirGrp.writeEntry( Config::Specific::cmakeInstallDirKey, model->value("CMAKE_INSTALL_PREFIX") );
    buildDirGrp.writeEntry( Config::Specific::cmakeBuildTypeKey, model->value("CMAKE_BUILD_TYPE") );
    if (deleteModel)
        delete model;
}

void attemptMigrate( KDevelop::IProject* project )
{
    if ( !baseGroup(project).hasKey( Config::Old::projectBuildDirs ) )
    {
        qCDebug(CMAKE) << "CMake settings migration: already done, exiting";
        return;
    }

    KConfigGroup baseGrp = baseGroup(project);

    KDevelop::Path buildDir( baseGrp.readEntry( Config::Old::currentBuildDirKey, QString() ) );
    int buildDirIndex = -1;
    const QStringList existingBuildDirs = baseGrp.readEntry( Config::Old::projectBuildDirs, QStringList() );
    {
        // also, find current build directory in this list (we need an index, not path)
        QString currentBuildDirCanonicalPath = QDir( buildDir.toLocalFile() ).canonicalPath();

        for( int i = 0; i < existingBuildDirs.count(); ++i )
        {
            const QString& nextBuildDir = existingBuildDirs.at(i);
            if( QDir(nextBuildDir).canonicalPath() == currentBuildDirCanonicalPath )
            {
                buildDirIndex = i;
            }
        }
    }
    int buildDirsCount = existingBuildDirs.count();

    qCDebug(CMAKE) << "CMake settings migration: existing build directories" << existingBuildDirs;
    qCDebug(CMAKE) << "CMake settings migration: build directory count" << buildDirsCount;
    qCDebug(CMAKE) << "CMake settings migration: current build directory" << buildDir << "(index" << buildDirIndex << ")";

    baseGrp.writeEntry( Config::buildDirCountKey, buildDirsCount );
    baseGrp.writeEntry( Config::buildDirIndexKey, buildDirIndex );

    for (int i = 0; i < buildDirsCount; ++i)
    {
        qCDebug(CMAKE) << "CMake settings migration: writing group" << i << ": path" << existingBuildDirs.at(i);

        KConfigGroup buildDirGrp = buildDirGroup( project, i );
        buildDirGrp.writeEntry( Config::Specific::buildDirPathKey, existingBuildDirs.at(i) );
    }

    baseGrp.deleteEntry( Config::Old::currentBuildDirKey );
    baseGrp.deleteEntry( Config::Old::currentCMakeBinaryKey );
    baseGrp.deleteEntry( Config::Old::currentBuildTypeKey );
    baseGrp.deleteEntry( Config::Old::currentInstallDirKey );
    baseGrp.deleteEntry( Config::Old::currentEnvironmentKey );
    baseGrp.deleteEntry( Config::Old::currentExtraArgumentsKey );
    baseGrp.deleteEntry( Config::Old::projectBuildDirs );
}

void setOverrideBuildDirIndex( KDevelop::IProject* project, int overrideBuildDirIndex )
{
    writeProjectBaseParameter( project, Config::buildDirOverrideIndexKey, QString::number(overrideBuildDirIndex) );
}

void removeOverrideBuildDirIndex( KDevelop::IProject* project, bool writeToMainIndex )
{
    KConfigGroup baseGrp = baseGroup(project);

    if( !baseGrp.hasKey(Config::buildDirOverrideIndexKey) )
        return;
    if( writeToMainIndex )
        baseGrp.writeEntry( Config::buildDirIndexKey, baseGrp.readEntry(Config::buildDirOverrideIndexKey) );

    baseGrp.deleteEntry(Config::buildDirOverrideIndexKey);
}

ICMakeDocumentation* cmakeDocumentation()
{
    return KDevelop::ICore::self()->pluginController()->extensionForPlugin<ICMakeDocumentation>("org.kdevelop.ICMakeDocumentation");
}

QStringList allBuildDirs(KDevelop::IProject* project)
{
    QStringList result;
    int bdCount = buildDirCount(project);
    for (int i = 0; i < bdCount; ++i)
        result += buildDirGroup( project, i ).readEntry( Config::Specific::buildDirPathKey );
    return result;
}

QString executeProcess(const QString& execName, const QStringList& args)
{
    Q_ASSERT(!execName.isEmpty());
    qCDebug(CMAKE) << "Executing:" << execName << "::" << args /*<< "into" << *m_vars*/;

    QProcess p;
    QTemporaryDir tmp("kdevcmakemanager");
    p.setWorkingDirectory( tmp.path() );
    p.start(execName, args, QIODevice::ReadOnly);

    if(!p.waitForFinished())
    {
        qCDebug(CMAKE) << "failed to execute:" << execName;
    }

    QByteArray b = p.readAllStandardOutput();
    QString t;
    t.prepend(b.trimmed());
    return t;
}

}

