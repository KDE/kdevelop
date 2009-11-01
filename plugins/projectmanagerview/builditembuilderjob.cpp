/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "builditembuilderjob.h"

#include <interfaces/iproject.h>
#include <project/projectmodel.h>
#include <project/projectbuildsetmodel.h>

BuildItemBuilderJob::BuildItemBuilderJob( KDevelop::BuilderJob::BuildType t, const QList<KDevelop::BuildItem>& items )
{
    foreach( const KDevelop::BuildItem &item, items )
    {
        KDevelop::ProjectBaseItem *it=item.findItem();
        //FIXME: should have disabled the building before
        //convert to assert or remove "if(it)" when it's done
        if(it)
            addItem( t, it );
    }

}

BuildItemBuilderJob::BuildItemBuilderJob( KDevelop::BuilderJob::BuildType t, const QList<KDevelop::ProjectBaseItem*>& items )
{
    addItems( t, items );
}

BuildItemBuilderJob::BuildItemBuilderJob( KDevelop::BuilderJob::BuildType t, const QList<KDevelop::IProject*>& projects )
{
    addProjects( t, projects );
}

#include "builditembuilderjob.moc"
