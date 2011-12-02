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

#include "cmakebuilddirchooser.h"
#include "icmakedocumentation.h"
#include <interfaces/idocumentationcontroller.h>
#include <interfaces/iplugincontroller.h>

static QString currentBuildDirKey = "CurrentBuildDir";
static QString currentCMakeBinaryKey = "Current CMake Binary";
static QString currentBuildTypeKey = "CurrentBuildType";
static QString currentInstallDirKey = "CurrentInstallDir";
static QString currentEnvironmentKey = "CurrentEnvironment";
static QString currentExtraArgumentsKey = "Extra Arguments";
static QString projectRootRelativeKey = "ProjectRootRelative";
static QString projectBuildDirs = "BuildDirs";

namespace CMake
{

///NOTE: when you change this, update @c defaultConfigure in cmakemanagertest.cpp
bool checkForNeedingConfigure( KDevelop::ProjectBaseItem* item )
{
    KConfigGroup cmakeGrp = item->project()->projectConfiguration()->group("CMake");
    KUrl builddir = cmakeGrp.readEntry( currentBuildDirKey, KUrl() );
    QStringList builddirs = cmakeGrp.readEntry( "BuildDirs", QStringList() );

    if( !builddir.isValid() || builddir.isEmpty() )
    {
        CMakeBuildDirChooser bd;
        
        KUrl folderUrl=item->project()->folder();
        QString relative=CMake::projectRootRelative(item->project());
        folderUrl.cd(relative);
        
        bd.setSourceFolder( folderUrl );
        if( !bd.exec() )
        {
            return false;
        }

        {   // if the buildfolder does not exist, create it
            // TODO: the whole configuration stuff has to be changed since it expects a configured cmake project
            //       creating the buildfolder alone is not enough.
            QDir buildFolder( bd.buildFolder().toLocalFile() );
            if ( !buildFolder.exists() ) {
                if ( !buildFolder.mkpath( buildFolder.absolutePath() ) ) {
                    KMessageBox::error( KDevelop::ICore::self()->uiController()->activeMainWindow(),
                                        i18n( "The build directory did not exist and could not be created." ),
                                        i18n("Error creating build directory") );
                    return false;
                }
            }
        }

        cmakeGrp.writeEntry( currentBuildDirKey, bd.buildFolder() );
        cmakeGrp.writeEntry( currentCMakeBinaryKey, bd.cmakeBinary() );
        cmakeGrp.writeEntry( currentInstallDirKey, bd.installPrefix() );
        cmakeGrp.writeEntry( currentExtraArgumentsKey, bd.extraArguments() );
        cmakeGrp.writeEntry( currentBuildTypeKey, bd.buildType() );

        if(!builddirs.contains(bd.buildFolder().toLocalFile())) {
            builddirs.append(bd.buildFolder().toLocalFile());
            cmakeGrp.writeEntry( "BuildDirs", builddirs);
        }
        cmakeGrp.sync();

        return true;
    } else if( !QFileInfo( builddir.toLocalFile() + "/CMakeCache.txt" ).exists() )
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
    KConfigGroup cmakeGrp = project->projectConfiguration()->group("CMake");
    return cmakeGrp.readEntry( currentBuildDirKey, KUrl() );
}

QString currentBuildType( KDevelop::IProject* project )
{
    KConfigGroup cmakeGrp = project->projectConfiguration()->group("CMake");
    return cmakeGrp.readEntry( currentBuildTypeKey, "Release" );
}

KUrl currentCMakeBinary( KDevelop::IProject* project )
{
    KConfigGroup cmakeGrp = project->projectConfiguration()->group("CMake");
    return cmakeGrp.readEntry( currentCMakeBinaryKey, KStandardDirs::findExe( "cmake" ) );
}

KUrl currentInstallDir( KDevelop::IProject* project )
{
    KConfigGroup cmakeGrp = project->projectConfiguration()->group("CMake");
    return cmakeGrp.readEntry( currentInstallDirKey, KUrl("/usr/local") );
}

QString projectRootRelative( KDevelop::IProject* project )
{
    KConfigGroup cmakeGrp = project->projectConfiguration()->group("CMake");
    return cmakeGrp.readEntry( projectRootRelativeKey, QString());
}

QString currentExtraArguments( KDevelop::IProject* project )
{
    KConfigGroup cmakeGrp = project->projectConfiguration()->group("CMake");
    return cmakeGrp.readEntry( currentExtraArgumentsKey, "");
}

void setCurrentInstallDir( KDevelop::IProject* project, const KUrl& url )
{
    KConfigGroup cmakeGrp = project->projectConfiguration()->group("CMake");
    cmakeGrp.writeEntry( currentInstallDirKey, url );
    cmakeGrp.sync();
}

void setCurrentBuildType( KDevelop::IProject* project, const QString& type )
{
    KConfigGroup cmakeGrp = project->projectConfiguration()->group("CMake");
    cmakeGrp.writeEntry( currentBuildTypeKey, type );
    cmakeGrp.sync();
}

void setCurrentCMakeBinary( KDevelop::IProject* project, const KUrl& url )
{
    KConfigGroup cmakeGrp = project->projectConfiguration()->group("CMake");
    cmakeGrp.writeEntry( currentCMakeBinaryKey, url );
    cmakeGrp.sync();
}

void setCurrentBuildDir( KDevelop::IProject* project, const KUrl& url )
{
    KConfigGroup cmakeGrp = project->projectConfiguration()->group("CMake");
    cmakeGrp.writeEntry( currentBuildDirKey, url );
    cmakeGrp.sync();
}

void setProjectRootRelative( KDevelop::IProject* project, const QString& relative)
{
    KConfigGroup cmakeGrp = project->projectConfiguration()->group("CMake");
    cmakeGrp.writeEntry( projectRootRelativeKey, relative );
    cmakeGrp.sync();
}

void setCurrentExtraArguments( KDevelop::IProject* project, const QString& string)
{
    KConfigGroup cmakeGrp = project->projectConfiguration()->group("CMake");
    cmakeGrp.writeEntry( currentExtraArgumentsKey, string );
    cmakeGrp.sync();
}

QString currentEnvironment(KDevelop::IProject* project)
{
    KConfigGroup cmakeGrp = project->projectConfiguration()->group("CMake");
    return cmakeGrp.readEntry(currentEnvironmentKey, QString());
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
    KConfigGroup cmakeGrp = project->projectConfiguration()->group("CMake");
    return cmakeGrp.readEntry( projectBuildDirs, QStringList() );
}

}

