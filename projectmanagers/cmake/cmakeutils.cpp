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

#include <kconfig.h>
#include <klocale.h>
#include <kconfiggroup.h>
#include <kurl.h>
#include <kparts/mainwindow.h>
#include <kdialog.h>
#include <kdebug.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <KMessageBox>

#include <project/projectmodel.h>
#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>

#include "icmakedocumentation.h"
#include "cmakebuilddirchooser.h"
#include "settings/cmakecachemodel.h"
#include <interfaces/idocumentationcontroller.h>
#include <interfaces/iplugincontroller.h>

namespace Config
{
namespace Old
{
static QString currentBuildDirKey = "CurrentBuildDir";
static QString currentCMakeBinaryKey = "Current CMake Binary";
static QString currentBuildTypeKey = "CurrentBuildType";
static QString currentInstallDirKey = "CurrentInstallDir";
static QString currentEnvironmentKey = "CurrentEnvironment";
static QString currentExtraArgumentsKey = "Extra Arguments";
static QString projectRootRelativeKey = "ProjectRootRelative";
static QString projectBuildDirs = "BuildDirs";
static QString cmakeDirectory = "CMakeDir";
}

static QString buildDirIndexKey = "Current Build Directory Index";
static QString buildDirOverrideIndexKey = "Temporary Build Directory Index";
static QString buildDirCountKey = "Build Directory Count";

namespace Specific
{
static QString buildDirPathKey = "Build Directory Path";
static QString cmakeBinKey = "CMake Binary";
static QString cmakeBuildTypeKey = "Build Type";
static QString cmakeInstallDirKey = "Install Directory";
static QString cmakeEnvironmentKey = "Environment Profile";
static QString cmakeArgumentsKey = "Extra Arguments";
}

static QString groupNameBuildDir = "CMake Build Directory %1";
static QString groupName = "CMake";

} // namespace Config

namespace
{

KConfigGroup baseGroup( KDevelop::IProject* project )
{
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
        buildDirGrp.sync();
    }

    else
    {
        kWarning() << "cannot write key" << key << "(" << value << ")" << "when no builddir is set!";
    }
}

void writeProjectBaseParameter( KDevelop::IProject* project, const QString& key, const QString& value )
{
    KConfigGroup baseGrp = baseGroup(project);
    baseGrp.writeEntry( key, value );
    baseGrp.sync();
}

} // namespace

namespace CMake
{

///NOTE: when you change this, update @c defaultConfigure in cmakemanagertest.cpp
bool checkForNeedingConfigure( KDevelop::ProjectBaseItem* item )
{
    KDevelop::IProject* project = item->project();
    KUrl builddir = currentBuildDir(project);
    if( !builddir.isValid() )
    {
        CMakeBuildDirChooser bd;
        
        KUrl folderUrl=item->project()->folder();
        QString relative=CMake::projectRootRelative(item->project());
        folderUrl.cd(relative);
        
        bd.setSourceFolder( folderUrl );
        bd.setAlreadyUsed( CMake::allBuildDirs(project) );
        bd.setCMakeBinary(KStandardDirs::findExe("cmake"));

        if( !bd.exec() )
        {
            return false;
        }

        QString newbuilddir = bd.buildFolder().toLocalFile( KUrl::RemoveTrailingSlash );
        int addedBuildDirIndex = buildDirCount( project ); // old count is the new index

        // Initialize the kconfig items with the values from the dialog, this ensures the settings
        // end up in the config file once the changes are saved
        kDebug( 9042 ) << "adding to cmake config: new builddir index" << addedBuildDirIndex;
        kDebug( 9042 ) << "adding to cmake config: builddir path " << bd.buildFolder().url();
        kDebug( 9042 ) << "adding to cmake config: installdir " << bd.installPrefix().url();
        kDebug( 9042 ) << "adding to cmake config: extra args" << bd.extraArguments();
        kDebug( 9042 ) << "adding to cmake config: build type " << bd.buildType();
        kDebug( 9042 ) << "adding to cmake config: cmake binary " << bd.cmakeBinary().url();
        kDebug( 9042 ) << "adding to cmake config: environment <null>";
        CMake::setBuildDirCount( project, addedBuildDirIndex + 1 );
        CMake::setCurrentBuildDirIndex( project, addedBuildDirIndex );
        CMake::setCurrentBuildDir( project, bd.buildFolder() );
        CMake::setCurrentInstallDir( project, bd.installPrefix() );
        CMake::setCurrentExtraArguments( project, bd.extraArguments() );
        CMake::setCurrentBuildType( project, bd.buildType() );
        CMake::setCurrentCMakeBinary( project, bd.cmakeBinary() );
        CMake::setCurrentEnvironment( project, QString() );

        return true;
    } else if( !QFileInfo( builddir.toLocalFile() + "/CMakeCache.txt" ).exists() ||
               !QFileInfo( builddir.toLocalFile() + "/Makefile" ).exists()
    )
    {
        // User entered information already, but cmake hasn't actually been run yet.
        return true;
    } 
    return false;
}

KUrl projectRoot(KDevelop::IProject* project)
{
    KUrl projectPath = project->folder();
    bool correct=projectPath.cd(CMake::projectRootRelative(project));
    Q_ASSERT(correct);
    return projectPath;
}

KUrl currentBuildDir( KDevelop::IProject* project )
{
    return readProjectParameter( project, Config::Specific::buildDirPathKey, QString() );
}

QString currentBuildType( KDevelop::IProject* project )
{
    return readProjectParameter( project, Config::Specific::cmakeBuildTypeKey, "Release" );
}

KUrl currentCMakeBinary( KDevelop::IProject* project )
{
    return readProjectParameter( project, Config::Specific::cmakeBinKey, KStandardDirs::findExe( "cmake" ) );
}

KUrl currentInstallDir( KDevelop::IProject* project )
{
    return readProjectParameter( project, Config::Specific::cmakeInstallDirKey, "/usr/local" );
}

QString projectRootRelative( KDevelop::IProject* project )
{
    return baseGroup(project).readEntry( Config::Old::projectRootRelativeKey, "." );
}

QString currentExtraArguments( KDevelop::IProject* project )
{
    return readProjectParameter( project, Config::Specific::cmakeArgumentsKey, QString() );
}

void setCurrentInstallDir( KDevelop::IProject* project, const KUrl& url )
{
    writeProjectParameter( project, Config::Specific::cmakeInstallDirKey, url.url() );
}

void setCurrentBuildType( KDevelop::IProject* project, const QString& type )
{
    writeProjectParameter( project, Config::Specific::cmakeBuildTypeKey, type );
}

void setCurrentCMakeBinary( KDevelop::IProject* project, const KUrl& url )
{
    writeProjectParameter( project, Config::Specific::cmakeBinKey, url.url() );
}

void setCurrentBuildDir( KDevelop::IProject* project, const KUrl& url )
{
    writeProjectParameter( project, Config::Specific::buildDirPathKey, url.url() );
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

KUrl cmakeDirectory( KDevelop::IProject* project )
{
    return baseGroup(project).readEntry( Config::Old::cmakeDirectory, QString() );
}

void setCmakeDirectory( KDevelop::IProject* project, const KUrl& url )
{
    writeProjectBaseParameter( project, Config::Old::cmakeDirectory, url.url() );
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
        kWarning() << "build directory config" << buildDirIndex << "to be removed but does not exist";
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

    project->projectConfiguration()->sync();
}

void updateConfig( KDevelop::IProject* project, int buildDirIndex, CMakeCacheModel* model )
{
    if (buildDirIndex < 0)
        return;

    KConfigGroup buildDirGrp = buildDirGroup( project, buildDirIndex );
    bool deleteModel = false;
    if (!model)
    {
        KUrl cacheFilePath( buildDirGrp.readEntry( Config::Specific::buildDirPathKey, QString() ) );
        cacheFilePath.addPath("CMakeCache.txt");

        if( QFile::exists( cacheFilePath.toLocalFile() ) )
        {
            model = new CMakeCacheModel( 0, cacheFilePath );
            deleteModel = true;
        }
    }
    if (!model)
        return;

    buildDirGrp.writeEntry( Config::Specific::cmakeBinKey, KUrl( model->value("CMAKE_COMMAND") ).url() );
    buildDirGrp.writeEntry( Config::Specific::cmakeInstallDirKey, KUrl( model->value("CMAKE_INSTALL_PREFIX") ).url() );
    buildDirGrp.writeEntry( Config::Specific::cmakeBuildTypeKey, model->value("CMAKE_BUILD_TYPE") );
    buildDirGrp.sync();
    if (deleteModel)
        delete model;
}

void attemptMigrate( KDevelop::IProject* project )
{
    if ( !baseGroup(project).hasKey( Config::Old::projectBuildDirs ) )
    {
        kDebug() << "CMake settings migration: already done, exiting";
        return;
    }

    KConfigGroup baseGrp = baseGroup(project);

    KUrl buildDir( baseGrp.readEntry( Config::Old::currentBuildDirKey, QString() ) );
    int buildDirIndex = -1;
    QStringList existingBuildDirs;
    {
        // the directories are originally stored in a path list, so need to convert them to URLs
        QStringList existingBuildDirPathes = baseGrp.readEntry( Config::Old::projectBuildDirs, QStringList() );

        // also, find current build directory in this list (we need an index, not path)
        QString currentBuildDirCanonicalPath = QDir( buildDir.toLocalFile() ).canonicalPath();

        for( int i = 0; i < existingBuildDirPathes.count(); ++i )
        {
            const QString& nextBuildDir = existingBuildDirPathes.at(i);
            existingBuildDirs += KUrl(nextBuildDir).url();
            if( QDir(nextBuildDir).canonicalPath() == currentBuildDirCanonicalPath )
            {
                buildDirIndex = i;
            }
        }
    }
    int buildDirsCount = existingBuildDirs.count();

    kDebug() << "CMake settings migration: existing build directories" << existingBuildDirs;
    kDebug() << "CMake settings migration: build directory count" << buildDirsCount;
    kDebug() << "CMake settings migration: current build directory" << buildDir << "(index" << buildDirIndex << ")";

    baseGrp.writeEntry( Config::buildDirCountKey, buildDirsCount );
    baseGrp.writeEntry( Config::buildDirIndexKey, buildDirIndex );

    for (int i = 0; i < buildDirsCount; ++i)
    {
        kDebug() << "CMake settings migration: writing group" << i << ": path" << existingBuildDirs.at(i);

        KConfigGroup buildDirGrp = buildDirGroup( project, i );
        buildDirGrp.writeEntry( Config::Specific::buildDirPathKey, KUrl( existingBuildDirs.at(i) ).url() );
    }

    baseGrp.deleteEntry( Config::Old::currentBuildDirKey );
    baseGrp.deleteEntry( Config::Old::currentCMakeBinaryKey );
    baseGrp.deleteEntry( Config::Old::currentBuildTypeKey );
    baseGrp.deleteEntry( Config::Old::currentInstallDirKey );
    baseGrp.deleteEntry( Config::Old::currentEnvironmentKey );
    baseGrp.deleteEntry( Config::Old::currentExtraArgumentsKey );
    baseGrp.deleteEntry( Config::Old::projectBuildDirs );
    baseGrp.sync();
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
    baseGrp.sync();
}

ICMakeDocumentation* cmakeDocumentation()
{
    ICMakeDocumentation* p=KDevelop::ICore::self()->pluginController()->extensionForPlugin<ICMakeDocumentation>("org.kdevelop.ICMakeDocumentation");
    
    if( !p ) 
    {
        return 0;
    }

    Q_ASSERT(p);
    return p;
}

QStringList allBuildDirs(KDevelop::IProject* project)
{
    QStringList result;
    int bdCount = buildDirCount(project);
    for (int i = 0; i < bdCount; ++i)
        result += KUrl( buildDirGroup( project, i ).readEntry( Config::Specific::buildDirPathKey ) ).toLocalFile(KUrl::RemoveTrailingSlash);
    return result;
}

}

