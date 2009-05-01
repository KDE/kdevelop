/*  This file is part of KDevelop
    Copyright 2009 Andreas Pakulat <apaku@gmx.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef EXECUTEPLUGINCONSTANTS_H
#define EXECUTEPLUGINCONSTANTS_H

#include <QtCore/QString>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <project/projectmodel.h>

class KConfigGroup;

/**
 * Provides access to the config entry names that are being used
 * by the execute plugins launch config type.
 */
namespace ExecutePlugin
{
    static QString nativeAppConfigTypeId = "Native Application";
    static QString workingDirEntry = "Working Directory";
    static QString executableEntry = "Executable";
    static QString argumentsEntry = "Arguments";
    static QString isExecutableEntry = "isExecutable";
    static QString dependencyEntry = "Dependencies";
    static QString environmentGroupEntry = "EnvironmentGroup";
    static QString useTerminalEntry = "Use External Terminal";
    static QString userIdToRunEntry = "User Id to Run";
    static QString dependencyActionEntry = "Dependency Action";
    static QString projectTargetEntry = "Project Target";
    
    /**
     * Convenience method to fetch the executable from a config group
     */
    static QString executableFromConfig( KConfigGroup grp )
    {
        QString executable;
        if( grp.readEntry(ExecutePlugin::isExecutableEntry, false ) )
        {
            executable = grp.readEntry( ExecutePlugin::executableEntry, KUrl("") ).toLocalFile();
        } else 
        {
            QString prjitem = grp.readEntry( ExecutePlugin::projectTargetEntry, "" );
            KDevelop::ProjectModel* model = KDevelop::ICore::self()->projectController()->projectModel();
            //TODO: Need to think about escaping here and in projectitem!
            KDevelop::ProjectBaseItem* item = dynamic_cast<KDevelop::ProjectBaseItem*>( model->itemFromIndex( 
            KDevelop::ProjectModel::pathToIndex( model, prjitem.split( '/' )  ) ) );
            if( item && item->executable() )
            {
                // TODO: Need an option in the gui to choose between installed and builddir url here, currently cmake only supports builddir url
                executable = item->executable()->builtUrl().toLocalFile();
            }
        }
        return executable;
    }
    
}

#endif

