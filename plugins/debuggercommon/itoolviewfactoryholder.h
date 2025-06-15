/*
    SPDX-FileCopyrightText: 2025 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_PLUGIN_DEBUGGERCOMMON_I_TOOL_VIEW_FACTORY_HOLDER_H
#define KDEVPLATFORM_PLUGIN_DEBUGGERCOMMON_I_TOOL_VIEW_FACTORY_HOLDER_H

#include <QtClassHelperMacros>

namespace KDevMI {
class MIDebuggerPlugin;

/**
 * Derived classes should register MI debugger tool views in the constructor and unregister them in the destructor.
 */
class IToolViewFactoryHolder
{
public:
    Q_DISABLE_COPY_MOVE(IToolViewFactoryHolder)
    virtual ~IToolViewFactoryHolder() = 0;

    /**
     * @return the plugin instance associated with this holder or @c nullptr if it has been destroyed
     */
    [[nodiscard]] virtual const MIDebuggerPlugin* plugin() const = 0;

protected:
    IToolViewFactoryHolder() = default;
};
}

#endif // KDEVPLATFORM_PLUGIN_DEBUGGERCOMMON_I_TOOL_VIEW_FACTORY_HOLDER_H
