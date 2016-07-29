/*
 * LLDB specific version of MI command
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
            command = "";
            break;
        case BreakInfo:
            command = "";
            break;
        case BreakInsert: // in lldb-mi, '-f' must be the last option switch right before location
            command = "break-insert";
            isMI = true;
            break;
        case BreakList:
            command = "";
            break;
        case BreakWatch:
            command = "break set var";
            break;

        case DataListChangedRegisters:
            command = "data-list-changed-registers";
            break;
        case DataReadMemory: // not implemented, deprecated
            command = "data-read-memory";
            break;
        case DataWriteRegisterVariables:
            command = "data-write-register-values";
            break;

        case EnableTimings:
            command = "enable-timings";
            break;

        case EnvironmentDirectory:
            command = "environment-directory";
            break;
        case EnvironmentPath:
            command = "environment-path";
            break;
        case EnvironmentPwd:
            command = "environment-pwd";
            break;

        case ExecUntil:
            // TODO: write test case for this
            command = "thread until";
            break;

        case FileExecFile:
            command = "file-exec-file";//"exec-file"
            break;
        case FileListExecSourceFile:
            command = "file-list-exec-source-file";
            break;
        case FileListExecSourceFiles:
            command = "file-list-exec-source-files";
            break;
        case FileSymbolFile:
            command = "file-symbol-file";//"symbol-file"
            break;

        case GdbVersion:
            command = "gdb-version";//"show version"
            break;

        case InferiorTtyShow:
            command = "inferior-tty-show";
            break;

        case SignalHandle:
            command = "process handle";
            break;

        case TargetDisconnect:
            command = "target-disconnect";//"disconnect"
            break;
        case TargetDownload:
            command = "target-download";
            break;

        case ThreadListIds:
            command = "thread-list-ids";
            break;
        case ThreadSelect:
            command = "thread-select";
            break;

        case TraceFind:
            command = "trace-find";
            break;
        case TraceStart:
            command = "trace-start";
            break;
        case TraceStop:
            command = "trace-stop";
            break;

        case VarInfoNumChildren:
            command = "var-info-num-children";
            break;
        case VarInfoType:
            command = "var-info-type";
            break;
        case VarSetFrozen:
            command = "var-set-frozen";
            break;
        case VarShowFormat:
            command = "var-show-format";
            break;
        default:
            return MICommand::miCommand();
    }

    if (isMI) {
        command.prepend('-');
    }
    return command;
}

QString LldbCommand::cmdToSend()
{
    switch (type()) {
        // -gdb-set is only partially implemented
        case GdbSet: {
            QString env_name = "environment ";
            QString disassembly_flavor = "disassembly-flavor ";
            if (command_.startsWith(env_name)) {
                command_ = command_.mid(env_name.length());
                overrideCmd = "settings set target.env-vars";
            } else if (command_.startsWith(disassembly_flavor)) {
                command_ = command_.mid(disassembly_flavor.length());
                overrideCmd = "settings set target.x86-disassembly-flavor";
            }
            break;
        }
        // find the postion to insert '-f'
        case BreakInsert: {
            if (!overrideCmd.isEmpty()) {
                // already done
                break;
            }
            int p = command_.length() - 1;
            bool quoted = false;
            if (command_[p] == '"') {
                quoted = true; // should always be the case
            }
            --p;
            for (; p >= 0; --p) {
                // find next '"' or ' '
                if (quoted) {
                    if (command_[p] == '"' && (p == 0 || command_[p-1] != '\\'))
                        break;
                } else {
                    if (command_[p] == ' ')
                        break;
                }
            }
            if (p < 0) p = 0; // this means the command is malformated, we proceed anyway.

            // move other switches like '-d' '-c' into miCommand part
            overrideCmd = miCommand() + " " + command_.left(p);
            command_ = "-f " + command_.mid(p, command_.length());
            break;
        }
        case BreakWatch:
            if (command_.startsWith("-r ")) {
                command_ = "-w read " + command_.mid(3);
            } else if (command_.startsWith("-a ")) {
                command_ = "-w read_write " + command_.mid(3);
            }
            break;
        case StackListArguments:
            // some times when adding the command, the current frame is invalid,
            // but is valid at sending time
            if (command_.endsWith("-1 -1")) {
                command_.replace("-1 -1", QStringLiteral("%1 %1").arg(frame()));
            }
            break;
        default:
            break;
    }
    return MICommand::cmdToSend();
}
