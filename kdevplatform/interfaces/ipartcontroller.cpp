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

namespace {
KPluginFactory* findPartFactory(const QString& mimetype, const QString& preferredName)
{
    const KService::List offers = KMimeTypeTrader::self()->query(mimetype, QStringLiteral("KParts/ReadOnlyPart"));
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
}

IPartController::IPartController(QWidget* toplevel)
    : KParts::PartManager(toplevel, nullptr)
{
}

KParts::Part* IPartController::createPart ( const QString& mimetype, const QString& prefName )
{
    if (auto* editorFactory = findPartFactory(mimetype, prefName)) {
        return editorFactory->create<KParts::ReadOnlyPart>(nullptr, this);
    }

    return nullptr;
}
}

#include "moc_ipartcontroller.cpp"
