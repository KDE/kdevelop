/*
    SPDX-FileCopyrightText: 2025 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KDEVPLATFORM_PLUGIN_LLDB_TOOL_VIEW_FACTORY_HOLDER_H
#define KDEVPLATFORM_PLUGIN_LLDB_TOOL_VIEW_FACTORY_HOLDER_H

#include "itoolviewfactoryholder.h"

#include <util/simpletoolviewfactory.h>

namespace KDevMI {
class MIDebuggerPlugin;

namespace LLDB {
class NonInterruptDebuggerConsoleView;

/**
 * This class registers LLDB tool views in the constructor and unregisters them in the destructor.
 */
class ToolViewFactoryHolder : public IToolViewFactoryHolder
{
public:
    explicit ToolViewFactoryHolder(MIDebuggerPlugin* plugin);
    ~ToolViewFactoryHolder() override;

    [[nodiscard]] const MIDebuggerPlugin* plugin() const override;

private:
    template<class Widget, class Plugin = MIDebuggerPlugin>
    using Factory = KDevelop::SimpleGuardedDataToolViewFactory<Widget, Plugin>;

    Factory<NonInterruptDebuggerConsoleView>* m_debuggerConsoleFactory;
};

} // namespace LLDB
} // namespace KDevMI

#endif // KDEVPLATFORM_PLUGIN_LLDB_TOOL_VIEW_FACTORY_HOLDER_H
