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

#include "lldblauncher.h"

#include "debuggerplugin.h"
#include "debuglog.h"
#include "midebugjobs.h"
#include "widgets/lldbconfigpage.h"

#include <execute/iexecuteplugin.h>
#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>
#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/iruncontroller.h>
#include <util/executecompositejob.h>

#include <KLocalizedString>
#include <KMessageBox>

#include <QApplication>

using namespace KDevelop;
using namespace KDevMI;
using namespace KDevMI::LLDB;

KDevMI::LLDB::LldbLauncher::LldbLauncher(LldbDebuggerPlugin *plugin, IExecutePlugin *iexec)
    : m_plugin(plugin)
    , m_iexec(iexec)
{
    m_factoryList << new LldbConfigPageFactory();
}

QString LldbLauncher::id()
{
    return QStringLiteral("lldb");
}

QString LldbLauncher::name() const
{
    return i18n("LLDB");
}

QString LldbLauncher::description() const
{
    return i18n("Debug a native application in LLDB");
}

QStringList LldbLauncher::supportedModes() const
{
    return {"debug"};
}

QList< KDevelop::LaunchConfigurationPageFactory * > LldbLauncher::configPages() const
{
    return m_factoryList;
}

KJob *LldbLauncher::start(const QString &launchMode, KDevelop::ILaunchConfiguration *cfg)
{
    qCDebug(DEBUGGERLLDB) << "LldbLauncher: starting debugging";
    if (!cfg) {
        qCWarning(DEBUGGERLLDB) << "LldbLauncher: can't start with null configuration";
        return nullptr;
    }

    if (launchMode == "debug") {
        if (ICore::self()->debugController()->currentSession()) {
            auto ans = KMessageBox::warningYesNo(qApp->activeWindow(),
                i18n("A program is already being debugged. Do you want to abort the "
                     "currently running debug session and continue with the launch?"));
            if (ans == KMessageBox::No)
                return nullptr;
        }

        QList<KJob*> l;
        auto depJob = m_iexec->dependencyJob(cfg);
        if (depJob)
            l << depJob;
        l << new MIDebugJob(m_plugin, cfg, m_iexec);
        return new ExecuteCompositeJob(ICore::self()->runController(), l);
    }

    qCWarning(DEBUGGERLLDB) << "Unknown launch mode" << launchMode << "for config:" << cfg->name();
    return nullptr;
}
