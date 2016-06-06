/***************************************************************************
    begin                : Sun Aug 8 1999
    copyright            : (C) 1999 by John Birch
    email                : jbb@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "micommand.h"

using namespace KDevMI::MI;

FunctionCommandHandler::FunctionCommandHandler(const FunctionCommandHandler::Function& callback, CommandFlags flags)
    : _flags(flags)
    , _callback(callback)
{
}

bool FunctionCommandHandler::handlesError()
{
    return _flags & CmdHandlesError;
}

void FunctionCommandHandler::handle(const ResultRecord& r)
{
    _callback(r);
}


MICommand::MICommand(CommandType type, const QString& command, CommandFlags flags)
    : type_(type)
    , flags_(flags)
    , command_(command)
    , commandHandler_(nullptr)
    , stateReloading_(false)
    , m_thread(-1)
    , m_frame(-1)
{
}

MICommand::~MICommand()
{
    if (commandHandler_ && commandHandler_->autoDelete()) {
        delete commandHandler_;
    }
    commandHandler_ = nullptr;
}

QString MICommand::cmdToSend()
{
    return initialString() + '\n';
}

QString MICommand::initialString() const
{
    QString result = QString::number(token());

    if (type() == NonMI) {
        result += command_;
    } else
    {
        result += miCommand();

        if (m_thread != -1)
            result = result + QString(" --thread %1").arg(m_thread);
        if (m_frame != -1)
            result = result + QString(" --frame %1").arg(m_frame);

        if (!command_.isEmpty())
            result += ' ' + command_;
    }

    return result;
}

bool MICommand::isUserCommand() const
{
    return false;
}

void MICommand::setHandler(MICommandHandler* handler)
{
    if (commandHandler_ && commandHandler_->autoDelete())
        delete commandHandler_;
    commandHandler_ = handler;

    if (!commandHandler_) {
        flags_ = flags_ & ~CmdHandlesError;
    }
}

void MICommand::setHandler(const FunctionCommandHandler::Function& callback)
{
    setHandler(new FunctionCommandHandler(callback, flags()));
}

bool MICommand::invokeHandler(const ResultRecord& r)
{
    if (commandHandler_) {
        //ask before calling handler as it might deleted itself in handler
        bool autoDelete = commandHandler_->autoDelete();

        commandHandler_->handle(r);
        if (autoDelete) {
            delete commandHandler_;
        }
        commandHandler_ = 0;
        return true;
    } else {
        return false;
    }
}

void MICommand::newOutput(const QString& line)
{
    lines.push_back(line);
}

const QStringList& MICommand::allStreamOutput() const
{
    return lines;
}

bool MICommand::handlesError() const
{
    return commandHandler_ ? commandHandler_->handlesError() : false;
}

UserCommand::UserCommand(CommandType type, const QString& s)
    : MICommand(type, s, CmdMaybeStartsRunning)
{
}

bool UserCommand::isUserCommand() const
{
    return true;
}

QString MICommand::miCommand() const
{
    QString command;

    switch (type()) {
        case NonMI:
            command = "";
            break;
        case BreakAfter:
            command = "break-after";//"ignore"
            break;
        case BreakCatch: // FIXME: non-exist command
            command = "break-catch";
            break;
        case BreakCommands:
            command = "break-commands";
            break;
        case BreakCondition:
            command = "break-condition";//"cond"
            break;
        case BreakDelete:
            command = "break-delete";//"delete breakpoint"
            break;
        case BreakDisable:
            command = "break-disable";//"disable breakpoint"
            break;
        case BreakEnable:
            command = "break-enable";//"enable breakpoint"
            break;
        case BreakInfo:
            command = "break-info";//"info break"
            break;
        case BreakInsert:
            command = "break-insert -f";
            break;
        case BreakList:
            command = "break-list";//"info break"
            break;
        case BreakWatch:
            command = "break-watch";
            break;

        case DataDisassemble:
            command = "data-disassemble";
            break;
        case DataEvaluateExpression:
            command = "data-evaluate-expression";
            break;
        case DataListChangedRegisters:
            command = "data-list-changed-registers";
            break;
        case DataListRegisterNames:
            command = "data-list-register-names";
            break;
        case DataListRegisterValues:
            command = "data-list-register-values";
            break;
        case DataReadMemory:
            command = "data-read-memory";
            break;
        case DataWriteMemory:
            command = "data-write-memory";
            break;
        case DataWriteRegisterVariables:
            command = "data-write-register-values";
            break;

        case EnablePrettyPrinting:
            command = "enable-pretty-printing";
            break;
        case EnableTimings:
            command = "enable-timings";
            break;

        case EnvironmentCd:
            command = "environment-cd";
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

        case ExecAbort:
            command = "exec-abort";
            break;
        case ExecArguments:
            command = "exec-arguments";//"set args"
            break;
        case ExecContinue:
            command = "exec-continue";
            break;
        case ExecFinish:
            command = "exec-finish";
            break;
        case ExecInterrupt:
            command = "exec-interrupt";
            break;
        case ExecNext:
            command = "exec-next";
            break;
        case ExecNextInstruction:
            command = "exec-next-instruction";
            break;
        case ExecReturn:
            command = "exec-command =";
            break;
        case ExecRun:
            command = "exec-run";
            break;
        case ExecShowArguments:
            command = "exec-show-arguments";
            break;
        case ExecSignal:
            command = "exec-signal";
            break;
        case ExecStep:
            command = "exec-step";
            break;
        case ExecStepInstruction:
            command = "exec-step-instruction";
            break;
        case ExecUntil:
            command = "exec-until";
            break;

        case FileClear:
            command = "file-clear";
            break;
        case FileExecAndSymbols:
            command = "file-exec-and-symbols";//"file"
            break;
        case FileExecFile:
            command = "file-exec-file";//"exec-file"
            break;
        case FileListExecSections:
            command = "file-list-exec-sections";
            break;
        case FileListExecSourceFile:
            command = "file-list-exec-source-file";
            break;
        case FileListExecSourceFiles:
            command = "file-list-exec-source-files";
            break;
        case FileListSharedLibraries:
            command = "file-list-shared-libraries";
            break;
        case FileListSymbolFiles:
            command = "file-list-symbol-files";
            break;
        case FileSymbolFile:
            command = "file-symbol-file";//"symbol-file"
            break;

        case GdbComplete:
            command = "gdb-complete";
            break;
        case GdbExit:
            command = "gdb-exit";
            break;
        case GdbSet:
            command = "gdb-set";//"set"
            break;
        case GdbShow:
            command = "gdb-show";//"show"
            break;
        case GdbSource:
            command = "gdb-source";
            break;
        case GdbVersion:
            command = "gdb-version";//"show version"
            break;

        case InferiorTtySet:
            command = "inferior-tty-set";
            break;
        case InferiorTtyShow:
            command = "inferior-tty-show";
            break;

        case InterpreterExec:
            command = "interpreter-exec";
            break;

        case ListFeatures:
            command = "list-features";
            break;

        case OverlayAuto:
            command = "overlay-auto";
            break;
        case OverlayListMappingState:
            command = "overlay-list-mapping-state";
            break;
        case OverlayListOverlays:
            command = "overlay-list-overlays";
            break;
        case OverlayMap:
            command = "overlay-map";
            break;
        case OverlayOff:
            command = "overlay-off";
            break;
        case OverlayOn:
            command = "overlay-on";
            break;
        case OverlayUnmap:
            command = "overlay-unmap";
            break;

        case SignalHandle:
            return "handle";
            //command = "signal-handle";
            break;
        case SignalListHandleActions:
            command = "signal-list-handle-actions";
            break;
        case SignalListSignalTypes:
            command = "signal-list-signal-types";
            break;

        case StackInfoDepth:
            command = "stack-info-depth";
            break;
        case StackInfoFrame:
            command = "stack-info-frame";
            break;
        case StackListArguments:
            command = "stack-list-arguments";
            break;
        case StackListExceptionHandlers:
            command = "stack-list-exception-handlers";
            break;
        case StackListFrames:
            command = "stack-list-frames";
            break;
        case StackListLocals:
            command = "stack-list-locals";
            break;
        case StackSelectFrame:
            command = "stack-select-frame";
            break;

        case SymbolInfoAddress:
            command = "symbol-info-address";
            break;
        case SymbolInfoFile:
            command = "symbol-info-file";
            break;
        case SymbolInfoFunction:
            command = "symbol-info-function";
            break;
        case SymbolInfoLine:
            command = "symbol-info-line";
            break;
        case SymbolInfoSymbol:
            command = "symbol-info-symbol";
            break;
        case SymbolListFunctions:
            command = "symbol-list-functions";
            break;
        case SymbolListLines:
            command = "symbol-list-lines";
            break;
        case SymbolListTypes:
            command = "symbol-list-types";
            break;
        case SymbolListVariables:
            command = "symbol-list-variables";
            break;
        case SymbolLocate:
            command = "symbol-locate";
            break;
        case SymbolType:
            command = "symbol-type";
            break;

        case TargetAttach:
            command = "target-attach";
            break;
        case TargetCompareSections:
            command = "target-compare-sections";
            break;
        case TargetDetach:
            command = "target-detach";//"detach"
            break;
        case TargetDisconnect:
            command = "target-disconnect";//"disconnect"
            break;
        case TargetDownload:
            command = "target-download";
            break;
        case TargetExecStatus:
            command = "target-exec-status";
            break;
        case TargetListAvailableTargets:
            command = "target-list-available-targets";
            break;
        case TargetListCurrentTargets:
            command = "target-list-current-targets";
            break;
        case TargetListParameters:
            command = "target-list-parameters";
            break;
        case TargetSelect:
            command = "target-select";
            break;

        case ThreadInfo:
            command = "thread-info";
            break;
        case ThreadListAllThreads:
            command = "thread-list-all-threads";
            break;
        case ThreadListIds:
            command = "thread-list-ids";
            break;
        case ThreadSelect:
            command = "thread-select";
            break;

        case TraceActions:
            command = "trace-actions";
            break;
        case TraceDelete:
            command = "trace-delete";
            break;
        case TraceDisable:
            command = "trace-disable";
            break;
        case TraceDump:
            command = "trace-dump";
            break;
        case TraceEnable:
            command = "trace-enable";
            break;
        case TraceExists:
            command = "trace-exists";
            break;
        case TraceFind:
            command = "trace-find";
            break;
        case TraceFrameNumber:
            command = "trace-frame-number";
            break;
        case TraceInfo:
            command = "trace-info";
            break;
        case TraceInsert:
            command = "trace-insert";
            break;
        case TraceList:
            command = "trace-list";
            break;
        case TracePassCount:
            command = "trace-pass-count";
            break;
        case TraceSave:
            command = "trace-save";
            break;
        case TraceStart:
            command = "trace-start";
            break;
        case TraceStop:
            command = "trace-stop";
            break;

        case VarAssign:
            command = "var-assign";
            break;
        case VarCreate:
            command = "var-create";
            break;
        case VarDelete:
            command = "var-delete";
            break;
        case VarEvaluateExpression:
            command = "var-evaluate-expression";
            break;
        case VarInfoPathExpression:
            command = "var-info-path-expression";
            break;
        case VarInfoExpression:
            command = "var-info-expression";
            break;
        case VarInfoNumChildren:
            command = "var-info-num-children";
            break;
        case VarInfoType:
            command = "var-info-type";
            break;
        case VarListChildren:
            command = "var-list-children";
            break;
        case VarSetFormat:
            command = "var-set-format";
            break;
        case VarSetFrozen:
            command = "var-set-frozen";
            break;
        case VarShowAttributes:
            command = "var-show-attributes";
            break;
        case VarShowFormat:
            command = "var-show-format";
            break;
        case VarUpdate:
            command = "var-update";
            break;

        default:
            command = "unknown";
            break;
    }

    return '-' + command;
}

CommandType MICommand::type() const
{
    return type_;
}

int MICommand::thread() const
{
    return m_thread;
}

void MICommand::setThread(int thread)
{
    m_thread = thread;
}

int MICommand::frame() const
{
    return m_frame;
}

void MICommand::setFrame(int frame)
{
    m_frame = frame;
}

QString MICommand::command() const
{
    return command_;
}

void MICommand::setStateReloading(bool f)
{
    stateReloading_ = f;
}

bool MICommand::stateReloading() const
{
    return stateReloading_;
}
