/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

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
            for (auto& offer : offers) {
                if (offer->desktopEntryName() == preferredName) {
                    ptr = offer;
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

#include "moc_ipartcontroller.cpp"
