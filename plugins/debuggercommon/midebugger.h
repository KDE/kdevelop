/*
    SPDX-FileCopyrightText: 2007 Vladimir Prus <ghost@cs.msu.su>
    SPDX-FileCopyrightText: 2016 Aetf <aetf@unlimitedcodeworks.xyz>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MIDEBUGGER_H
#define MIDEBUGGER_H

#include "mi/mi.h"
#include "mi/miparser.h"

#include <KProcess>

#include <QByteArray>
#include <QObject>

#include <memory>

class KConfigGroup;
class KProcess;

namespace KDevMI {

namespace MI {
class MICommand;
}


class MIDebugger : public QObject
{
    Q_OBJECT
public:
    explicit MIDebugger(QObject* parent = nullptr);
    ~MIDebugger() override;

    /** Starts the debugger.  This should be done after connecting to all
        signals the client is interested in.  */
    virtual bool start(KConfigGroup& config, const QStringList& extraArguments = {}) = 0;

    /** Executes a command.  This method may be called at
        most once each time 'ready' is emitted.  When the
        debugger instance is just constructed, one should wait
        for 'ready' as well.

        The ownership of 'command' is transferred to the debugger.  */
    void execute(std::unique_ptr<MI::MICommand> command);

    /** Returns true if 'execute' can be called immediately.  */
    bool isReady() const;

    /** FIXME: temporary, to be eliminated.  */
    MI::MICommand* currentCommand() const;

    /** Arrange to debugger to stop doing whatever it's doing,
        and start waiting for a command.
        FIXME: probably should make sure that 'ready' is
        emitted, or something.  */
    void interrupt();

    /** Kills the debugger.  */
    void kill();

Q_SIGNALS:
    /** Emitted when debugger becomes ready -- i.e. when
        isReady call will return true.  */
    void ready();

    /** Emitted when the debugger itself exits. This could happen because
        it just crashed due to internal bug, or we killed it
        explicitly.  */
    void exited(bool abnormal, const QString &msg);

    /** Emitted when debugger reports stop, with 'r' being the
        data provided by the debugger. */
    void programStopped(const MI::AsyncRecord& r);

    /** Emitted when debugger believes that the program is running.  */
    void programRunning();

    /** Emitted for each MI stream record found.  Presently only
     used to recognize some CLI messages that mean that the program
    has died. 
    FIXME: connect to parseCliLine
    */
    void streamRecord(const MI::StreamRecord& s);

    /** Reports an async notification record.  */
    void notification(const MI::AsyncRecord& n);

    /** Emitted for error that is not handled by the
        command being executed. */
    void error(const MI::ResultRecord& s);

    /** Reports output from the running application.
        Generally output will only be available when
        using remote debugger targets. When running locally,
        the output will either appear on debugger stdout, and
        ignored, or routed via pty.  */
    void applicationOutput(const QString& s);

    /** Reports output of a command explicitly typed by
        the user, or output from .gdbinit commands.  */
    void userCommandOutput(const QString& s);

    /** Reports output of a command issued internally
        by KDevelop. */
    void internalCommandOutput(const QString& s);

    /** Reports debugger internal output, including stderr output from debugger
        and the 'log' MI channel */
    void debuggerInternalOutput(const QString& s);

protected Q_SLOTS:
    void readyReadStandardOutput();
    void readyReadStandardError();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void processErrored(QProcess::ProcessError);

protected:
    void processLine(const QByteArray& line);

protected:
    QString m_debuggerExecutable;
    KProcess* m_process = nullptr;

    std::unique_ptr<MI::MICommand> m_currentCmd;
    MI::MIParser m_parser;

    /** The unprocessed output from debugger. Output is
        processed as soon as we see newline. */
    QByteArray m_buffer;
};

}

#endif
