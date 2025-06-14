/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef GDBDEBUGGERPLUGIN_H
#define GDBDEBUGGERPLUGIN_H

#include "midebuggerplugin.h"
#include "debugsession.h"

#include <QHash>
#include <QVariantList>

class GdbLauncher;

namespace KDevMI {
class DisassembleWidget;

namespace GDB {
class GDBOutputWidget;
class MemoryViewerWidget;

class CppDebuggerPlugin : public MIDebuggerPlugin
{
    Q_OBJECT

public:
    friend class DebugSession;

    explicit CppDebuggerPlugin(QObject* parent, const KPluginMetaData& metaData, const QVariantList& = QVariantList());
    ~CppDebuggerPlugin() override;

    void unload() override;

    void unloadToolViews() override;
    void setupToolViews() override;

private:
    void setupExecutePlugin(KDevelop::IPlugin* plugin, bool load);

    [[nodiscard]] DebugSession* createSessionObject() override;

    DebuggerToolFactory<DisassembleWidget>* disassemblefactory;
    DebuggerToolFactory<GDBOutputWidget, CppDebuggerPlugin>* gdbfactory;
    DebuggerToolFactory<MemoryViewerWidget, CppDebuggerPlugin>* memoryviewerfactory;
    QHash<KDevelop::IPlugin*, GdbLauncher*> m_launchers;
};

} // end of namespace GDB
} // end of namespace KDevMI

#endif
