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
#include <QDateTime>

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
            result = result + QStringLiteral(" --thread %1").arg(m_thread);
        if (m_frame != -1)
            result = result + QStringLiteral(" --frame %1").arg(m_frame);

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
        commandHandler_ = nullptr;
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
            command = QLatin1String("");
            break;
        case BreakAfter:
            command = QLatin1String("break-after");//"ignore"
            break;
        case BreakCommands:
            command = QLatin1String("break-commands");
            break;
        case BreakCondition:
            command = QLatin1String("break-condition");//"cond"
            break;
        case BreakDelete:
            command = QLatin1String("break-delete");//"delete breakpoint"
            break;
        case BreakDisable:
            command = QLatin1String("break-disable");//"disable breakpoint"
            break;
        case BreakEnable:
            command = QLatin1String("break-enable");//"enable breakpoint"
            break;
        case BreakInfo:
            command = QLatin1String("break-info");//"info break"
            break;
        case BreakInsert:
            command = QLatin1String("break-insert -f");
            break;
        case BreakList:
            command = QLatin1String("break-list");//"info break"
            break;
        case BreakWatch:
            command = QLatin1String("break-watch");
            break;

        case DataDisassemble:
            command = QLatin1String("data-disassemble");
            break;
        case DataEvaluateExpression:
            command = QLatin1String("data-evaluate-expression");
            break;
        case DataListChangedRegisters:
            command = QLatin1String("data-list-changed-registers");
            break;
        case DataListRegisterNames:
            command = QLatin1String("data-list-register-names");
            break;
        case DataListRegisterValues:
            command = QLatin1String("data-list-register-values");
            break;
        case DataReadMemory:
            command = QLatin1String("data-read-memory");
            break;
        case DataWriteMemory:
            command = QLatin1String("data-write-memory");
            break;
        case DataWriteRegisterVariables:
            command = QLatin1String("data-write-register-values");
            break;

        case EnablePrettyPrinting:
            command = QLatin1String("enable-pretty-printing");
            break;
        case EnableTimings:
            command = QLatin1String("enable-timings");
            break;

        case EnvironmentCd:
            command = QLatin1String("environment-cd");
            break;
        case EnvironmentDirectory:
            command = QLatin1String("environment-directory");
            break;
        case EnvironmentPath:
            command = QLatin1String("environment-path");
            break;
        case EnvironmentPwd:
            command = QLatin1String("environment-pwd");
            break;

        case ExecAbort:
            command = QLatin1String("exec-abort");
            break;
        case ExecArguments:
            command = QLatin1String("exec-arguments");//"set args"
            break;
        case ExecContinue:
            command = QLatin1String("exec-continue");
            break;
        case ExecFinish:
            command = QLatin1String("exec-finish");
            break;
        case ExecInterrupt:
            command = QLatin1String("exec-interrupt");
            break;
        case ExecNext:
            command = QLatin1String("exec-next");
            break;
        case ExecNextInstruction:
            command = QLatin1String("exec-next-instruction");
            break;
        case ExecRun:
            command = QLatin1String("exec-run");
            break;
        case ExecStep:
            command = QLatin1String("exec-step");
            break;
        case ExecStepInstruction:
            command = QLatin1String("exec-step-instruction");
            break;
        case ExecUntil:
            command = QLatin1String("exec-until");
            break;

        case FileExecAndSymbols:
            command = QLatin1String("file-exec-and-symbols");//"file"
            break;
        case FileExecFile:
            command = QLatin1String("file-exec-file");//"exec-file"
            break;
        case FileListExecSourceFile:
            command = QLatin1String("file-list-exec-source-file");
            break;
        case FileListExecSourceFiles:
            command = QLatin1String("file-list-exec-source-files");
            break;
        case FileSymbolFile:
            command = QLatin1String("file-symbol-file");//"symbol-file"
            break;

        case GdbExit:
            command = QLatin1String("gdb-exit");
            break;
        case GdbSet:
            command = QLatin1String("gdb-set");//"set"
            break;
        case GdbShow:
            command = QLatin1String("gdb-show");//"show"
            break;
        case GdbVersion:
            command = QLatin1String("gdb-version");//"show version"
            break;

        case InferiorTtySet:
            command = QLatin1String("inferior-tty-set");
            break;
        case InferiorTtyShow:
            command = QLatin1String("inferior-tty-show");
            break;

        case InterpreterExec:
            command = QLatin1String("interpreter-exec");
            break;

        case ListFeatures:
            command = QLatin1String("list-features");
            break;

        case SignalHandle:
            return QStringLiteral("handle");
            //command = "signal-handle";
            break;

        case StackInfoDepth:
            command = QLatin1String("stack-info-depth");
            break;
        case StackInfoFrame:
            command = QLatin1String("stack-info-frame");
            break;
        case StackListArguments:
            command = QLatin1String("stack-list-arguments");
            break;
        case StackListFrames:
            command = QLatin1String("stack-list-frames");
            break;
        case StackListLocals:
            command = QLatin1String("stack-list-locals");
            break;
        case StackSelectFrame:
            command = QLatin1String("stack-select-frame");
            break;

        case SymbolListLines:
            command = QLatin1String("symbol-list-lines");
            break;

        case TargetAttach:
            command = QLatin1String("target-attach");
            break;
        case TargetDetach:
            command = QLatin1String("target-detach");//"detach"
            break;
        case TargetDisconnect:
            command = QLatin1String("target-disconnect");//"disconnect"
            break;
        case TargetDownload:
            command = QLatin1String("target-download");
            break;
        case TargetSelect:
            command = QLatin1String("target-select");
            break;

        case ThreadInfo:
            command = QLatin1String("thread-info");
            break;
        case ThreadListIds:
            command = QLatin1String("thread-list-ids");
            break;
        case ThreadSelect:
            command = QLatin1String("thread-select");
            break;

        case TraceFind:
            command = QLatin1String("trace-find");
            break;
        case TraceStart:
            command = QLatin1String("trace-start");
            break;
        case TraceStop:
            command = QLatin1String("trace-stop");
            break;

        case VarAssign:
            command = QLatin1String("var-assign");
            break;
        case VarCreate:
            command = QLatin1String("var-create");
            break;
        case VarDelete:
            command = QLatin1String("var-delete");
            break;
        case VarEvaluateExpression:
            command = QLatin1String("var-evaluate-expression");
            break;
        case VarInfoPathExpression:
            command = QLatin1String("var-info-path-expression");
            break;
        case VarInfoNumChildren:
            command = QLatin1String("var-info-num-children");
            break;
        case VarInfoType:
            command = QLatin1String("var-info-type");
            break;
        case VarListChildren:
            command = QLatin1String("var-list-children");
            break;
        case VarSetFormat:
            command = QLatin1String("var-set-format");
            break;
        case VarSetFrozen:
            command = QLatin1String("var-set-frozen");
            break;
        case VarShowAttributes:
            command = QLatin1String("var-show-attributes");
            break;
        case VarShowFormat:
            command = QLatin1String("var-show-format");
            break;
        case VarUpdate:
            command = QLatin1String("var-update");
            break;

        default:
            command = QLatin1String("unknown");
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

void MICommand::markAsEnqueued()
{
    m_enqueueTimestamp = QDateTime::currentMSecsSinceEpoch();
}

void MICommand::markAsSubmitted()
{
    m_submitTimestamp = QDateTime::currentMSecsSinceEpoch();
}

void MICommand::markAsCompleted()
{
    m_completeTimestamp = QDateTime::currentMSecsSinceEpoch();
}

qint64 MICommand::gdbProcessingTime() const
{
    return m_completeTimestamp - m_submitTimestamp;
}

qint64 MICommand::queueTime() const
{
    return m_submitTimestamp - m_enqueueTimestamp;
}

qint64 MICommand::totalProcessingTime() const
{
    return m_completeTimestamp - m_enqueueTimestamp;
}
