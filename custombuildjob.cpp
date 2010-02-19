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

#include "custombuildjob.h"

#include <KLocale>

#include <outputview/outputmodel.h>
#include <project/projectmodel.h>

#include "custombuildsystemplugin.h"

CustomBuildJob::CustomBuildJob( CustomBuildSystem* plugin, KDevelop::ProjectBaseItem* item, Type t )
    : OutputJob( plugin ), type( t )
{
    setCapabilities( Killable );
    QString cmd;
    switch( type ) {
        case Build:
            cmd = i18n( "Building:" );
            break;
        case Clean:
            cmd = i18n( "Cleaning:" );
            break;
        case Install:
            cmd = i18n( "Installing:" );
            break;
        case Configure:
            cmd = i18n( "Configuring:" );
            break;
    }
    setTitle( QString("%1 %2").arg( cmd ).arg( item->text() ) );
    setObjectName( QString("%1 %2").arg( cmd ).arg( item->text() ) );
}

void CustomBuildJob::start()
{
    setStandardToolView( KDevelop::IOutputView::BuildView );
    setBehaviours( KDevelop::IOutputView::AllowUserClose | KDevelop::IOutputView::AutoScroll );
    KDevelop::OutputModel* model = new KDevelop::OutputModel( this );
    setModel( model, KDevelop::IOutputView::TakeOwnership );
    startOutput();
    model->appendLine( QString(">%1").arg( objectName() ) );
    emitResult();
}

#include "custombuildjob.moc"
