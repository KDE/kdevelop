/*
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "lldbcommand.h"

using namespace KDevMI::LLDB;
using namespace KDevMI::MI;

LldbCommand::LldbCommand(CommandType type, const QString& arguments, CommandFlags flags)
    : MICommand(type, arguments, flags)
{

}

LldbCommand::~LldbCommand()
{
}

QString LldbCommand::miCommand() const
{
    if (!overrideCmd.isEmpty()) {
        return overrideCmd;
    }

    QString command;
    bool isMI = false;

    // TODO: find alternatives to the following command which are not supported in lldb-mi
    switch(type()) {
        case BreakCommands:
            // empty command
            break;
        case BreakInfo:
            // empty command
            break;
        case BreakInsert: // in lldb-mi, '-f' must be the last option switch right before location
            command = QStringLiteral("break-insert");
            isMI = true;
            break;
        case BreakList:
            // empty command
            break;
        case BreakWatch:
            command = QStringLiteral("break set var");
            break;

        case DataListChangedRegisters:
            command = QStringLiteral("data-list-changed-registers");
            break;
        case DataReadMemory: // not implemented, deprecated
            command = QStringLiteral("data-read-memory");
            break;
        case DataWriteRegisterVariables:
            command = QStringLiteral("data-write-register-values");
            break;

        case EnableTimings:
            command = QStringLiteral("enable-timings");
            break;

        case EnvironmentDirectory:
            command = QStringLiteral("environment-directory");
            break;
        case EnvironmentPath:
            command = QStringLiteral("environment-path");
            break;
        case EnvironmentPwd:
            command = QStringLiteral("environment-pwd");
            break;

        case ExecUntil:
            // TODO: write test case for this
            command = QStringLiteral("thread until");
            break;

        case FileExecFile:
            command = QStringLiteral("file-exec-file");//"exec-file"
            break;
        case FileListExecSourceFile:
            command = QStringLiteral("file-list-exec-source-file");
            break;
        case FileListExecSourceFiles:
            command = QStringLiteral("file-list-exec-source-files");
            break;
        case FileSymbolFile:
            command = QStringLiteral("file-symbol-file");//"symbol-file"
            break;

        case GdbVersion:
            command = QStringLiteral("gdb-version");//"show version"
            break;

        case InferiorTtyShow:
            command = QStringLiteral("inferior-tty-show");
            break;

        case SignalHandle:
            command = QStringLiteral("process handle");
            break;

        case TargetDisconnect:
            command = QStringLiteral("target-disconnect");//"disconnect"
            break;
        case TargetDownload:
            command = QStringLiteral("target-download");
            break;

        case ThreadListIds:
            command = QStringLiteral("thread-list-ids");
            break;
        case ThreadSelect:
            command = QStringLiteral("thread-select");
            break;

        case TraceFind:
            command = QStringLiteral("trace-find");
            break;
        case TraceStart:
            command = QStringLiteral("trace-start");
            break;
        case TraceStop:
            command = QStringLiteral("trace-stop");
            break;

        case VarInfoNumChildren:
            command = QStringLiteral("var-info-num-children");
            break;
        case VarInfoType:
            command = QStringLiteral("var-info-type");
            break;
        case VarSetFrozen:
            command = QStringLiteral("var-set-frozen");
            break;
        case VarShowFormat:
            command = QStringLiteral("var-show-format");
            break;
        default:
            return MICommand::miCommand();
    }

    if (isMI) {
        command.prepend(QLatin1Char('-'));
    }
    return command;
}

QString LldbCommand::cmdToSend()
{
    switch (type()) {
        // -gdb-set is only partially implemented
        case GdbSet: {
            QString env_name = QStringLiteral("environment ");
            QString disassembly_flavor = QStringLiteral("disassembly-flavor ");
            if (command_.startsWith(env_name)) {
                command_.remove(0, env_name.length());
                overrideCmd = QStringLiteral("settings set target.env-vars");
            } else if (command_.startsWith(disassembly_flavor)) {
                command_.remove(0, disassembly_flavor.length());
                overrideCmd = QStringLiteral("settings set target.x86-disassembly-flavor");
            }
            break;
        }
        // find the position to insert '-f'
        case BreakInsert: {
            if (!overrideCmd.isEmpty()) {
                // already done
                break;
            }
            int p = command_.length() - 1;
            bool quoted = false;
            if (command_.at(p) == QLatin1Char('"')) {
                quoted = true; // should always be the case
            }
            --p;
            for (; p >= 0; --p) {
                // find next '"' or ' '
                if (quoted) {
                    if (command_.at(p) == QLatin1Char('"') && (p == 0 || command_.at(p - 1) != QLatin1Char('\\')))
                        break;
                } else {
                    if (command_.at(p) == QLatin1Char(' '))
                        break;
                }
            }
            if (p < 0) p = 0; // this means the command is malformated, we proceed anyway.

            // move other switches like '-d' '-c' into miCommand part
            const QStringView commandView = command_;
            overrideCmd = miCommand() + QLatin1Char(' ') + commandView.first(p);
            command_ = QLatin1String("-f ") + commandView.sliced(p);
            break;
        }
        case BreakWatch:
            if (command_.startsWith(QLatin1String("-r "))) {
                command_ = QLatin1String("-w read ") + QStringView{command_}.sliced(3);
            } else if (command_.startsWith(QLatin1String("-a "))) {
                command_ = QLatin1String("-w read_write ") + QStringView{command_}.sliced(3);
            }
            break;
        case StackListArguments:
            // some times when adding the command, the current frame is invalid,
            // but is valid at sending time
            if (command_.endsWith(QLatin1String("-1 -1"))) {
                command_.replace(QLatin1String("-1 -1"), QStringLiteral("%1 %1").arg(frame()));
            }
            break;
        default:
            break;
    }
    return MICommand::cmdToSend();
}
