/*
    SPDX-FileCopyrightText: 1999 John Birch <jbb@kdevelop.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    , stateReloading_(false)
    , m_thread(-1)
    , m_frame(-1)
{
}

MICommand::~MICommand() = default;

QString MICommand::cmdToSend()
{
    return initialString() + QLatin1Char('\n');
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
            result += QStringLiteral(" --thread %1").arg(m_thread);
        if (m_frame != -1)
            result += QStringLiteral(" --frame %1").arg(m_frame);

        if (!command_.isEmpty())
            result += QLatin1Char(' ') + command_;
    }

    return result;
}

bool MICommand::isUserCommand() const
{
    return false;
}

void MICommand::setHandler(MICommandHandler* handler)
{
    m_commandHandler.reset(handler);

    if (!m_commandHandler) {
        flags_ = flags_ & ~CmdHandlesError;
    }
}

void MICommand::setHandler(const FunctionCommandHandler::Function& callback)
{
    setHandler(new FunctionCommandHandler(callback, flags()));
}

bool MICommand::invokeHandler(const ResultRecord& r)
{
    if (m_commandHandler) {
        m_commandHandler->handle(r);
        m_commandHandler.reset();
        return true;
    } else {
        return false;
    }
}

void MICommand::newOutput(const QString& line)
{
    lines.push_back(line);
}

void MICommand::newLogStreamOutput(const QString& receivedLine)
{
    if (m_storeLogStreamOutput) {
        lines.push_back(receivedLine);
    }
}

void MICommand::storeLogStreamOutput()
{
    m_storeLogStreamOutput = true;
}

const QStringList& MICommand::allStreamOutput() const
{
    return lines;
}

bool MICommand::handlesError() const
{
    return m_commandHandler ? m_commandHandler->handlesError() : false;
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
    switch (type()) {
        case NonMI:
            return QStringLiteral("-");
        case BreakAfter:
            return QStringLiteral("-break-after");//"ignore"
        case BreakCommands:
            return QStringLiteral("-break-commands");
        case BreakCondition:
            return QStringLiteral("-break-condition");//"cond"
        case BreakDelete:
            return QStringLiteral("-break-delete");//"delete breakpoint"
        case BreakDisable:
            return QStringLiteral("-break-disable");//"disable breakpoint"
        case BreakEnable:
            return QStringLiteral("-break-enable");//"enable breakpoint"
        case BreakInfo:
            return QStringLiteral("-break-info");//"info break"
        case BreakInsert:
            return QStringLiteral("-break-insert -f");
        case BreakList:
            return QStringLiteral("-break-list");//"info break"
        case BreakWatch:
            return QStringLiteral("-break-watch");

        case DataDisassemble:
            return QStringLiteral("-data-disassemble");
        case DataEvaluateExpression:
            return QStringLiteral("-data-evaluate-expression");
        case DataListChangedRegisters:
            return QStringLiteral("-data-list-changed-registers");
        case DataListRegisterNames:
            return QStringLiteral("-data-list-register-names");
        case DataListRegisterValues:
            return QStringLiteral("-data-list-register-values");
        case DataReadMemory:
            return QStringLiteral("-data-read-memory");
        case DataWriteMemory:
            return QStringLiteral("-data-write-memory");
        case DataWriteRegisterVariables:
            return QStringLiteral("-data-write-register-values");

        case EnablePrettyPrinting:
            return QStringLiteral("-enable-pretty-printing");
        case EnableTimings:
            return QStringLiteral("-enable-timings");

        case EnvironmentCd:
            return QStringLiteral("-environment-cd");
        case EnvironmentDirectory:
            return QStringLiteral("-environment-directory");
        case EnvironmentPath:
            return QStringLiteral("-environment-path");
        case EnvironmentPwd:
            return QStringLiteral("-environment-pwd");

        case ExecAbort:
            return QStringLiteral("-exec-abort");
        case ExecArguments:
            return QStringLiteral("-exec-arguments");//"set args"
        case ExecContinue:
            return QStringLiteral("-exec-continue");
        case ExecFinish:
            return QStringLiteral("-exec-finish");
        case ExecInterrupt:
            return QStringLiteral("-exec-interrupt");
        case ExecNext:
            return QStringLiteral("-exec-next");
        case ExecNextInstruction:
            return QStringLiteral("-exec-next-instruction");
        case ExecRun:
            return QStringLiteral("-exec-run");
        case ExecStep:
            return QStringLiteral("-exec-step");
        case ExecStepInstruction:
            return QStringLiteral("-exec-step-instruction");
        case ExecUntil:
            return QStringLiteral("-exec-until");

        case FileExecAndSymbols:
            return QStringLiteral("-file-exec-and-symbols");//"file"
        case FileExecFile:
            return QStringLiteral("-file-exec-file");//"exec-file"
        case FileListExecSourceFile:
            return QStringLiteral("-file-list-exec-source-file");
        case FileListExecSourceFiles:
            return QStringLiteral("-file-list-exec-source-files");
        case FileSymbolFile:
            return QStringLiteral("-file-symbol-file");//"symbol-file"

        case GdbExit:
            return QStringLiteral("-gdb-exit");
        case GdbSet:
            return QStringLiteral("-gdb-set");//"set"
        case GdbShow:
            return QStringLiteral("-gdb-show");//"show"
        case GdbVersion:
            return QStringLiteral("-gdb-version");//"show version"

        case InferiorTtySet:
            return QStringLiteral("-inferior-tty-set");
        case InferiorTtyShow:
            return QStringLiteral("-inferior-tty-show");

        case InterpreterExec:
            return QStringLiteral("-interpreter-exec");

        case ListFeatures:
            return QStringLiteral("-list-features");

        case SignalHandle:
            return QStringLiteral("handle");
            //return QStringLiteral("-signal-handle");

        case StackInfoDepth:
            return QStringLiteral("-stack-info-depth");
        case StackInfoFrame:
            return QStringLiteral("-stack-info-frame");
        case StackListArguments:
            return QStringLiteral("-stack-list-arguments");
        case StackListFrames:
            return QStringLiteral("-stack-list-frames");
        case StackListLocals:
            return QStringLiteral("-stack-list-locals");
        case StackSelectFrame:
            return QStringLiteral("-stack-select-frame");

        case SymbolListLines:
            return QStringLiteral("-symbol-list-lines");

        case TargetAttach:
            return QStringLiteral("-target-attach");
        case TargetDetach:
            return QStringLiteral("-target-detach");//"detach"
        case TargetDisconnect:
            return QStringLiteral("-target-disconnect");//"disconnect"
        case TargetDownload:
            return QStringLiteral("-target-download");
        case TargetSelect:
            return QStringLiteral("-target-select");

        case ThreadInfo:
            return QStringLiteral("-thread-info");
        case ThreadListIds:
            return QStringLiteral("-thread-list-ids");
        case ThreadSelect:
            return QStringLiteral("-thread-select");

        case TraceFind:
            return QStringLiteral("-trace-find");
        case TraceStart:
            return QStringLiteral("-trace-start");
        case TraceStop:
            return QStringLiteral("-trace-stop");

        case VarAssign:
            return QStringLiteral("-var-assign");
        case VarCreate:
            return QStringLiteral("-var-create");
        case VarDelete:
            return QStringLiteral("-var-delete");
        case VarEvaluateExpression:
            return QStringLiteral("-var-evaluate-expression");
        case VarInfoPathExpression:
            return QStringLiteral("-var-info-path-expression");
        case VarInfoNumChildren:
            return QStringLiteral("-var-info-num-children");
        case VarInfoType:
            return QStringLiteral("-var-info-type");
        case VarListChildren:
            return QStringLiteral("-var-list-children");
        case VarSetFormat:
            return QStringLiteral("-var-set-format");
        case VarSetFrozen:
            return QStringLiteral("-var-set-frozen");
        case VarShowAttributes:
            return QStringLiteral("-var-show-attributes");
        case VarShowFormat:
            return QStringLiteral("-var-show-format");
        case VarUpdate:
            return QStringLiteral("-var-update");
    }

    return QStringLiteral("-unknown");
}

CommandType MICommand::type() const
{
    return type_;
}

bool MICommand::needsContext() const
{
    return (type_ >= StackInfoDepth && type_ <= StackListLocals)
        || (type_ >= VarAssign && type_ <= VarUpdate && type_ != VarDelete && type_ != VarSetFormat);
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
