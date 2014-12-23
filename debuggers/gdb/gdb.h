/*
 * Low level GDB interface.
 *
 * Copyright 2007 Vladimir Prus <ghost@cs.msu.su>
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

#ifndef GDB_H_d5c9cb274cbad688fe7a507a84f6633b
#define GDB_H_d5c9cb274cbad688fe7a507a84f6633b

#include "mi/gdbmi.h"
#include "mi/miparser.h"
#include "gdbcommand.h"

#include <KProcess>

#include <QObject>
#include <QByteArray>

class KConfigGroup;

namespace GDBDebugger
{

class GDB : public QObject
{
    Q_OBJECT
public:
    explicit GDB(QObject* parent = 0);
    virtual ~GDB();

    /** Starts GDB.  This should be done after connecting to all
        signals the client is interested in.  */
    void start(KConfigGroup& config, const QStringList& extraArguments = {});

    /** Executes a command.  This method may be called at
        most once each time 'ready' is emitted.  When the
        GDB instance is just constructed, one should wait
        for 'ready' as well.  

        The ownership of 'command' is transferred to GDB.  */
    void execute(GDBCommand* command);

    /** Returns true if 'execute' can be called immediately.  */
    bool isReady() const;

    /** FIXME: temporary, to be eliminated.  */
    GDBCommand* currentCommand() const;
    
    /** Arrange to gdb to stop doing whatever it's doing,
        and start waiting for a command.  
        FIXME: probably should make sure that 'ready' is
        emitted, or something.  */
    void interrupt();

    /** Kills GDB.  */
    void kill();

Q_SIGNALS:
    /** Emitted when debugger becomes ready -- i.e. when
        isReady call will return true.  */
    void ready();

    /** Emitted when GDB itself exits.  This could happen because
        it just crashed due to internal bug, or we killed it
        explicitly.  */
    void gdbExited();

    /** Emitted when GDB reports stop, with 'r' being the
        data provided by GDB. */
    void programStopped(const GDBMI::AsyncRecord& r);
    
    /** Emitted when GDB believes that the program is running.  */
    void programRunning();

    /** Emitted for each MI stream record found.  Presently only
     used to recognize some CLI messages that mean that the program
    has died. 
    FIXME: connect to parseCliLine
    */
    void streamRecord(const GDBMI::StreamRecord& s);

    /** Reports an async notification record.  */
    void notification(const GDBMI::AsyncRecord& n);
    
    /** Emitted for error that is not handled by the
        command being executed. */
    void error(const GDBMI::ResultRecord& s);

    /** Reports output from the running application.
        Generally output will only be available when
        using remote GDB targets. When running locally,
        the output will either appear on GDB stdout, and
        ignored, or routed via pty.  */
    void applicationOutput(const QString& s);

    /** Reports output of a command explicitly typed by
        the user, or output from .gdbinit commands.  */
    void userCommandOutput(const QString& s);

    /** Reports output of a command issued internally
        by KDevelop.  At the moment, stderr output from
        GDB and the 'log' MI channel will be also routed here.  */
    void internalCommandOutput(const QString& s);

private Q_SLOTS:
    void readyReadStandardOutput();
    void readyReadStandardError();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void processErrored(QProcess::ProcessError);

private:
    void processLine(const QByteArray& line);

private:
    QString gdbBinary_;
    KProcess* process_;

    GDBCommand* currentCmd_;

    MIParser mi_parser_;

    /** The unprocessed output from gdb. Output is
        processed as soon as we see newline. */
    QByteArray buffer_;
};

}

#endif
