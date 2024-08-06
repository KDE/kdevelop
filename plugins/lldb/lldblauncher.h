/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef LLDBLAUNCHER_H
#define LLDBLAUNCHER_H

#include <interfaces/ilauncher.h>

class IExecutePlugin;
namespace KDevMI { namespace LLDB {

class LldbDebuggerPlugin;
class LldbLauncher : public KDevelop::ILauncher
{
public:
    LldbLauncher(LldbDebuggerPlugin *plugin, IExecutePlugin *iexec);
    ~LldbLauncher() override;
    QList<KDevelop::LaunchConfigurationPageFactory*> configPages() const override;
    QString description() const override;
    QString id() override;
    QString name() const override;
    KJob* start(const QString &launchMode, KDevelop::ILaunchConfiguration *cfg) override;
    QStringList supportedModes() const override;

private:
    QList<KDevelop::LaunchConfigurationPageFactory*> m_factoryList;
    LldbDebuggerPlugin *m_plugin;
    IExecutePlugin *m_iexec;
};

} // end of namespace LLDB
} // end of namespace KDevMI

#endif // LLDBLAUNCHER_H
