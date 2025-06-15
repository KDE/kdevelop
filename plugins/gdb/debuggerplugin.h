/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef GDBDEBUGGERPLUGIN_H
#define GDBDEBUGGERPLUGIN_H

#include "midebuggerplugin.h"

#include <QHash>
#include <QVariantList>

class GdbLauncher;

namespace KDevMI {
namespace GDB {

class CppDebuggerPlugin : public MIDebuggerPlugin
{
    Q_OBJECT
public:
    explicit CppDebuggerPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& = QVariantList());
    ~CppDebuggerPlugin() override;

    void unload() override;

private:
    void setupExecutePlugin(KDevelop::IPlugin* plugin, bool load);

    [[nodiscard]] MIDebugSession* createSessionObject() override;
    [[nodiscard]] ToolViewFactoryHolderPtr createToolViewFactoryHolder() override;

    QHash<KDevelop::IPlugin*, GdbLauncher*> m_launchers;
};

} // end of namespace GDB
} // end of namespace KDevMI

#endif
