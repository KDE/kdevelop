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

using namespace GDBDebugger;

GDB::GDB() 
: sawPrompt_(false), currentCmd_(0)
{
    process_ = new KProcess(this);
    process_->setOutputChannelMode( KProcess::SeparateChannels ); 
    connect(process_, SIGNAL(readyReadStandardOutput()), 
            SLOT(readyReadStandardOutput()));
    connect(process_, SIGNAL(readyReadStandardError()), 
            SLOT(readyReadStandardError()));
    connect(process_, 
            SIGNAL(finished(int, QProcess::ExitStatus)),
            SLOT(processFinished(int, QProcess::ExitStatus)));
    connect(process_, SIGNAL(error(QProcess::ProcessError)),
            SLOT(processErrored(QProcess::ProcessError)));

    KConfigGroup config(KGlobal::config(), "GDB Debugger");    
    QString gdb = config.readEntry("GDB Path", "gdb");

    QStringList arguments;
    arguments << "--interpreter=mi2" << "-quiet";

    QString shell = config.readEntry("Debugger Shell");
    if( !shell.isEmpty() )
    {
        shell = shell.simplified();
        QString shell_without_args = shell.split(QChar(' ')).first();

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

        arguments.insert(0, gdb );
        arguments.insert(0, shell);
        process_->setShellCommand( KShell::joinArgs( arguments ) );
    }
    else
    {
        process_->setProgram( gdb, arguments );
    }

    process_->start();

    emit userCommandOutput(shell + " " + gdb
                           + " --interpreter=mi2 -quiet\n" );
}


void GDB::execute(GDBCommand* command)
{
    currentCmd_ = command;
    QString commandText = currentCmd_->cmdToSend();

    kDebug(9012) << "SEND:" << commandText;
    
    process_->write(commandText.toLatin1(),
                    commandText.length());
    
    QString prettyCmd = currentCmd_->cmdToSend();
    prettyCmd.replace( QRegExp("set prompt \032.\n"), "" );
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
    ::kill(process_->pid(), SIGINT);
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
    emit internalCommandOutput(QString::fromLocal8Bit(
                                   process_->readAll()));
}

void GDB::processLine(const QByteArray& line)
{
    FileSymbol file;
    file.contents = line;

    std::auto_ptr<GDBMI::Record> r(mi_parser_.parse(&file));

    if (r.get() == 0)
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
       bool ready_for_next_command = false;

       try
       {
           switch(r->kind)
           {
           case GDBMI::Record::Result: {
               
               GDBMI::ResultRecord& result = static_cast<GDBMI::ResultRecord&>(*r);
               
               if (currentCmd_->isUserCommand())
                   emit userCommandOutput(QString::fromLocal8Bit(line));
               else
                   emit internalCommandOutput(QString::fromLocal8Bit(line) + "\n");

               if (result.reason == "stopped")
               {
                   emit programStopped(result);
                   ready_for_next_command = true;
               }
               else if (result.reason == "running")
               {
                   emit programRunning();
               }

               /* In theory, commands that run inferior and asynchronous,
                  and we get back gdb prompt, and can send further
                  commands. As of Dec 2007, this actually does not work,
                  and fully async operation of GDB is only being designed.
                  So, if we see "running", assume we can't yet send
                  commands.  */

               ready_for_next_command = (result.reason != "running");

               if (result.reason == "done")
               {
                   currentCmd_->invokeHandler(result);
                   emit resultRecord(result);
               }
               else if (result.reason == "error")
               {
                   // Some commands want to handle errors themself.
                   if (currentCmd_->handlesError() &&
                       currentCmd_->invokeHandler(result))
                   {
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
           }
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

           delete currentCmd_;
           currentCmd_ = 0;
           ready_for_next_command = true;
       }

       if (ready_for_next_command)
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
    // FIXME: revive
#if 0
    bool abnormal = exitCode != 0 || exitStatus != QProcess::NormalExit;
    delete m_process;
    m_process = 0;
    delete tty_;
    tty_ = 0;

    if (abnormal)
        emit debuggerAbnormalExit();

    raiseEvent(debugger_exited);

    destroyCmds();
    setState(s_dbgNotStarted|s_appNotStarted|s_programExited);
    emit showMessage(i18n("Process exited"), 3000);

    emit gdbUserCommandStdout("(gdb) Process exited\n");
#endif
}

void GDB::processErrored(QProcess::ProcessError error)
{
    // FIXME: revive
#if 0
    if( error == QProcess::FailedToStart )
    {
        KMessageBox::information(
            qApp->activeWindow(),
            i18n("<b>Could not start debugger.</b>"
                 "<p>Could not run '%1'. "
                 "Make sure that the path name is specified correctly."
                , m_process->program()[0]),
            i18n("Could not start debugger"));
        delete m_process;
        m_process = 0;
        delete tty_;
        tty_ = 0;

        emit debuggerAbnormalExit();

        raiseEvent(debugger_exited);

        destroyCmds();
        setState(s_dbgNotStarted|s_appNotStarted|s_programExited);
        emit showMessage(i18n("Process didn't start"), 3000);

        emit gdbUserCommandStdout("(gdb) didn't start\n");
    }
#endif
}
