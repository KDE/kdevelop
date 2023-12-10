/*
    SPDX-FileCopyrightText: 2007 Alexander Dymo <adymo@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ipartcontroller.h"

#include <KParts/Part>
#include <KParts/PartLoader>
#include <KParts/ReadOnlyPart>
#include <KPluginFactory>

#include <algorithm>
#include <array>

namespace KDevelop {

namespace {
KPluginFactory* findPartFactory(const QString& mimetype, const QString& preferredName)
{
    const auto parts = KParts::PartLoader::partsForMimeType(mimetype);
    if (parts.isEmpty()) {
        return nullptr;
    }

    auto it = parts.end();

    // if there is a preferred plugin we'll take it
    if (!preferredName.isEmpty()) {
        it = std::find_if(parts.begin(), parts.end(), [&preferredName](const KPluginMetaData& part) {
            return part.pluginId() == preferredName;
        });
    }

    // otherwise use the first available part by default
    if (it == parts.end()) {
        it = parts.begin();
    }

    return KPluginFactory::loadFactory(*it).plugin;
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
