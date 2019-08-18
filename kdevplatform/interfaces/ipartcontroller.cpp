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

#include <KMimeTypeTrader>
#include <KParts/Part>
#include <KParts/ReadOnlyPart>
#include <KService>

#include <array>

namespace KDevelop {

IPartController::IPartController( QWidget* toplevel )
    : KParts::PartManager( toplevel, nullptr )
{
}


KPluginFactory* IPartController::findPartFactory ( const QString& mimetype, const QString& parttype, const QString& preferredName )
{
    // parttype may be a interface type not derived from KParts/ReadOnlyPart
    const KService::List offers = KMimeTypeTrader::self()->query( mimetype,
                                        QStringLiteral( "KParts/ReadOnlyPart" ),
                                        QStringLiteral( "'%1' in ServiceTypes" ).arg( parttype ) );
    if ( ! offers.isEmpty() )
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
                    break;
                }
            }
        }
        // else we just take the first in the list
        if ( !ptr )
        {
            ptr = offers.first();
        }
        KPluginLoader loader( ptr->library() );
        return loader.factory();
    }

    return nullptr;
}


KParts::Part* IPartController::createPart ( const QString& mimetype, const QString& prefName )
{
    static const std::array<QString, 1> services = {
        // Disable read/write parts until we can support them
        /*"KParts/ReadWritePart",*/
        QStringLiteral("KParts/ReadOnlyPart")
    };

    KParts::Part* part = nullptr;
    for (auto& service : services) {
        KPluginFactory* editorFactory = findPartFactory(mimetype, service, prefName);
        if ( editorFactory )
        {
            part = editorFactory->create<KParts::ReadOnlyPart>( nullptr, this );
            break;
        }
    }

    return part;
}


}


