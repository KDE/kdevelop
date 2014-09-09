/*
 * Low level GDB interface.
 *
 * Copyright 1999 John Birch <jbb@kdevelop.org >
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

#include "gdb.h"
#include "debugsession.h"

#include <KConfig>
#include <KConfigGroup>
#include <KGlobal>
#include <KMessageBox>
#include <KShell>
#include <KLocale>
#include <kdebug.h>

#include <QApplication>
#include <QFileInfo>

#include <memory>

#include <sys/types.h>
#include <signal.h>

// #define DEBUG_NO_TRY //to get a backtrace to where the exception was thrown

using namespace GDBDebugger;

GDB::GDB(QObject* parent)
: QObject(parent), process_(0), sawPrompt_(false), currentCmd_(0), receivedReply_(false), isRunning_(false)
{
}

GDB::~GDB()
{
    // prevent Qt warning: QProcess: Destroyed while process is still running.
    if (process_ && process_->state() == QProcess::Running) {
        disconnect(process_, SIGNAL(error(QProcess::ProcessError)),
                    this, SLOT(processErrored(QProcess::ProcessError)));
        process_->kill();
        process_->waitForFinished(10);
    }
}

void GDB::start(KConfigGroup& config)
{
    // FIXME: verify that default value leads to something sensible
    QUrl gdbUrl = config.readEntry(GDBDebugger::gdbPathEntry, QUrl());
    if (gdbUrl.isEmpty()) {
        gdbBinary_ = "gdb";
    } else {
        // FIXME: verify its' a local path.
        gdbBinary_ = gdbUrl.url(QUrl::PreferLocalFile | QUrl::StripTrailingSlash);
    }
    process_ = new KProcess(this);
    process_->setOutputChannelMode( KProcess::SeparateChannels );
    connect(process_, SIGNAL(readyReadStandardOutput()),
            SLOT(readyReadStandardOutput()));
    connect(process_, SIGNAL(readyReadStandardError()),
            SLOT(readyReadStandardError()));
    connect(process_,
            SIGNAL(finished(int,QProcess::ExitStatus)),
            SLOT(processFinished(int,QProcess::ExitStatus)));
    connect(process_, SIGNAL(error(QProcess::ProcessError)),
            SLOT(processErrored(QProcess::ProcessError)));


    QStringList arguments;
    arguments << "--interpreter=mi2" << "-quiet";

    QUrl shell = config.readEntry(GDBDebugger::debuggerShellEntry, QUrl());
    if( !shell.isEmpty() )
    {
        kDebug(9012) << "have shell" << shell;
        QString shell_without_args = shell.toLocalFile().split(QChar(' ')).first();

        QFileInfo info( shell_without_args );
        /*if( info.isRelative() )
        {
            shell_without_args = build_dir + "/" + shell_without_args;
            info.setFile( shell_without_args );
        }*/
        if( !info.exists() )
        {
            KMessageBox::information(
                qApp->activeWindow(),
                i18n("Could not locate the debugging shell '%1'.", shell_without_args ),
                i18n("Debugging Shell Not Found") );
            // FIXME: throw, or set some error message.
            return;
        }

        arguments.insert(0, gdbBinary_);
        arguments.insert(0, shell.toLocalFile());
        process_->setShellCommand( KShell::joinArgs( arguments ) );
    }
    else
    {
        process_->setProgram( gdbBinary_, arguments );
    }

    process_->start();

    kDebug(9012) << "STARTING GDB\n";
    emit userCommandOutput(shell.toLocalFile() + ' ' + gdbBinary_
                           + " --interpreter=mi2 -quiet\n" );
}


void GDB::execute(GDBCommand* command)
{
    currentCmd_ = command;
    QString commandText = currentCmd_->cmdToSend();

    kDebug(9012) << "SEND:" << commandText;
    
    isRunning_ = false;
    receivedReply_ = false;

    QByteArray commandUtf8 = commandText.toUtf8();

    process_->write(commandUtf8, commandUtf8.length());

    QString prettyCmd = currentCmd_->cmdToSend();
    prettyCmd.remove( QRegExp("set prompt \032.\n") );
    prettyCmd = "(gdb) " + prettyCmd;

    if (currentCmd_->isUserCommand())
        emit userCommandOutput(prettyCmd);
    else
        emit internalCommandOutput(prettyCmd);
}

bool GDB::isReady() const
{
    return currentCmd_ == 0;
}

void GDB::interrupt()
{
    //TODO:win32 Porting needed
    int pid = process_->pid();
    if (pid != 0)
        ::kill(pid, SIGINT);
}

GDBCommand* GDB::currentCommand() const
{
    return currentCmd_;
}

void GDB::kill()
{
    process_->kill();
}

void GDB::readyReadStandardOutput()
{
    process_->setReadChannel(QProcess::StandardOutput);

    buffer_ += process_->readAll();
    for (;;)
    {
        /* In MI mode, all messages are exactly one line.
           See if we have any complete lines in the buffer. */
        int i = buffer_.indexOf('\n');
        if (i == -1)
            break;
        QByteArray reply(buffer_.left(i));
        buffer_ = buffer_.mid(i+1);

        processLine(reply);
    }
}

void GDB::readyReadStandardError()
{
    process_->setReadChannel(QProcess::StandardOutput);
    emit internalCommandOutput(QString::fromUtf8(process_->readAll()));
}

void GDB::processLine(const QByteArray& line)
{
    kDebug(9012) << "GDB output: " << line;
    if(!currentCmd_)
    {
        kDebug(9012) << "No current command\n";
        return;
    }

    FileSymbol file;
    file.contents = line;

    std::unique_ptr<GDBMI::Record> r(mi_parser_.parse(&file));

    if (!r)
    {
        // FIXME: Issue an error!
        kDebug(9012) << "Invalid MI message:" << line;
        // We don't consider the current command done.
        // So, if a command results in unparseable reply,
        // we'll just wait for the "right" reply, which might
        // never come.  However, marking the command as
        // done in this case is even more risky.
        // It's probably possible to get here if we're debugging
        // natively without PTY, though this is uncommon case.
        return;
    }

   if (!sawPrompt_)
   {
       if (r->kind == GDBMI::Record::Stream)
       {
           GDBMI::StreamRecord& s = dynamic_cast<GDBMI::StreamRecord&>(*r);
           emit userCommandOutput(s.message);
       }
       else if (r->kind == GDBMI::Record::Prompt)
       {
           sawPrompt_ = true;
       }
   }
   else
   {
       
       #ifndef DEBUG_NO_TRY
       try
       {
       #endif
           switch(r->kind)
           {
           case GDBMI::Record::Result: {

               GDBMI::ResultRecord& result = static_cast<GDBMI::ResultRecord&>(*r);

               emit internalCommandOutput(QString::fromUtf8(line) + '\n');
               
               // FIXME: the code below should be reviewed to consider result record
               // subtype when doing all decisions.
               
               if (result.subkind == GDBMI::ResultRecord::GeneralNotification)
               {
                   kDebug(9012) << "General notification";
                   emit notification(result);
                   return;
               }
               
               if (result.reason == "stopped")
               {
                   //stopped is *not* a reply, wait for ^running or ^done (running before stopped, done after stopped)
                   isRunning_ = false;
                   emit programStopped(result);
               }
               else if (result.reason == "running")
               {
                   receivedReply_ = true;
                   isRunning_ = true;
                   emit programRunning();
               }
               else
               {
                   receivedReply_ = true;
               }

               if (result.reason == "done")
               {
                   currentCmd_->invokeHandler(result);
                   emit resultRecord(result);
               }
               else if (result.reason == "error")
               {
                   kDebug(9012) << "Handling error";
                   // Some commands want to handle errors themself.
                   if (currentCmd_->handlesError() &&
                       currentCmd_->invokeHandler(result))
                   {
                       kDebug(9012) << "Invoked custom handler\n";
                       // Done, nothing more needed
                   }
                   else
                       emit error(result);
               }

               break;
           }

           case GDBMI::Record::Stream: {

               GDBMI::StreamRecord& s = dynamic_cast<GDBMI::StreamRecord&>(*r);

               if (s.reason == '@')
                   emit applicationOutput(s.message);

               if (currentCmd_->isUserCommand())
                   emit userCommandOutput(s.message);
               else
                   emit internalCommandOutput(s.message);

               currentCmd_->newOutput(s.message);

               emit streamRecord(s);

               break;
           }
           case GDBMI::Record::Prompt:
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
                     "processing reply from gdb. Please submit a bug report."),
               i18n("The exception is: %1\n"
                    "The MI response is: %2", e.what(),
                    QString::fromLatin1(line)),
               i18n("Internal debugger error"));
            isRunning_ = false;
            receivedReply_ = true;
       }
       #endif

       if (receivedReply_ && !isRunning_)
       {
           delete currentCmd_;
           currentCmd_ = 0;
           emit ready();
       }
    }
}

// **************************************************************************

void GDB::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
    kDebug(9012) << "GDB FINISHED\n";
    /* FIXME: return the status? */
    emit gdbExited();


/* FIXME: revive. Need to arrange for controller to delete us.
    bool abnormal = exitCode != 0 || exitStatus != QProcess::NormalExit;
    deleteLater();
    delete tty_;
    tty_ = 0;

    if (abnormal)
        emit debuggerAbnormalExit();

    raiseEvent(debugger_exited);

    destroyCmds();
    setState(s_dbgNotStarted|s_appNotStarted|s_programExited);
    emit showMessage(i18n("Process exited"), 3000);

    emit gdbUserCommandStdout("(gdb) Process exited\n");
*/
}

void GDB::processErrored(QProcess::ProcessError error)
{
    kDebug(9012) << "GDB ERRORED" << error;
    if( error == QProcess::FailedToStart )
    {
        KMessageBox::information(
            qApp->activeWindow(),
            i18n("<b>Could not start debugger.</b>"
                 "<p>Could not run '%1'. "
                 "Make sure that the path name is specified correctly.",
                 gdbBinary_),
            i18n("Could not start debugger"));

        /* FIXME: make sure the controller gets rids of GDB instance
        emit debuggerAbnormalExit();

        raiseEvent(debugger_exited); */

        /* Used to be before, GDB controller might want to do
           the same.
        destroyCmds();
        setState(s_dbgNotStarted|s_appNotStarted|s_programExited);
        emit showMessage(i18n("Process didn't start"), 3000);
        */
        emit userCommandOutput("(gdb) didn't start\n");
    } else if (error == QProcess::Crashed) {
        KMessageBox::error(
            qApp->activeWindow(),
            i18n("<b>Gdb crashed.</b>"
                 "<p>Because of that the debug session has to be ended.<br>"
                 "Try to reproduce the crash with plain gdb and report a bug.<br>"),
            i18n("Gdb crashed"));
    }
}
