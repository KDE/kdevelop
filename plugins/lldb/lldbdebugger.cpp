/*
 * Low level LLDB interface.
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

#include "lldbdebugger.h"

#include "dbgglobal.h"
#include "debuglog.h"

#include <util/environmentprofilelist.h>

#include <KConfigGroup>
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
    m_process->start();

    qCDebug(DEBUGGERLLDB) << "Starting LLDB with command" << m_debuggerExecutable + QLatin1Char(' ') + arguments.join(QLatin1Char(' '));
    qCDebug(DEBUGGERLLDB) << "LLDB process pid:" << m_process->pid();
    emit userCommandOutput(m_debuggerExecutable + QLatin1Char(' ') + arguments.join(QLatin1Char(' ')) + QLatin1Char('\n'));

    return true;
}
