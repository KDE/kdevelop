/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ipartcontroller.h"

#include <KParts/Part>
#include <KParts/PartLoader>
#include <KParts/ReadOnlyPart>
#include <KService>

#include <array>

namespace KDevelop {

IPartController::IPartController( QWidget* toplevel )
    : KParts::PartManager( toplevel, nullptr )
{
}

KParts::Part* IPartController::createPart ( const QString& mimetype, const QString& prefName )
{
    const auto parts = KParts::PartLoader::partsForMimeType(mimetype);
    auto it = std::find_if(parts.cbegin(), parts.cend(), [prefName] (auto part) {
        return part.pluginId() == prefName;
    });
    if (it == parts.cend())
        return KParts::PartLoader::instantiatePart<KParts::ReadOnlyPart>(*it).plugin;
    return KParts::PartLoader::instantiatePartForMimeType<KParts::ReadOnlyPart>(mimetype).plugin;
}

}

#include "moc_ipartcontroller.cpp"
