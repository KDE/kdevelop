/*
    SPDX-FileCopyrightText: 2015 Morten Danielsen Volden <mvolden2@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "perforceplugin.h"

#include <KPluginFactory>


// This file only exists so that the tests can be built:
// test_kdevperforce builds perforceplugin.cpp again but in a different directory.
// This means that the kdevperforce.json file is no longer found.
// Since the JSON metadata is not needed in the test, we simply move
// the K_PLUGIN_FACTORY_WITH_JSON to a separate file.

K_PLUGIN_FACTORY_WITH_JSON(KdevPerforceFactory, "kdevperforce.json", registerPlugin<PerforcePlugin>();)

#include "perforcepluginmetadata.moc"
