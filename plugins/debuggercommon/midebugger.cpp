/*
 * Low level GDB interface.
 *
 * Copyright 1999 John Birch <jbb@kdevelop.org >
 * Copyright 2007 Vladimir Prus <ghost@cs.msu.su>
 * Copyright 2016 Aetf <aetf@unlimitedcodeworks.xyz>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "midebugger.h"

#include "debuglog.h"
#include "mi/micommand.h"

#include <KLocalizedString>
#include <KMessageBox>

#include <QApplication>
#include <QString>
#include <QStringList>

#include <signal.h>

#include <memory>
#include <stdexcept>
#include <sstream>

// #define DEBUG_NO_TRY //to get a backtrace to where the exception was thrown

using namespace KDevMI;
using namespace KDevMI::MI;

MIDebugger::MIDebugger(QObject* parent)
    : QObject(parent)
    , m_process(nullptr)
    , m_currentCmd(nullptr)
{
    m_process = new KProcess(this);
    m_process->setOutputChannelMode(KProcess::SeparateChannels);
    connect(m_process, &KProcess::readyReadStandardOutput,
            this, &MIDebugger::readyReadStandardOutput);
    connect(m_process, &KProcess::readyReadStandardError,
            this, &MIDebugger::readyReadStandardError);
    connect(m_process,
            static_cast<void(KProcess::*)(int,QProcess::ExitStatus)>(&KProcess::finished),
            this, &MIDebugger::processFinished);
    connect(m_process, static_cast<void(KProcess::*)(QProcess::ProcessError)>(&KProcess::error),
            this, &MIDebugger::processErrored);
}

MIDebugger::~MIDebugger()
{
    // prevent Qt warning: QProcess: Destroyed while process is still running.
    if (m_process && m_process->state() == QProcess::Running) {
        disconnect(m_process, static_cast<void(KProcess::*)(QProcess::ProcessError)>(&KProcess::error),
                    this, &MIDebugger::processErrored);
        m_process->kill();
        m_process->waitForFinished(10);
    }
}

void MIDebugger::execute(MICommand* command)
{
    m_currentCmd = command;
    QString commandText = m_currentCmd->cmdToSend();

    qCDebug(DEBUGGERCOMMON) << "SEND:" << commandText.trimmed();

    QByteArray commandUtf8 = commandText.toUtf8();

    m_process->write(commandUtf8, commandUtf8.length());
    command->markAsSubmitted();

    QString prettyCmd = m_currentCmd->cmdToSend();
    prettyCmd.remove( QRegExp("set prompt \032.\n") );
    prettyCmd = "(gdb) " + prettyCmd;

    if (m_currentCmd->isUserCommand())
        emit userCommandOutput(prettyCmd);
    else
        emit internalCommandOutput(prettyCmd);
}

bool MIDebugger::isReady() const
{
    return m_currentCmd == nullptr;
}

void MIDebugger::interrupt()
{
    //TODO:win32 Porting needed
    int pid = m_process->pid();
    if (pid != 0) {
        ::kill(pid, SIGINT);
    }
}

MICommand* MIDebugger::currentCommand() const
{
    return m_currentCmd;
}

void MIDebugger::kill()
{
    m_process->kill();
}

void MIDebugger::readyReadStandardOutput()
{
    m_process->setReadChannel(QProcess::StandardOutput);

    m_buffer += m_process->readAll();
    for (;;)
    {
        /* In MI mode, all messages are exactly one line.
           See if we have any complete lines in the buffer. */
        int i = m_buffer.indexOf('\n');
        if (i == -1)
            break;
        QByteArray reply(m_buffer.left(i));
        m_buffer = m_buffer.mid(i+1);

        processLine(reply);
    }
}

void MIDebugger::readyReadStandardError()
{
    m_process->setReadChannel(QProcess::StandardError);
    emit debuggerInternalOutput(QString::fromUtf8(m_process->readAll()));
}

void MIDebugger::processLine(const QByteArray& line)
{
    if (line != "(gdb) ") {
        qCDebug(DEBUGGERCOMMON) << "Debugger output (pid =" << m_process->pid() << "): " << line;
    }

    FileSymbol file;
    file.contents = line;

    std::unique_ptr<MI::Record> r(m_parser.parse(&file));

    if (!r)
    {
        // simply ignore the invalid MI message because both gdb and lldb
        // sometimes produces invalid messages that can be safely ignored.
        qCDebug(DEBUGGERCOMMON) << "Invalid MI message:" << line;
        // We don't consider the current command done.
        // So, if a command results in unparseable reply,
        // we'll just wait for the "right" reply, which might
        // never come.  However, marking the command as
        // done in this case is even more risky.
        // It's probably possible to get here if we're debugging
        // natively without PTY, though this is uncommon case.
        return;
    }

    #ifndef DEBUG_NO_TRY
    try
    {
    #endif
        switch(r->kind)
        {
        case MI::Record::Result: {
            MI::ResultRecord& result = static_cast<MI::ResultRecord&>(*r);

            // it's still possible for the user to issue a MI command,
            // emit correct signal
            if (m_currentCmd && m_currentCmd->isUserCommand()) {
                emit userCommandOutput(QString::fromUtf8(line) + '\n');
            } else {
                emit internalCommandOutput(QString::fromUtf8(line) + '\n');
            }

            // protect against wild replies that sometimes returned from gdb without a pending command
            if (!m_currentCmd)
            {
                qCWarning(DEBUGGERCOMMON) << "Received a result without a pending command";
                throw std::runtime_error("Received a result without a pending command");
            }
            else if (m_currentCmd->token() != result.token)
            {
                std::stringstream ss;
                ss << "Received a result with token not matching pending command. "
                   << "Pending: " << m_currentCmd->token() << "Received: " << result.token;
                qCWarning(DEBUGGERCOMMON) << ss.str().c_str();
                throw std::runtime_error(ss.str());
            }

            // GDB doc: "running" and "exit" are status codes equivalent to "done"
            if (result.reason == QLatin1String("done") || result.reason == QLatin1String("running") || result.reason == QLatin1String("exit"))
            {
                qCDebug(DEBUGGERCOMMON) << "Result token is" << result.token;
                m_currentCmd->markAsCompleted();
                qCDebug(DEBUGGERCOMMON) << "Command successful, times "
                                        << m_currentCmd->totalProcessingTime()
                                        << m_currentCmd->queueTime()
                                        << m_currentCmd->gdbProcessingTime();
                m_currentCmd->invokeHandler(result);
            }
            else if (result.reason == QLatin1String("error"))
            {
                qCDebug(DEBUGGERCOMMON) << "Handling error";
                m_currentCmd->markAsCompleted();
                qCDebug(DEBUGGERCOMMON) << "Command error, times"
                                        << m_currentCmd->totalProcessingTime()
                                        << m_currentCmd->queueTime()
                                        << m_currentCmd->gdbProcessingTime();
                // Some commands want to handle errors themself.
                if (m_currentCmd->handlesError() &&
                    m_currentCmd->invokeHandler(result))
                {
                    qCDebug(DEBUGGERCOMMON) << "Invoked custom handler\n";
                    // Done, nothing more needed
                }
                else
                    emit error(result);
            }
            else
            {
                qCDebug(DEBUGGERCOMMON) << "Unhandled result code: " << result.reason;
            }

            delete m_currentCmd;
            m_currentCmd = nullptr;
            emit ready();
            break;
        }

        case MI::Record::Async: {
            MI::AsyncRecord& async = static_cast<MI::AsyncRecord&>(*r);

            switch (async.subkind) {
            case MI::AsyncRecord::Exec: {
                // Prefix '*'; asynchronous state changes of the target
                if (async.reason == QLatin1String("stopped"))
                {
                    emit programStopped(async);
                }
                else if (async.reason == QLatin1String("running"))
                {
                    emit programRunning();
                }
                else
                {
                    qCDebug(DEBUGGERCOMMON) << "Unhandled exec notification: " << async.reason;
                }
                break;
            }

            case MI::AsyncRecord::Notify: {
                // Prefix '='; supplementary information that we should handle (new breakpoint etc.)
                emit notification(async);
                break;
            }

            case MI::AsyncRecord::Status: {
                // Prefix '+'; GDB documentation:
                // On-going status information about progress of a slow operation; may be ignored
                break;
            }

            default:
                Q_ASSERT(false);
            }
            break;
        }

        case MI::Record::Stream: {

            MI::StreamRecord& s = static_cast<MI::StreamRecord&>(*r);

            if (s.subkind == MI::StreamRecord::Target) {
                emit applicationOutput(s.message);
            } else if (s.subkind == MI::StreamRecord::Console) {
                if (m_currentCmd && m_currentCmd->isUserCommand())
                    emit userCommandOutput(s.message);
                else
                    emit internalCommandOutput(s.message);

                if (m_currentCmd)
                    m_currentCmd->newOutput(s.message);
            } else {
                emit debuggerInternalOutput(s.message);
            }

            emit streamRecord(s);

            break;
        }

        case MI::Record::Prompt:
            break;
        }
    #ifndef DEBUG_NO_TRY
    }
    catch(const std::exception& e)
    {
        KMessageBox::detailedSorry(
            qApp->activeWindow(),
            i18nc("<b>Internal debugger error</b>",
                    "<p>The debugger component encountered internal error while "
                    "processing reply from gdb. Please submit a bug report. "
                    "The debug session will now end to prevent potential crash"),
            i18n("The exception is: %1\n"
                "The MI response is: %2", e.what(),
                QString::fromLatin1(line)),
            i18n("Internal debugger error"));
        emit exited(true, e.what());
    }
    #endif
}

void MIDebugger::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCDebug(DEBUGGERCOMMON) << "Debugger FINISHED\n";

    bool abnormal = exitCode != 0 || exitStatus != QProcess::NormalExit;
    emit userCommandOutput(QStringLiteral("Process exited\n"));
    emit exited(abnormal, i18n("Process exited"));
}

void MIDebugger::processErrored(QProcess::ProcessError error)
{
    qCDebug(DEBUGGERCOMMON) << "Debugger ERRORED" << error;
    if(error == QProcess::FailedToStart)
    {
        KMessageBox::information(
            qApp->activeWindow(),
            i18n("<b>Could not start debugger.</b>"
                 "<p>Could not run '%1'. "
                 "Make sure that the path name is specified correctly.",
                 m_debuggerExecutable),
            i18n("Could not start debugger"));

        emit userCommandOutput(QStringLiteral("Process failed to start\n"));
        emit exited(true, i18n("Process failed to start"));

    } else if (error == QProcess::Crashed) {
        KMessageBox::error(
            qApp->activeWindow(),
            i18n("<b>Debugger crashed.</b>"
                 "<p>The debugger process '%1' crashed.<br>"
                 "Because of that the debug session has to be ended.<br>"
                 "Try to reproduce the crash without KDevelop and report a bug.<br>",
                 m_debuggerExecutable),
            i18n("Debugger crashed"));

        emit userCommandOutput(QStringLiteral("Process crashed\n"));
        emit exited(true, i18n("Process crashed"));
    }
}
