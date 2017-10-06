/*
 * This file is part of KDevelop
 *
 * Copyright 2017 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <interfaces/iplugin.h>

#include <KPluginFactory>

class GlobalNonDefaultPlugin : public KDevelop::IPlugin
{
    Q_OBJECT
public:
    explicit GlobalNonDefaultPlugin(QObject* parent, const QVariantList&);
};

GlobalNonDefaultPlugin::GlobalNonDefaultPlugin(QObject* parent, const QVariantList&)
    : IPlugin(QStringLiteral("globalnondefaultplugin"), parent)
{
}

K_PLUGIN_FACTORY_WITH_JSON(GlobalNonDefaultPluginFactory, "globalnondefaultplugin.testpluginjson",
                           registerPlugin<GlobalNonDefaultPlugin>();)

#include "globalnondefaultplugin.moc"
