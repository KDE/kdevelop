/*
 * This file is part of KDevelop
 * Copyright (C) 2017  Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "sourceformatterplugin.h"

#include "config/projectconfigpage.h"

#include <project/projectconfigpage.h>

#include <KPluginFactory>

K_PLUGIN_FACTORY_WITH_JSON(SourceFormatterPluginFactory, "kdevsourceformatter.json", registerPlugin<SourceFormatterPlugin>();)

SourceFormatterPlugin::SourceFormatterPlugin(QObject* parent, const QVariantList& args)
    : KDevelop::IPlugin(QStringLiteral("kdevsourceformatter"), parent)
{
    Q_UNUSED(args);

}

SourceFormatterPlugin::~SourceFormatterPlugin() = default;

int SourceFormatterPlugin::perProjectConfigPages() const
{
    return 1;
}

KDevelop::ConfigPage* SourceFormatterPlugin::perProjectConfigPage(int number, const KDevelop::ProjectConfigOptions& options, QWidget* parent)
{
    if (number != 0) {
        return nullptr;
    }

    return new SourceFormatter::ProjectConfigPage(this, options.project, parent);
}

// needed for QObject class created from K_PLUGIN_FACTORY_WITH_JSON
#include "sourceformatterplugin.moc"
