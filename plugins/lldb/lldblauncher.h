/*
 * LLDB Debugger Support
 * Copyright 2016  Aetf <aetf@unlimitedcodeworks.xyz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
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
