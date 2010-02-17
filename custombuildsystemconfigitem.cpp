/************************************************************************
* KDevelop4 Custom Buildsystem Support                                 *
*                                                                      *
* Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
*                                                                      *
* This program is free software; you can redistribute it and/or modify *
* it under the terms of the GNU General Public License as published by *
* the Free Software Foundation; either version 3 of the License, or    *
* (at your option) any later version.                                  *
*                                                                      *
* This program is distributed in the hope that it will be useful, but  *
* WITHOUT ANY WARRANTY; without even the implied warranty of           *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     *
* General Public License for more details.                             *
*                                                                      *
* You should have received a copy of the GNU General Public License    *
* along with this program; if not, see <http://www.gnu.org/licenses/>. *
************************************************************************/

#include "custombuildsystemconfigitem.h"

#include <QDebug>

#include <KConfigGroup>

#include <KLocale>

#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <interfaces/iproject.h>

#include "custombuildsystemconfig.h"
#include "configconstants.h"

CustomBuildSystemConfigItem::CustomBuildSystemConfigItem( KDevelop::ProjectBaseItem* item )
    : QObject( 0 ), ProjectBaseItem( item->project(), "config", item ), projectitem( item )
{
    connect( KDevelop::ICore::self()->projectController(), SIGNAL(projectConfigurationChanged(KDevelop::IProject*)),
             SLOT(projectConfigChanged(KDevelop::IProject*)) );
    updateCurrentConfig( item->project() );
}

void CustomBuildSystemConfigItem::projectConfigChanged( KDevelop::IProject* project )
{
    if( project == projectitem->project() )
    {
        updateCurrentConfig( project );
    }
}

void CustomBuildSystemConfigItem::updateCurrentConfig( KDevelop::IProject* project )
{
    KConfigGroup grp = project->projectConfiguration()->group( ConfigConstants::customBuildSystemGroup );
    KConfigGroup configgrp = grp.group( grp.readEntry( ConfigConstants::currentConfigKey ) );
    setText( i18n( "Build Configuration: %1", configgrp.readEntry( ConfigConstants::configTitleKey ) ) );
}

int CustomBuildSystemConfigItem::type() const
{
    return KDevelop::ProjectBaseItem::CustomProjectItemType+1;
}

bool CustomBuildSystemConfigItem::lessThan( const KDevelop::ProjectBaseItem* ) const
{
    return true;
}

#include "custombuildsystemconfigitem.moc"
