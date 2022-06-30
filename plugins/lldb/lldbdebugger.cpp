/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "lldbdebugger.h"

#include "dbgglobal.h"
#include "debuglog.h"

#include <interfaces/icore.h>
#include <interfaces/iruntime.h>
#include <interfaces/iruntimecontroller.h>
#include <util/environmentprofilelist.h>

#include <KConfigGroup>
#include <kcoreaddons_version.h>
#include <KLocalizedString>
#include <KShell>

#include <QUrl>

using namespace KDevelop;
using namespace KDevMI::LLDB;
using namespace KDevMI::MI;

LldbDebugger::LldbDebugger(QObject* parent)
    : MIDebugger(parent)
{
}

LldbDebugger::~LldbDebugger()
{
}

bool LldbDebugger::start(KConfigGroup& config, const QStringList& extraArguments)
{
    // Get path to executable
    QUrl lldbUrl = config.readEntry(Config::LldbExecutableEntry, QUrl());
    if (!lldbUrl.isValid() || !lldbUrl.isLocalFile()) {
        m_debuggerExecutable = QStringLiteral("lldb-mi");
    } else {
        m_debuggerExecutable = lldbUrl.toLocalFile();
    }

    // Get arguments
    QStringList arguments = extraArguments;
    //arguments << "-quiet";
    arguments.append(KShell::splitArgs(config.readEntry(Config::LldbArgumentsEntry, QString())));

    // Get environment
    const EnvironmentProfileList egl(config.config());
    const auto &envs = egl.variables(config.readEntry(Config::LldbEnvironmentEntry, egl.defaultProfileName()));
    QProcessEnvironment processEnv;
    if (config.readEntry(Config::LldbInheritSystemEnvEntry, true)) {
        processEnv = QProcessEnvironment::systemEnvironment();
    }
    for (auto it = envs.begin(), ite = envs.end(); it != ite; ++it) {
        processEnv.insert(it.key(), it.value());
    }

    // Start!
    m_process->setProcessEnvironment(processEnv);
    m_process->setProgram(m_debuggerExecutable, arguments);
    ICore::self()->runtimeController()->currentRuntime()->startProcess(m_process);

    qCDebug(DEBUGGERLLDB) << "Starting LLDB with command" << m_debuggerExecutable + QLatin1Char(' ') + arguments.join(QLatin1Char(' '));
#if KCOREADDONS_VERSION < QT_VERSION_CHECK(5, 78, 0)
    qCDebug(DEBUGGERLLDB) << "LLDB process pid:" << m_process->pid();
#else
    qCDebug(DEBUGGERLLDB) << "LLDB process pid:" << m_process->processId();
#endif
    emit userCommandOutput(m_debuggerExecutable + QLatin1Char(' ') + arguments.join(QLatin1Char(' ')) + QLatin1Char('\n'));

    return true;
}
