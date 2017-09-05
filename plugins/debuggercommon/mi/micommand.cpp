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
            command = QStringLiteral("break-after");//"ignore"
            break;
        case BreakCommands:
            command = QStringLiteral("break-commands");
            break;
        case BreakCondition:
            command = QStringLiteral("break-condition");//"cond"
            break;
        case BreakDelete:
            command = QStringLiteral("break-delete");//"delete breakpoint"
            break;
        case BreakDisable:
            command = QStringLiteral("break-disable");//"disable breakpoint"
            break;
        case BreakEnable:
            command = QStringLiteral("break-enable");//"enable breakpoint"
            break;
        case BreakInfo:
            command = QStringLiteral("break-info");//"info break"
            break;
        case BreakInsert:
            command = QStringLiteral("break-insert -f");
            break;
        case BreakList:
            command = QStringLiteral("break-list");//"info break"
            break;
        case BreakWatch:
            command = QStringLiteral("break-watch");
            break;

        case DataDisassemble:
            command = QStringLiteral("data-disassemble");
            break;
        case DataEvaluateExpression:
            command = QStringLiteral("data-evaluate-expression");
            break;
        case DataListChangedRegisters:
            command = QStringLiteral("data-list-changed-registers");
            break;
        case DataListRegisterNames:
            command = QStringLiteral("data-list-register-names");
            break;
        case DataListRegisterValues:
            command = QStringLiteral("data-list-register-values");
            break;
        case DataReadMemory:
            command = QStringLiteral("data-read-memory");
            break;
        case DataWriteMemory:
            command = QStringLiteral("data-write-memory");
            break;
        case DataWriteRegisterVariables:
            command = QStringLiteral("data-write-register-values");
            break;

        case EnablePrettyPrinting:
            command = QStringLiteral("enable-pretty-printing");
            break;
        case EnableTimings:
            command = QStringLiteral("enable-timings");
            break;

        case EnvironmentCd:
            command = QStringLiteral("environment-cd");
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

        case ExecAbort:
            command = QStringLiteral("exec-abort");
            break;
        case ExecArguments:
            command = QStringLiteral("exec-arguments");//"set args"
            break;
        case ExecContinue:
            command = QStringLiteral("exec-continue");
            break;
        case ExecFinish:
            command = QStringLiteral("exec-finish");
            break;
        case ExecInterrupt:
            command = QStringLiteral("exec-interrupt");
            break;
        case ExecNext:
            command = QStringLiteral("exec-next");
            break;
        case ExecNextInstruction:
            command = QStringLiteral("exec-next-instruction");
            break;
        case ExecRun:
            command = QStringLiteral("exec-run");
            break;
        case ExecStep:
            command = QStringLiteral("exec-step");
            break;
        case ExecStepInstruction:
            command = QStringLiteral("exec-step-instruction");
            break;
        case ExecUntil:
            command = QStringLiteral("exec-until");
            break;

        case FileExecAndSymbols:
            command = QStringLiteral("file-exec-and-symbols");//"file"
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

        case GdbExit:
            command = QStringLiteral("gdb-exit");
            break;
        case GdbSet:
            command = QStringLiteral("gdb-set");//"set"
            break;
        case GdbShow:
            command = QStringLiteral("gdb-show");//"show"
            break;
        case GdbVersion:
            command = QStringLiteral("gdb-version");//"show version"
            break;

        case InferiorTtySet:
            command = QStringLiteral("inferior-tty-set");
            break;
        case InferiorTtyShow:
            command = QStringLiteral("inferior-tty-show");
            break;

        case InterpreterExec:
            command = QStringLiteral("interpreter-exec");
            break;

        case ListFeatures:
            command = QStringLiteral("list-features");
            break;

        case SignalHandle:
            return QStringLiteral("handle");
            //command = "signal-handle";
            break;

        case StackInfoDepth:
            command = QStringLiteral("stack-info-depth");
            break;
        case StackInfoFrame:
            command = QStringLiteral("stack-info-frame");
            break;
        case StackListArguments:
            command = QStringLiteral("stack-list-arguments");
            break;
        case StackListFrames:
            command = QStringLiteral("stack-list-frames");
            break;
        case StackListLocals:
            command = QStringLiteral("stack-list-locals");
            break;
        case StackSelectFrame:
            command = QStringLiteral("stack-select-frame");
            break;

        case SymbolListLines:
            command = QStringLiteral("symbol-list-lines");
            break;

        case TargetAttach:
            command = QStringLiteral("target-attach");
            break;
        case TargetDetach:
            command = QStringLiteral("target-detach");//"detach"
            break;
        case TargetDisconnect:
            command = QStringLiteral("target-disconnect");//"disconnect"
            break;
        case TargetDownload:
            command = QStringLiteral("target-download");
            break;
        case TargetSelect:
            command = QStringLiteral("target-select");
            break;

        case ThreadInfo:
            command = QStringLiteral("thread-info");
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

        case VarAssign:
            command = QStringLiteral("var-assign");
            break;
        case VarCreate:
            command = QStringLiteral("var-create");
            break;
        case VarDelete:
            command = QStringLiteral("var-delete");
            break;
        case VarEvaluateExpression:
            command = QStringLiteral("var-evaluate-expression");
            break;
        case VarInfoPathExpression:
            command = QStringLiteral("var-info-path-expression");
            break;
        case VarInfoNumChildren:
            command = QStringLiteral("var-info-num-children");
            break;
        case VarInfoType:
            command = QStringLiteral("var-info-type");
            break;
        case VarListChildren:
            command = QStringLiteral("var-list-children");
            break;
        case VarSetFormat:
            command = QStringLiteral("var-set-format");
            break;
        case VarSetFrozen:
            command = QStringLiteral("var-set-frozen");
            break;
        case VarShowAttributes:
            command = QStringLiteral("var-show-attributes");
            break;
        case VarShowFormat:
            command = QStringLiteral("var-show-format");
            break;
        case VarUpdate:
            command = QStringLiteral("var-update");
            break;

        default:
            command = QStringLiteral("unknown");
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
