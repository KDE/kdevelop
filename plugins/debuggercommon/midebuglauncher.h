/*
    SPDX-FileCopyrightText: 2025 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MI_DEBUG_LAUNCHER_H
#define MI_DEBUG_LAUNCHER_H

#include <interfaces/ilauncher.h>
#include <util/owningrawpointercontainer.h>

#include <memory>

class IExecutePlugin;

namespace KDevMI {
class MIDebuggerPlugin;

class MIDebugLauncher : public KDevelop::ILauncher
{
public:
    ~MIDebugLauncher() override;

    [[nodiscard]] QStringList supportedModes() const override;
    [[nodiscard]] QList<KDevelop::LaunchConfigurationPageFactory*> configPages() const override;
    [[nodiscard]] KJob* start(const QString& launchMode, KDevelop::ILaunchConfiguration* cfg) override;

protected:
    explicit MIDebugLauncher(MIDebuggerPlugin* plugin, IExecutePlugin* execute,
                             std::unique_ptr<KDevelop::LaunchConfigurationPageFactory>&& factory);

private:
    MIDebuggerPlugin* const m_plugin;
    IExecutePlugin* const m_execute;
    const KDevelop::OwningRawPointerContainer<QList<KDevelop::LaunchConfigurationPageFactory*>> m_configPages;
    const QStringList m_supportedModes;
};

} // namespace KDevMI

#endif // MI_DEBUG_LAUNCHER_H
