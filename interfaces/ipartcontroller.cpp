/***************************************************************************
 *   Copyright 2007 Alexander Dymo <adymo@kdevelop.org>                    *
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
#include "ipartcontroller.h"
#include <klibloader.h>
#include <QFile>
#include <kservice.h>
#include <kmimetypetrader.h>
#include <kparts/factory.h>
#include <kmimetype.h>
#include <kparts/part.h>

namespace KDevelop {
    
IPartController::IPartController( QWidget* toplevel ) 
    : KParts::PartManager( toplevel, 0 )
{
}


KParts::Factory* IPartController::findPartFactory ( const QString& mimetype, const QString& parttype, const QString& preferredName ) {
    
    KService::List offers = KMimeTypeTrader::self() ->query(
                                        mimetype,
                                        "KParts/ReadOnlyPart",
                                        QString( "'%1' in ServiceTypes" ).arg( parttype ) );
                            
    if ( offers.count() > 0 )
    {
        KService::Ptr ptr;
        // if there is a preferred plugin we'll take it
        if ( !preferredName.isEmpty() )
        {
            KService::List::ConstIterator it;
            for ( it = offers.constBegin(); it != offers.constEnd(); ++it )
            {
                if ( ( *it ) ->desktopEntryName() == preferredName )
                {
                    ptr = ( *it );
                }
            }
        }
        // else we just take the first in the list
        if ( !ptr )
        {
            ptr = offers.first();
        }
        KParts::Factory *factory = static_cast<KParts::Factory*>    (
                        KLibLoader::self()->factory(
                        QFile::encodeName( ptr->library() ) ) );
        return factory;
    }
                                                                                                  
    return 0;
}


KParts::Part* IPartController::createPart ( const QString& mimetype, const QString& prefName )
{
    static const char* const services[] =
    {
        "KParts/ReadWritePart", "KParts/ReadOnlyPart"
    };

    static const char* const classNames[] =
    {
        "KParts::ReadWritePart", "KParts::ReadOnlyPart"
    };

    KParts::Factory* editorFactory;
    const char* className;
    for ( uint i = 0; i < 2; ++i )
    {
        editorFactory = findPartFactory( mimetype, QString::fromLatin1(services[ i ]), prefName );
        if ( editorFactory )
        {
            className = classNames[ i ];
            break;
        }
    }

    if ( editorFactory )
    {
        KParts::Part * part = editorFactory->createPart(
        0, this, className );

        return part;
    }

    return 0;
}


}

#include "ipartcontroller.moc"

