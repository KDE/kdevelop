/*
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>
    SPDX-FileCopyrightText: 2004 Roberto Raggi <roberto@kdevelop.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "genericmanager.h"

#include <KPluginFactory>

using namespace KDevelop;

K_PLUGIN_FACTORY_WITH_JSON(GenericSupportFactory, "kdevgenericmanager.json", registerPlugin<GenericProjectManager>();)

GenericProjectManager::GenericProjectManager( QObject* parent, const QVariantList& args )
    : AbstractFileManagerPlugin( QStringLiteral( "kdevgenericmanager" ), parent, args )
{
}

#include "genericmanager.moc"
