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

#include <kconfig.h>
#include <klocale.h>
#include <kconfiggroup.h>
#include <kurl.h>
#include <kparts/mainwindow.h>
#include <kdialog.h>
#include <kdebug.h>

#include <project/projectmodel.h>
#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>

#include "cmakebuilddirchooser.h"

namespace CMake
{

bool checkForNeedingConfigure( KDevelop::ProjectBaseItem* item )
{
    KConfigGroup cmakeGrp = item->project()->projectConfiguration()->group("CMake");
    KUrl builddir = cmakeGrp.readEntry( "CurrentBuildDir", KUrl() );
    QStringList builddirs = cmakeGrp.readEntry( "BuildDirs", QStringList() );

    if( !builddir.isValid() || builddir.isEmpty() )
    {
        KDialog choosedlg(KDevelop::ICore::self()->uiController()->activeMainWindow());
        choosedlg.setButtons( KDialog::Ok | KDialog::Cancel );
        choosedlg.setWindowTitle( i18n( "Choose CMake Build Directory" ) );
        choosedlg.resize( 600, 250 );
        CMakeBuildDirChooser bd( &choosedlg );
        bd.setSourceFolder( item->project()->folder() );
        choosedlg.setButtons( KDialog::Ok | KDialog::Cancel );
        choosedlg.setMainWidget( &bd );
        if( !choosedlg.exec() )
        {
            return false;
        }

        cmakeGrp.writeEntry( "CurrentBuildDir", bd.buildFolder() );
        cmakeGrp.writeEntry( "Current CMake Binary", bd.cmakeBinary() );
        cmakeGrp.writeEntry( "CurrentInstallDir", bd.installPrefix() );
        cmakeGrp.writeEntry( "CurrentBuildType", bd.buildType() );

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

KUrl buildDirForProject( KDevelop::IProject* project )
{
    KConfigGroup cmakeGrp = project->projectConfiguration()->group("CMake");
    return cmakeGrp.readEntry( "CurrentBuildDir", KUrl() );
}

}

