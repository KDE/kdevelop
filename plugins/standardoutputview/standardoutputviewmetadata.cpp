/*
    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardson.kde@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "standardoutputview.h"

#include <KPluginFactory>

// this is split out to a separate file so that compiling the test doesn't need the json file
K_PLUGIN_FACTORY_WITH_JSON(StandardOutputViewFactory, "kdevstandardoutputview.json", registerPlugin<StandardOutputView>(); )

#include "standardoutputviewmetadata.moc"
