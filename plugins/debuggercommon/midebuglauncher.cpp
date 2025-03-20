/*
    SPDX-FileCopyrightText: 2006 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Andreas Pakulat <apaku@gmx.de>
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>
    SPDX-FileCopyrightText: 2025 Igor Kushnir <igorkuo@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "midebuglauncher.h"

#include "midebugjobs.h"

#include <debuglog.h>

#include <execute/iexecutepluginhelpers.h>

#include <interfaces/icore.h>
#include <interfaces/idebugcontroller.h>
#include <interfaces/ilaunchconfiguration.h>
#include <interfaces/iuicontroller.h>
#include <interfaces/launchconfigurationpage.h>

#include <KLocalizedString>
#include <KMessageBox>
#include <KParts/MainWindow>

using namespace KDevMI;

MIDebugLauncher::MIDebugLauncher(MIDebuggerPlugin* plugin, IExecutePlugin* execute,
                                 std::unique_ptr<KDevelop::LaunchConfigurationPageFactory>&& factory)
    : m_plugin(plugin)
    , m_execute(execute)
    , m_configPages{{factory.release()}}
    , m_supportedModes{QStringLiteral("debug")}
{
    Q_ASSERT(m_plugin);
    Q_ASSERT(m_execute);
}

MIDebugLauncher::~MIDebugLauncher() = default;

QStringList MIDebugLauncher::supportedModes() const
{
    return m_supportedModes;
}

QList<KDevelop::LaunchConfigurationPageFactory*> MIDebugLauncher::configPages() const
{
    return *m_configPages;
}

KJob* MIDebugLauncher::start(const QString& launchMode, KDevelop::ILaunchConfiguration* cfg)
{
    Q_ASSERT(cfg);
    if (launchMode != m_supportedModes.constFirst()) {
        qCWarning(DEBUGGERCOMMON) << "unknown launch mode" << launchMode << "for config" << cfg->name();
        return nullptr;
    }
    qCDebug(DEBUGGERCOMMON) << "MIDebugLauncher: starting debugging";

    if (KDevelop::ICore::self()->debugController()->currentSession()) {
        const auto answer = KMessageBox::warningTwoActions(
            KDevelop::ICore::self()->uiController()->activeMainWindow(),
            i18n("A program is already being debugged. Do you want to abort the "
                 "currently running debug session and continue with the launch?"),
            {}, KGuiItem(i18nc("@action:button", "Abort Current Session"), QStringLiteral("application-exit")),
            KStandardGuiItem::cancel());
        if (answer == KMessageBox::SecondaryAction) {
            return nullptr;
        }
    }

    auto* const debugJob = new MIDebugJob(m_plugin, cfg, m_execute);
    return makeJobWithDependency(debugJob, *m_execute, cfg);
}
