/*
    SPDX-FileCopyrightText: 2025 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_GDB_TOOL_VIEW_FACTORY_HOLDER_H
#define KDEVPLATFORM_PLUGIN_GDB_TOOL_VIEW_FACTORY_HOLDER_H

#include "itoolviewfactoryholder.h"

#include <util/simpletoolviewfactory.h>

namespace KDevMI {
class DisassembleWidget;
class MIDebuggerPlugin;

namespace GDB {
class CppDebuggerPlugin;
class GDBOutputWidget;
class MemoryViewerWidget;

/**
 * This class registers GDB tool views in the constructor and unregisters them in the destructor.
 */
class ToolViewFactoryHolder : public IToolViewFactoryHolder
{
public:
    explicit ToolViewFactoryHolder(CppDebuggerPlugin* plugin);
    ~ToolViewFactoryHolder() override;

    [[nodiscard]] const MIDebuggerPlugin* plugin() const override;

private:
    template<class Widget, class Plugin = MIDebuggerPlugin>
    using Factory = KDevelop::SimpleGuardedDataToolViewFactory<Widget, Plugin>;

    Factory<DisassembleWidget>* m_disassembleFactory;
    Factory<GDBOutputWidget, CppDebuggerPlugin>* m_gdbOutputFactory;
    Factory<MemoryViewerWidget, CppDebuggerPlugin>* m_memoryViewerFactory;
};

} // namespace GDB
} // namespace KDevMI

#endif // KDEVPLATFORM_PLUGIN_GDB_TOOL_VIEW_FACTORY_HOLDER_H
