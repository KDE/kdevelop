/*
    SPDX-FileCopyrightText: 2024 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGINTESTHELPERS_H
#define KDEVPLATFORM_PLUGINTESTHELPERS_H

#include "testsexport.h"

class KPluginMetaData;

class QString;

namespace KDevelop {

/**
 * Creates a KPluginMetaData instance with just the "name" and "pluginId"
 * properties set and and no file origin.
 * @param name name of the plugin, used lowercased also as plugin id
 */
KDEVPLATFORMTESTS_EXPORT
KPluginMetaData makeTestPluginMetaData(const QString& name);

}

#endif
