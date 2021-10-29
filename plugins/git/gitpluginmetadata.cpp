/*
    SPDX-FileCopyrightText: 2014 Alex Richardson <arichardosn.kde@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "gitplugin.h"

#include <KPluginFactory>


// This file only exists so that the tests can be built:
// test_git builds gitplugin.cpp again but in a different directory.
// This means that the kdevgit.json file is no longer found.
// Since the JSON metadata is not needed in the test, we simply move
// the K_PLUGIN_FACTORY_WITH_JSON to a separate file.
// TODO: use object or static library?

K_PLUGIN_FACTORY_WITH_JSON(KDevGitFactory, "kdevgit.json", registerPlugin<GitPlugin>();)

#include "gitpluginmetadata.moc"
