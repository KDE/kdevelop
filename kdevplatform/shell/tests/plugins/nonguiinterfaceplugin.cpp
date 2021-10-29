/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

