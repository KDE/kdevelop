/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef LLDB_DEBUGGERPLUGIN_H
#define LLDB_DEBUGGERPLUGIN_H

#include "midebuggerplugin.h"

#include <QHash>
#include <QVariantList>

namespace KDevMI { namespace LLDB {

class LldbLauncher;

class LldbDebuggerPlugin : public MIDebuggerPlugin
{
    Q_OBJECT
public:
    explicit LldbDebuggerPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& = QVariantList());
    ~LldbDebuggerPlugin() override;

    void unload() override;

private:
    void setupExecutePlugin(KDevelop::IPlugin* plugin, bool load);

    [[nodiscard]] MIDebugSession* createSessionObject() override;
    [[nodiscard]] ToolViewFactoryHolderPtr createToolViewFactoryHolder() override;

    QHash<KDevelop::IPlugin*, LldbLauncher*> m_launchers;
};

} // end of namespace LLDB
} // end of namespace KDevMI

#endif // LLDB_DEBUGGERPLUGIN_H
