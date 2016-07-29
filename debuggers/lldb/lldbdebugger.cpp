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

#include <util/environmentgrouplist.h>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KShell>

#include <QApplication>
#include <QFileInfo>
#include <QRegularExpression>
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
        debuggerBinary_ = "lldb-mi";
    } else {
        debuggerBinary_ = lldbUrl.toLocalFile();
    }

    if (!checkVersion()) {
        return false;
    }

    // Get arguments
    QStringList arguments = extraArguments;
    //arguments << "-quiet";
    arguments.append(KShell::splitArgs(config.readEntry(Config::LldbArgumentsEntry, QString())));

    // Get environment
    const EnvironmentGroupList egl(config.config());
    const auto &envs = egl.variables(config.readEntry(Config::LldbEnvironmentEntry, egl.defaultGroup()));
    QProcessEnvironment processEnv;
    if (config.readEntry(Config::LldbInheritSystemEnvEntry, true)) {
        processEnv = QProcessEnvironment::systemEnvironment();
    }
    for (auto it = envs.begin(), ite = envs.end(); it != ite; ++it) {
        processEnv.insert(it.key(), it.value());
    }

    // Start!
    process_->setProcessEnvironment(processEnv);
    process_->setProgram(debuggerBinary_, arguments);
    process_->start();

    qCDebug(DEBUGGERLLDB) << "Starting LLDB with command" << debuggerBinary_ + ' ' + arguments.join(' ');
    qCDebug(DEBUGGERLLDB) << "LLDB process pid:" << process_->pid();
    emit userCommandOutput(debuggerBinary_ + ' ' + arguments.join(' ') + '\n');

    return true;
}

bool LldbDebugger::checkVersion()
{
    KProcess process;
    process.setProgram(debuggerBinary_, {"--versionLong"});
    process.setOutputChannelMode(KProcess::MergedChannels);
    process.start();
    process.waitForFinished(5000);
    auto output = QString::fromLatin1(process.readAll());
    qCDebug(DEBUGGERLLDB) << output;

    QRegularExpression rx("^Version: (\\d+).(\\d+).(\\d+).(\\d+)$", QRegularExpression::MultilineOption);
    auto match = rx.match(output);
    int version[] = {0, 0, 0, 0};
    if (match.hasMatch()) {
        for (int i = 0; i != 4; ++i) {
            version[i] = match.captured(i+1).toInt();
        }
    }

    // minimal version is 1.0.0.9
    bool ok = true;
    const int min_ver[] = {1, 0, 0, 9};
    for (int i = 0; i < 4; ++i) {
        if (version[i] < min_ver[i]) {
            ok = false;
            break;
        } else if (version[i] > min_ver[i]) {
            ok = true;
            break;
        }
    }

    if (!ok) {
        if (!qobject_cast<QGuiApplication*>(qApp))  {
            //for unittest
            qFatal("You need a graphical application.");
        }

        KMessageBox::error(
            qApp->activeWindow(),
            i18n("<b>You need lldb-mi 1.0.0.9 or higher.</b><br />"
            "You are using: %1", output),
            i18n("LLDB Error"));
    }

    return ok;
}
