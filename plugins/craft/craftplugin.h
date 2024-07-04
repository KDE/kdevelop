// SPDX-FileCopyrightText: 2022 Gleb Popov <arrowd@FreeBSD.org>
// SPDX-License-Identifier: BSD-3-Clause

#ifndef CRAFTPLUGIN_H
#define CRAFTPLUGIN_H

#include <interfaces/iplugin.h>

class CraftPlugin : public KDevelop::IPlugin
{
    Q_OBJECT
public:
    CraftPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& args);
};

#endif // CRAFTPLUGIN_H
