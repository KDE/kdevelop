/*
    SPDX-FileCopyrightText: 1999 John Birch <jbb@kdevelop.org >
    SPDX-FileCopyrightText: 2007 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "midebugger.h"

#include "debuglog.h"
#include "mi/micommand.h"

#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <sublime/message.h>

#include <KLocalizedString>
#include <KMessageBox>

#include <QApplication>
#include <QString>
#include <QStringList>

#include <csignal>

#include <memory>
#include <stdexcept>
#include <sstream>

#ifdef Q_OS_WIN
#include <Windows.h>
#endif

// #define DEBUG_NO_TRY //to get a backtrace to where the exception was thrown

using namespace KDevMI;
using namespace KDevMI::MI;

MIDebugger::MIDebugger(QObject* parent)
    : QObject(parent)
{
    m_process = new KProcess(this);
    m_process->setOutputChannelMode(KProcess::SeparateChannels);
    connect(m_process, &KProcess::readyReadStandardOutput,
            this, &MIDebugger::readyReadStandardOutput);
    connect(m_process, &KProcess::readyReadStandardError,
            this, &MIDebugger::readyReadStandardError);
    connect(m_process, QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MIDebugger::processFinished);
    connect(m_process, &QProcess::errorOccurred,
            this, &MIDebugger::processErrored);
}

MIDebugger::~MIDebugger()
{
    // prevent Qt warning: QProcess: Destroyed while process is still running.
    if (m_process && m_process->state() == QProcess::Running) {
        disconnect(m_process, &QProcess::errorOccurred,
                    this, &MIDebugger::processErrored);
        m_process->kill();
        m_process->waitForFinished(10);
    }
}

void MIDebugger::execute(std::unique_ptr<MICommand> command)
{
    m_currentCmd = std::move(command);
    QString commandText = m_currentCmd->cmdToSend();

    qCDebug(DEBUGGERCOMMON) << "SEND:" << commandText.trimmed();

    QByteArray commandUtf8 = commandText.toUtf8();

    m_process->write(commandUtf8);
    m_currentCmd->markAsSubmitted();

    const QString prettyCmd = QLatin1String("(gdb) ") + m_currentCmd->cmdToSend();
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
#ifndef Q_OS_WIN
    int pid = m_process->processId();
    if (pid != 0) {
        ::kill(pid, SIGINT);
    }
#else
    SetConsoleCtrlHandler(nullptr, true);
    GenerateConsoleCtrlEvent(0, 0);
    SetConsoleCtrlHandler(nullptr, false);
#endif
}

MICommand* MIDebugger::currentCommand() const
{
    return m_currentCmd.get();
}

void MIDebugger::kill()
{
    m_process->kill();
}

void MIDebugger::readyReadStandardOutput()
{
    auto* const core = KDevelop::ICore::self();
    if (!core || !core->debugController()) {
        const auto nullObject = core ? QLatin1String("the debug controller")
                                     : QLatin1String("the KDevelop core");
        qCDebug(DEBUGGERCOMMON).nospace().noquote()
                << "Cannot process standard output without " << nullObject
                << ". KDevelop must be exiting and " << nullObject << " already destroyed.";
        return;
    }

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
        m_buffer.remove(0, i+1);

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
        qCDebug(DEBUGGERCOMMON) << "Debugger output (pid =" << m_process->processId() << "): " << line;
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
        // So, if a command results in unparsable reply,
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
            auto& result = static_cast<MI::ResultRecord&>(*r);

            // it's still possible for the user to issue a MI command,
            // emit correct signal
            if (m_currentCmd && m_currentCmd->isUserCommand()) {
                emit userCommandOutput(QString::fromUtf8(line) + QLatin1Char('\n'));
            } else {
                emit internalCommandOutput(QString::fromUtf8(line) + QLatin1Char('\n'));
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
            } else if (result.isReasonError()) {
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

            m_currentCmd.reset();
            emit ready();
            break;
        }

        case MI::Record::Async: {
            auto& async = static_cast<MI::AsyncRecord&>(*r);

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
            }
            break;
        }

        case MI::Record::Stream: {

            auto& s = static_cast<MI::StreamRecord&>(*r);

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
        qCCritical(DEBUGGERCOMMON) << "Internal debugger error! The exception:" << e.what()
                                   << "; the MI response:" << line;
        KMessageBox::detailedError(
            qApp->activeWindow(),
            i18nc("<b>Internal debugger error</b>",
                    "<p>The debugger component encountered an internal error while "
                    "processing the reply from the debugger. Please submit a bug report. "
                    "The debug session will now end to prevent potential crash"),
            i18n("The exception is: %1\n"
                "The MI response is: %2", QString::fromUtf8(e.what()),
                QString::fromLatin1(line)),
            i18nc("@title:window", "Internal Debugger Error"));
        emit exited(true, QString::fromUtf8(e.what()));
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
    qCWarning(DEBUGGERCOMMON) << "Debugger ERRORED" << error << m_process->errorString();
    if(error == QProcess::FailedToStart)
    {
        const QString messageText =
            i18n("<b>Could not start debugger.</b>"
                 "<p>Could not run '%1'. "
                 "Make sure that the path name is specified correctly.",
                 m_debuggerExecutable);
        auto* message = new Sublime::Message(messageText, Sublime::Message::Error);
        KDevelop::ICore::self()->uiController()->postMessage(message);

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
            i18nc("@title:window", "Debugger Crashed"));

        emit userCommandOutput(QStringLiteral("Process crashed\n"));
        emit exited(true, i18n("Process crashed"));
    }
}

#include "moc_midebugger.cpp"
