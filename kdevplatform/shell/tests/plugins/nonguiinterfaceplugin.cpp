/*
 * This file is part of KDevelop
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
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

#include "nonguiinterface.h"

#include <interfaces/iplugin.h>

#include <KPluginFactory>

class NonGuiInterfacePlugin : public KDevelop::IPlugin, ITestNonGuiInterface
{
    Q_OBJECT
    Q_INTERFACES(ITestNonGuiInterface)
public:
    explicit NonGuiInterfacePlugin(QObject* parent, const QVariantList&);
};

NonGuiInterfacePlugin::NonGuiInterfacePlugin(QObject* parent, const QVariantList&)
    : IPlugin(QStringLiteral("nonguiinterfaceplugin"), parent)
{
}

K_PLUGIN_FACTORY_WITH_JSON(NonGuiInterfacePluginFactory, "nonguiinterfaceplugin.testpluginjson",
                           registerPlugin<NonGuiInterfacePlugin>();)

#include "nonguiinterfaceplugin.moc"

