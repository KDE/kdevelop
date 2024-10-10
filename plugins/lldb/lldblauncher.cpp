/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

LldbLauncher::LldbLauncher(LldbDebuggerPlugin* plugin, IExecutePlugin* iexec)
    : m_plugin(plugin)
    , m_iexec(iexec)
{
    m_factoryList << new LldbConfigPageFactory();
}

LldbLauncher::~LldbLauncher()
{
    qDeleteAll(m_factoryList);
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
    return {QStringLiteral("debug")};
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

    if (launchMode == QLatin1String("debug")) {
        if (ICore::self()->debugController()->currentSession()) {
            auto ans = KMessageBox::warningTwoActions(
                qApp->activeWindow(),
                i18n("A program is already being debugged. Do you want to abort the "
                     "currently running debug session and continue with the launch?"),
                {}, KGuiItem(i18nc("@action:button", "Abort Current Session"), QStringLiteral("application-exit")),
                KStandardGuiItem::cancel());
            if (ans == KMessageBox::SecondaryAction)
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
