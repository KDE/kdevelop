// *************************************************************************
//                          rdbcontroller.cpp  -  description
//                             -------------------
//    begin                : Sun Aug 8 1999
//    copyright            : (C) 1999 by John Birch
//    email                : jbb@kdevelop.org
//	
//                          Adapted for ruby debugging
//                          --------------------------
//    begin                : Mon Nov 1 2004
//    copyright            : (C) 2004 by Richard Dale
//    email                : Richard_Dale@tipitina.demon.co.uk
// **************************************************************************
//
// **************************************************************************
// *                                                                        *
// *   This program is free software; you can redistribute it and/or modify *
// *   it under the terms of the GNU General Public License as published by *
// *   the Free Software Foundation; either version 2 of the License, or    *
// *   (at your option) any later version.                                  *
// *                                                                        *
// **************************************************************************

#include "rdbcontroller.h"

#include <sys/types.h> 
#include <sys/fcntl.h>
#include <sys/socket.h> 
#include <sys/un.h>
#include <errno.h>

#include "breakpoint.h"
#include "framestackwidget.h"
#include "rdbcommand.h"
#include "stty.h"
#include "variablewidget.h"
#include "domutil.h"

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>

#include <qdatetime.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <qstring.h>
#include <qtextstream.h>

#include <iostream>
#include <ctype.h>
#include <stdlib.h>
using namespace std;

// **************************************************************************
//
// Does all the communication between rdb and the kdevelop's debugger code.
// Significatant classes being used here are
//
// RDBParser  - parses the "variable" data using the vartree and varitems
// VarTree    - where the variable data will end up
// FrameStack - tracks the program frames and allows the user to switch between
//              and therefore view the calling funtions and their data
// Breakpoint - Where and what to do with breakpoints.
// STTY       - the tty that the _application_ will run on.
//
// Significant variables
// state_     - be very careful setting this. The controller is totally
//              dependent on this reflecting the correct state. For instance,
//              if the app is busy but we don't think so, then we lose control
//              of the app. The only way to get out of these situations is to
//              delete (stop) the controller.
// currentFrame_
//            - Holds the frame number where and locals/variable information will
//              go to
//
//
// **************************************************************************

namespace RDBDebugger
{

// This is here so we can check for startup /shutdown problems
int debug_controllerExists = false;

// At the moment a Unix domain socket is used. It might be better to
// change to tcp/ip and listen on a port instead
const char * RDBController::unixSocketPath_ = "/tmp/.rubydebugger";


RDBController::RDBController(VariableTree *varTree, FramestackWidget *frameStack, QDomDocument &projectDom)
        : DbgController(),
        frameStack_(frameStack),
        varTree_(varTree),
        currentFrame_(1),
        viewedThread_(-1),
        stdoutOutputLen_(0),
        stdoutOutput_(new char[4096]),
        holdingZone_(),
        rdbOutputLen_(0),
        rdbOutput_(new char[49152]),
		socketNotifier_(0),
        currentCmd_(0),
		currentPrompt_("(rdb:1) "),
        tty_(0),
        state_(s_dbgNotStarted|s_appNotStarted|s_silent),
        programHasExited_(false),
        dom(projectDom),
        config_forceBPSet_(true),
        config_dbgTerminal_(false)
{
    struct sockaddr_un	sockaddr;
	QFileInfo			unixSocket(unixSocketPath_);
	
    stdoutSizeofBuf_ = sizeof(stdoutOutput_);
    rdbSizeofBuf_ = sizeof(rdbOutput_);
    
	if (unixSocket.exists()) {
		unlink(unixSocketPath_);
	}
	
	masterSocket_ = socket(AF_UNIX, SOCK_STREAM, 0);
	sockaddr.sun_family = AF_UNIX;
	strcpy(sockaddr.sun_path, unixSocketPath_);
	bind(masterSocket_, (const struct sockaddr*) &sockaddr, sizeof(sockaddr));
	listen(masterSocket_, 1);
	acceptNotifier_ = new QSocketNotifier(masterSocket_, QSocketNotifier::Read, this);
    QObject::connect( acceptNotifier_, SIGNAL(activated(int)),
                          this, SLOT(slotAcceptConnection(int)) );	
						  	
    configure();
    cmdList_.setAutoDelete(true);

    Q_ASSERT(! debug_controllerExists);
    debug_controllerExists = true;
}

// **************************************************************************

// Deleting the controller involves shutting down rdb nicely.
// When were attached to a process, we must first detach so that the process
// can continue running as it was before being attached. rdb is quite slow to
// detach from a process, so we must process events within here to get a "clean"
// shutdown.
RDBController::~RDBController()
{
    delete[] stdoutOutput_;
    delete[] rdbOutput_;
    debug_controllerExists = false;
}

// **************************************************************************

void RDBController::configure()
{    
}

// **************************************************************************

// Fairly obvious that we'll add whatever command you give me to a queue
// If you tell me to, I'll put it at the head of the queue so it'll run ASAP
// Not quite so obvious though is that if we are going to run again. then any
// information requests become redundent and must be removed.
// We also try and run whatever command happens to be at the head of
// the queue.
void RDBController::queueCmd(DbgCommand *cmd, bool executeNext)
{
    // We remove any info command or _run_ command if we are about to
    // add a run command.
    if (cmd->isARunCmd())
        removeInfoRequests();

    if (executeNext)
        cmdList_.insert(0, cmd);
    else
        cmdList_.append (cmd);
}

// **************************************************************************

// If the appliction can accept a command and we've got one waiting
// then send it.
// Commands can be just request for data (or change rdbs state in someway)
// or they can be "run" commands. If a command is sent to rdb our internal
// state will get updated.
void RDBController::executeCmd()
{
    if (stateIsOn(s_dbgNotStarted|s_waitForWrite|s_appBusy|s_shuttingDown) || !dbgProcess_)
        return;

    if (currentCmd_ == 0) {
        if (cmdList_.isEmpty())
            return;

        currentCmd_ = cmdList_.take(0);
    }

    if (!currentCmd_->moreToSend()) {
        delete currentCmd_;
        if (cmdList_.isEmpty()) {
            currentCmd_ = 0;
            return;
        }

        currentCmd_ = cmdList_.take(0);
    }
    
	char * ptr = currentCmd_->cmdToSend().data();
	int bytesToWrite = currentCmd_->cmdLength();
	int bytesWritten = 0;
	
	while (bytesToWrite > 0) {
    	bytesWritten = write(socket_, ptr, bytesToWrite);
		bytesToWrite -= bytesWritten;
		ptr += bytesWritten;
	}
	
    if (currentCmd_->isARunCmd()) {
        setStateOn(s_appBusy);
        kdDebug(9012) << "App is busy" << endl;
        setStateOff(s_appNotStarted|s_programExited|s_silent);
    }

    QString prettyCmd = currentCmd_->cmdToSend();
    prettyCmd = currentPrompt_ + prettyCmd;
    emit rdbStdout( prettyCmd.latin1() );
	
    if (!stateIsOn(s_silent))
        emit dbgStatus("", state_);
}

// **************************************************************************

void RDBController::destroyCmds()
{
    if (currentCmd_)
    {
        delete currentCmd_;
        currentCmd_ = 0;
    }

    while (!cmdList_.isEmpty())
        delete cmdList_.take(0);
}

// **********************************************************************

void RDBController::removeInfoRequests()
{
    int i = cmdList_.count();
    while (i)
    {
        i--;
        DbgCommand *cmd = cmdList_.at(i);
        if (cmd->isAnInfoCmd() || cmd->isARunCmd())
            delete cmdList_.take(i);
    }
}

// **********************************************************************

// Pausing an app removes any pending run commands so that the app doesn't
// start again. If we want to be silent then we remove any pending info
// commands as well.
void RDBController::pauseApp()
{
    int i = cmdList_.count();
    while (i)
    {
        i--;
        DbgCommand *cmd = cmdList_.at(i);
        if ((stateIsOn(s_silent) && cmd->isAnInfoCmd()) || cmd->isARunCmd())
            delete cmdList_.take(i);
    }

    if (dbgProcess_ && stateIsOn(s_appBusy))
        dbgProcess_->kill(SIGINT);
}

// **********************************************************************

// Whenever the program pauses we need to refresh the data visible to
// the user. The reason we've stopped may be passed in  to be emitted.
void RDBController::actOnProgramPause(const QString &msg)
{
    // We're only stopping if we were running, of course.
    if (stateIsOn(s_appBusy))
    {
        kdDebug(9012) << "App is paused" << endl;
        setStateOff(s_appBusy);
        if (stateIsOn(s_silent))
            return;

        emit dbgStatus (msg, state_);

        // We're always at frame one when the program stops
        // and we must reset the active flag
        currentFrame_ = 1;
        varTree_->nextActivationId();
		setStateOn(s_fetchLocals);
        
		queueCmd(new RDBCommand("where", NOTRUNCMD, INFOCMD), true);
        queueCmd(new RDBCommand("thread list", NOTRUNCMD, INFOCMD), true);
		
		if (stateIsOn(s_fetchGlobals)) {
			queueCmd(new RDBCommand("var global", NOTRUNCMD, INFOCMD));
		}
		        
		emit acceptPendingBPs();
    }
}

// **************************************************************************

// There is no app anymore. This can be caused by program exiting
// an invalid program specified or ...
// rdb is still running though, but only the run command (may) make sense
// all other commands are disabled.
void RDBController::programNoApp(const QString &msg, bool msgBox)
{
    state_ = (s_appNotStarted|s_programExited|(state_&(s_shuttingDown)));
    destroyCmds();

    // We're always at frame one when the program stops
    // and we must reset the active flag
    viewedThread_ = -1;
    currentFrame_ = 1;
    varTree_->nextActivationId();

    // Now wipe the tree out
    varTree_->viewport()->setUpdatesEnabled(false);
    varTree_->prune();
    varTree_->viewport()->setUpdatesEnabled(true);
    varTree_->repaint();

    frameStack_->clear();

    if (msgBox)
        KMessageBox::error(0, i18n("rdb message:\n")+msg);

    emit dbgStatus (msg, state_);
}

// **************************************************************************

// The program location falls out of rdb. We treat
// it as a wrapped command.
// The data gets parsed here and emitted in its component parts.
void RDBController::parseProgramLocation(char *buf)
{
	QString buffer(buf);
	QString line;
	QTextStream input(&buffer, IO_ReadOnly);
	QString sourceFile;
	int sourceLine = 0;
	
    //  "1: a = 1"
    QRegExp display_re("^(\\d+):\\s(.*)$");
	
    //  "/opt/qt/src/widgets/qlistview.rb:1558:puts 'hello world'"
    QRegExp sourcepos_re("^([^:]+):(\\d+):");
		
	line = input.readLine();
	while (! line.isNull()) {
 		if (sourcepos_re.search(line, 0) >= 0) {
			sourceFile = sourcepos_re.cap(1);
			sourceLine = sourcepos_re.cap(2).toInt();
    	} else if (display_re.search(line, 0) >= 0) {
			varTree_->watchRoot()->updateWatchExpression(display_re.cap(1).toInt(), display_re.cap(2));
		}
		
		line = input.readLine();
	}

	if ( !sourceFile.isNull() 
		&& !sourceFile.endsWith("/qtruby.rb")
		&& !sourceFile.endsWith("/korundum.rb")
		&& !sourceFile.endsWith("/debuggee.rb") ) 
	{
        actOnProgramPause(QString());
        emit showStepInSource(sourceFile, sourceLine, "");
		return;
	}
	
    if (stateIsOn(s_appBusy))
        actOnProgramPause(i18n("No source: %1").arg(sourceFile));
    else
        emit dbgStatus (i18n("No source: %1").arg(sourceFile), state_);
}

// **************************************************************************

// parsing the backtrace list will cause the vartree to be refreshed
void RDBController::parseBacktraceList(char *buf)
{
    frameStack_->parseRDBBacktraceList(buf);
}

// **************************************************************************

void RDBController::parseThreadList(char *buf)
{
	frameStack_->parseRDBThreadList(buf);
    viewedThread_ = frameStack_->viewedThread();
    varTree_->setCurrentThread(viewedThread_);
}

// **************************************************************************

void RDBController::parseSwitchThread(char *buf)
{
	// Look for the thread number
	// 2 #<Thread:0x30091998 sleep>   /home/duke/play/testit/trykorundum/src/bar.rb:13
    QRegExp thread_re("(\\d+)");
	if (thread_re.search(buf) != -1) {
		viewedThread_ = thread_re.cap(1).toInt();
		currentFrame_ = 1;
	}
}

// **************************************************************************

// After an 'up nnn' or 'down nnn' command, get the new source file and line no.
void RDBController::parseFrameMove(char *buf)
{
	QString sourceFile;
	int sourceLine = 0;
	
    if (stateIsOn(s_fetchLocals)) {
		return;
	}
	
	// "#2 /home/duke/play/testit/trykorundum/src/main.rb:11"
    QRegExp sourcepos_re("#\\d+\\s([^:]+):(\\d+)");
	if (sourcepos_re.search(buf) != -1) {
		sourceFile = sourcepos_re.cap(1);
		sourceLine = sourcepos_re.cap(2).toInt();
		
		if (	!sourceFile.isNull()
				&& !sourceFile.endsWith("/qtruby.rb")
				&& !sourceFile.endsWith("/korundum.rb")
				&& !sourceFile.endsWith("/debuggee.rb") )
		{ 
        	emit showStepInSource(sourceFile, sourceLine, "");
			return;
		}
	}
	
	emit dbgStatus(i18n("No source: %1").arg(sourceFile), state_);
}

// **************************************************************************

// When a breakpoint has been set, rdb responds with some data about the
// new breakpoint. We just inform the breakpoint system about this.
void RDBController::parseBreakpointSet(char *buf)
{
    if (RDBSetBreakpointCommand *BPCmd = dynamic_cast<RDBSetBreakpointCommand*>(currentCmd_))
    {
        // ... except in this case :-) A -1 key tells us that this is
        // a special internal breakpoint, and we shouldn't do anything
        // with it. Currently there are _no_ internal breakpoints.
        if (BPCmd->getKey() != -1) {
            emit rawRDBBreakpointSet(buf, BPCmd->getKey());
		}
    }
}

// **************************************************************************

// Extra data needed by an item was requested. Here's the result.
// If it's an ordinary 'p ' command then just echo the result on
// the RDB console and don't bother parsing.
void RDBController::parseRequestedData(char *buf)
{
    if (RDBItemCommand *rdbItemCommand = dynamic_cast<RDBItemCommand*> (currentCmd_))
    {
        // Fish out the item from the command and let it deal with the data
        VarItem *item = rdbItemCommand->getItem();
        varTree_->viewport()->setUpdatesEnabled(false);
        item->expandValue(buf);
        varTree_->viewport()->setUpdatesEnabled(true);
        varTree_->repaint();
    }
}


// **************************************************************************

// Select a different frame to view. We need to get and (maybe) display
// where we are in the program source.
void RDBController::parseFrameSelected(char *buf)
{
    if (!stateIsOn(s_silent)) {
        emit showStepInSource("", -1, "");
        emit dbgStatus (i18n("No source: %1").arg(QString(buf)), state_);
    }
}

// **************************************************************************

// Sets the id of the display in the VarTree and a current value.
void RDBController::parseDisplay(char *buf, char * expr)
{
    varTree_->viewport()->setUpdatesEnabled(false);
    varTree_->watchRoot()->setWatchExpression(buf, expr);
    varTree_->viewport()->setUpdatesEnabled(true);
    varTree_->repaint();
}

// **************************************************************************

// Updates the watch expressions with current values
void RDBController::parseUpdateDisplay(char *buf)
{
	varTree_->viewport()->setUpdatesEnabled(false);
	
    QRegExp display_re("(\\d+):\\s([^\n]*)\n");
	
	int pos = display_re.search(buf);
	while (pos != -1) {
		varTree_->watchRoot()->updateWatchExpression(display_re.cap(1).toInt(), display_re.cap(2));
		
		pos += display_re.matchedLength();
		pos = display_re.search(buf, pos);
	}

    varTree_->viewport()->setUpdatesEnabled(true);
    varTree_->repaint();
}

// **************************************************************************

// This is called on program stop to process the globals.
void RDBController::parseGlobals(char *buf)
{
    varTree_->viewport()->setUpdatesEnabled(false);
    varTree_->globalRoot()->setGlobals(buf);
    varTree_->viewport()->setUpdatesEnabled(true);
    varTree_->repaint();
}

// **************************************************************************

// This is called on program stop to process the locals.
// Once the locals have been processed we prune the tree of items that are
// inactive.
void RDBController::parseLocals(char type, char *buf)
{
    varTree_->viewport()->setUpdatesEnabled(false);

    // The locals are always attached to the currentFrame
    VarFrameRoot *frame = varTree_->findFrame(currentFrame_, viewedThread_);
    if (!frame)
    {
        frame = new VarFrameRoot(varTree_, currentFrame_, viewedThread_);
        frame->setFrameName(
                frameStack_->findFrame(currentFrame_, viewedThread_)->frameName());
    }

    Q_ASSERT(frame);
	
    if (type == (char) CONSTANTS) {
        frame->addLocals(buf);
    } else if (type == (char) CVARS) {
        frame->addLocals(buf);
    } else if (type == (char) IVARS) {
        frame->addLocals(buf);
    } else {
        frame->addLocals(buf);
        frame->setLocals();
	}

    varTree_->viewport()->setUpdatesEnabled(true);
    varTree_->repaint();
}



// **************************************************************************

void RDBController::parse(char *buf)
{
	if (currentCmd_ == 0) {
		return;
	}
	
	if (currentCmd_->isARunCmd()) {
        parseProgramLocation(buf);
	} else if (currentCmd_->rawDbgCommand() == "break") {
        emit rawRDBBreakpointList(buf);
	} else if (qstrncmp(currentCmd_->rawDbgCommand(), "break ", strlen("break ")) == 0) {
		parseBreakpointSet(buf); 
	} else if (qstrncmp(currentCmd_->rawDbgCommand(), "watch ", strlen("watch ")) == 0) {
		parseBreakpointSet(buf); 
	} else if (qstrncmp(currentCmd_->rawDbgCommand(), "display ", strlen("display ")) == 0) {
		parseDisplay(buf, currentCmd_->rawDbgCommand().data() + strlen("display "));
	} else if (currentCmd_->rawDbgCommand() == "display") {
		parseUpdateDisplay(buf);
	} else if (qstrncmp(currentCmd_->rawDbgCommand(), "undisplay ", strlen("undisplay ")) == 0) {
		;
	} else if (qstrncmp(currentCmd_->rawDbgCommand(), "method instance ", strlen("method instance ")) == 0) { 
	} else if (qstrncmp(currentCmd_->rawDbgCommand(), "method ", strlen("method ")) == 0) {
	} else if (qstrncmp(currentCmd_->rawDbgCommand(), "pp ", strlen("pp ")) == 0) {
		parseRequestedData(buf);
	} else if (currentCmd_->rawDbgCommand() == "thread list") {
		parseThreadList(buf);
	} else if (	qstrncmp(currentCmd_->rawDbgCommand(), "up ", strlen("up ")) == 0
				|| qstrncmp(currentCmd_->rawDbgCommand(), "down ", strlen("down ")) == 0 ) 
	{
		parseFrameMove(buf);
	} else if (qstrncmp(currentCmd_->rawDbgCommand(), "thread switch ", strlen("thread switch ")) == 0) {
		parseSwitchThread(buf);
	} else if (currentCmd_->rawDbgCommand() == "thread current") {
		parseThreadList(buf);
	} else if (currentCmd_->rawDbgCommand() == "where") {
		parseBacktraceList(buf);
	} else if (currentCmd_->rawDbgCommand() == "var global") {
		parseGlobals(buf);
	} else if (currentCmd_->rawDbgCommand() == "var local") {
		parseLocals(LOCALS, buf);
	} else if (qstrncmp(currentCmd_->rawDbgCommand(), "var instance ", strlen("var instance ")) == 0) {
		parseLocals(IVARS, buf);
	} else if (qstrncmp(currentCmd_->rawDbgCommand(), "var class ", strlen("var class ")) == 0) {
		parseLocals(CVARS, buf);
	} else if (qstrncmp(currentCmd_->rawDbgCommand(), "var const ", strlen("var const ")) == 0) {
		parseLocals(CONSTANTS, buf);
	}

    return;
}

// **************************************************************************

void RDBController::setBreakpoint(const QCString &BPSetCmd, int key)
{
    queueCmd(new RDBSetBreakpointCommand(BPSetCmd, key));
}

// **************************************************************************

void RDBController::clearBreakpoint(const QCString &BPClearCmd)
{
    queueCmd(new RDBCommand(BPClearCmd, NOTRUNCMD, NOTINFOCMD));
    // Note: this is NOT an info command, because rdb doesn't explictly tell
    // us that the breakpoint has been deleted, so if we don't have it the
    // BP list doesn't get updated.
    queueCmd(new RDBCommand("break", NOTRUNCMD, NOTINFOCMD));
}

// **************************************************************************

void RDBController::modifyBreakpoint( const Breakpoint& BP )
{
    Q_ASSERT(BP.isActionModify());
    if (BP.dbgId() > 0)
    {
        if (BP.changedEnable())
            queueCmd(new RDBCommand(QCString().sprintf("%s %d",
                            BP.isEnabled() ? "enable" : "disable",
                            BP.dbgId()), NOTRUNCMD, NOTINFOCMD));

        //        BP.setDbgProcessing(true);
        // Note: this is NOT an info command, because rdb doesn't explictly tell
        // us that the breakpoint has been deleted, so if we don't have it the
        // BP list doesn't get updated.
        queueCmd(new RDBCommand("break", NOTRUNCMD, NOTINFOCMD));
    }
}

// **************************************************************************
//                                SLOTS
//                                *****
// For most of these slots data can only be sent to rdb when it
// isn't busy and it is running.

// **************************************************************************

void RDBController::slotStart(const QString& ruby_interpreter, const QString& character_coding, const QString& run_directory, const QString& debuggee_path, const QString &application, const QString& run_arguments)
{
    Q_ASSERT (!dbgProcess_ && !tty_);
	
    tty_ = new STTY(config_dbgTerminal_, "konsole");
    if (!config_dbgTerminal_)
    {
        connect( tty_, SIGNAL(OutOutput(const char*)), SIGNAL(ttyStdout(const char*)) );
        connect( tty_, SIGNAL(ErrOutput(const char*)), SIGNAL(ttyStderr(const char*)) );
    }

    QString tty(tty_->getSlave());
    if (tty.isEmpty())
    {
        KMessageBox::error(0, i18n("The ruby debugger cannot use the tty* or pty* devices.\n"
                                   "Check the settings on /dev/tty* and /dev/pty*\n"
                                   "As root you may need to \"chmod ug+rw\" tty* and pty* devices "
                                   "and/or add the user to the tty group using "
                                   "\"usermod -G tty username\"."));

        delete tty_;
        tty_ = 0;
        return;
    }

    dbgProcess_ = new KProcess;

    connect( dbgProcess_, SIGNAL(receivedStdout(KProcess *, char *, int)),
             this,        SLOT(slotDbgStdout(KProcess *, char *, int)) );

    connect( dbgProcess_, SIGNAL(receivedStderr(KProcess *, char *, int)),
             this,        SLOT(slotDbgStderr(KProcess *, char *, int)) );

    connect( dbgProcess_, SIGNAL(wroteStdin(KProcess *)),
             this,        SLOT(slotDbgWroteStdin(KProcess *)) );

    connect( dbgProcess_, SIGNAL(processExited(KProcess*)),
             this,        SLOT(slotDbgProcessExited(KProcess*)) );

	rubyInterpreter_ = ruby_interpreter;
	characterCoding_ = character_coding;
	runDirectory_ = run_directory;
	debuggeePath_ = debuggee_path;
	application_ = application;
	runArguments_ = run_arguments;

	*dbgProcess_ << ruby_interpreter;
	*dbgProcess_ << character_coding;
	*dbgProcess_ << "-C" << QString(QFile::encodeName( run_directory ));
	*dbgProcess_ << "-r" << debuggee_path;
	*dbgProcess_ << application;
	
	if (! run_arguments.isNull()) {
		*dbgProcess_ << run_arguments;
	}

    emit rdbStdout(QString( ruby_interpreter + character_coding
							+ " -C " + QString(QFile::encodeName( run_directory ))
							+ " -r " + debuggee_path + " " 
							+ application + " " + run_arguments ).latin1() );

    if (!dbgProcess_->start( KProcess::NotifyOnExit,
                        KProcess::Communication(KProcess::All)) )
	{
        kdDebug(9012) << "Couldn't start ruby debugger" << endl;
	}

    // Initialise rdb. At this stage rdb is sitting wondering what to do,
    // and to whom. Organise a few things, then set up the tty for the application,
    // and the application itself

    // Now the ruby debugger has been started and the application has been loaded,
    // BUT the app hasn't been started yet! A run command is about to be issued
    // by whoever is controlling us.
	
    if (!dbgProcess_->writeStdin(QString("%1\n").arg(unixSocketPath_).latin1(), strlen(unixSocketPath_) + 1)) {
        kdDebug(9012) << "failed to write Unix domain socket path to rdb " 
		<< QString("%1\n").arg(unixSocketPath_).latin1() << endl;
	}
	
	setStateOff(s_programExited);
	setStateOn(s_dbgNotStarted|s_appNotStarted|s_silent);
}

// **************************************************************************

void RDBController::slotStopDebugger()
{
    if (stateIsOn(s_shuttingDown) || !dbgProcess_)
        return;

    setStateOn(s_shuttingDown|s_silent);
    destroyCmds();

    QTime start;
    QTime now;

    // Get rdb's attention if it's busy. We need rdb to be at the
    // command line so we can stop it.
    if (stateIsOn(s_appBusy))
    {
        kdDebug(9012) << "ruby debugger busy on shutdown - stopping rdb (SIGINT)" << endl;
        dbgProcess_->kill(SIGINT);
        start = QTime::currentTime();
        while (-1)
        {
            kapp->processEvents(20);
            now = QTime::currentTime();
            if (!stateIsOn(s_appBusy) || start.msecsTo( now ) > 2000)
                break;
        }
    }


    // Now try to stop the ruby debugger running.
    kdDebug(9012) << "App is busy" << endl;
    setStateOn(s_appBusy);
    const char *quit="quit\n";
    if (!dbgProcess_->writeStdin(quit, strlen(quit)))
        kdDebug(9012) << "failed to write 'quit' to ruby debugger" << endl;

    emit rdbStdout("(rdb:1) quit");
    start = QTime::currentTime();
    while (-1)
    {
         kapp->processEvents(20);
         now = QTime::currentTime();
         if (stateIsOn(s_programExited) || start.msecsTo( now ) > 2000)
             break;
    }

    // We cannot wait forever.
    if (!stateIsOn(s_programExited))
    {
        kdDebug(9012) << "rdb not shutdown - killing" << endl;
        dbgProcess_->kill(SIGKILL);
    }

    delete dbgProcess_;    dbgProcess_ = 0;
    delete tty_;           tty_ = 0;

    state_ = s_dbgNotStarted | s_appNotStarted | s_silent;
    emit dbgStatus (i18n("Debugger stopped"), state_);
}



// **************************************************************************

void RDBController::slotRun()
{
	if (stateIsOn(s_appBusy|s_dbgNotStarted|s_shuttingDown))
        return;
		
	if (stateIsOn(s_programExited)) {
		slotStart(rubyInterpreter_, characterCoding_, runDirectory_, debuggeePath_, application_, runArguments_);
		return;
	}

    queueCmd(new RDBCommand("cont", RUNCMD, NOTINFOCMD));
	if (currentCmd_ == 0) {
    	executeCmd();
	}
}

// **************************************************************************

void RDBController::slotRunUntil(const QString &fileName, int lineNum)
{
    if (stateIsOn(s_appBusy|s_dbgNotStarted|s_shuttingDown))
        return;

    if (fileName.isEmpty())
        queueCmd(new RDBCommand( QCString().sprintf("break %d", lineNum),
                                RUNCMD, NOTINFOCMD));
    else
        queueCmd(new RDBCommand(
                QCString().sprintf("break %s:%d", fileName.latin1(), lineNum),
                                RUNCMD, NOTINFOCMD));
    queueCmd(new RDBCommand("cont", RUNCMD, NOTINFOCMD));
								
	if (currentCmd_ == 0) {
    	executeCmd();
	}
}

// **************************************************************************

void RDBController::slotStepInto()
{
    if (stateIsOn(s_appBusy|s_appNotStarted|s_shuttingDown))
        return;

    queueCmd(new RDBCommand("step", RUNCMD, NOTINFOCMD));
	if (currentCmd_ == 0) {
    	executeCmd();
	}
}


// **************************************************************************

void RDBController::slotStepOver()
{
    if (stateIsOn(s_appBusy|s_appNotStarted|s_shuttingDown))
        return;

    queueCmd(new RDBCommand("next", RUNCMD, NOTINFOCMD));
	if (currentCmd_ == 0) {
    	executeCmd();
	}
}


// **************************************************************************

void RDBController::slotStepOutOff()
{
    if (stateIsOn(s_appBusy|s_appNotStarted|s_shuttingDown))
        return;

    queueCmd(new RDBCommand("finish", RUNCMD, NOTINFOCMD));
	if (currentCmd_ == 0) {
    	executeCmd();
	}
}

// **************************************************************************

// Only interrupt a running program.
void RDBController::slotBreakInto()
{
    pauseApp();
}

// **************************************************************************

// See what, if anything needs doing to this breakpoint.
void RDBController::slotBPState( const Breakpoint& BP )
{
	// Are we in a position to do anything to this breakpoint?
    if (stateIsOn(s_dbgNotStarted|s_shuttingDown) || !BP.isPending() ||
            BP.isActionDie())
        return;

    // We need this flag so that we can continue execution. I did use
    // the s_silent state flag but it can be set prior to this method being
    // called, hence is invalid.
    bool restart = false;
    if (stateIsOn(s_appBusy))
    {
        if (!config_forceBPSet_)
            return;

        // When forcing breakpoints to be set/unset, interrupt a running app
        // and change the state.
        setStateOn(s_silent);
        pauseApp();
        restart = true;
    }

    if (BP.isActionAdd())
    {
        setBreakpoint(BP.dbgSetCommand().latin1(), BP.key());
        //        BP.setDbgProcessing(true);
    }
    else
    {
        if (BP.isActionClear())
        {
            clearBreakpoint(BP.dbgRemoveCommand().latin1());
            //            BP.setDbgProcessing(true);
        }
        else
        {
            if (BP.isActionModify())
            {
                modifyBreakpoint(BP); // Note: DbgProcessing gets set in modify fn
            }
        }
    }

    if (restart)
        queueCmd(new RDBCommand("cont", RUNCMD, NOTINFOCMD));
}

// **************************************************************************

void RDBController::slotClearAllBreakpoints()
{
    // Are we in a position to do anything to this breakpoint?
    if (stateIsOn(s_dbgNotStarted|s_shuttingDown))
        return;

    bool restart = false;
    if (stateIsOn(s_appBusy))
    {
        if (!config_forceBPSet_)
            return;

        // When forcing breakpoints to be set/unset, interrupt a running app
        // and change the state.
        setStateOn(s_silent);
        pauseApp();
        restart = true;
    }

    queueCmd(new RDBCommand("delete", NOTRUNCMD, NOTINFOCMD));
    // Note: this is NOT an info command, because rdb doesn't explictly tell
    // us that the breakpoint has been deleted, so if we don't have it the
    // BP list doesn't get updated.
    queueCmd(new RDBCommand("break", NOTRUNCMD, NOTINFOCMD));

    if (restart)
        queueCmd(new RDBCommand("cont", RUNCMD, NOTINFOCMD));
		
	executeCmd();
}



// **************************************************************************

void RDBController::slotSelectFrame(int frameNo, int threadNo, const QString& frameName)
{
	if (stateIsOn(s_appBusy|s_dbgNotStarted|s_shuttingDown)) {
		kdDebug(9012) << "RDBController::slotSelectFrame wrong state" << endl;
        return;
	}

	if (viewedThread_ != threadNo) {
		// Note that 'thread switch nnn' is a run command
		queueCmd(new RDBCommand(QCString().sprintf("thread switch %d",
                                threadNo), RUNCMD, INFOCMD));
		executeCmd();
		return;
	}
		
	if (frameNo > currentFrame_) {
    	queueCmd(new RDBCommand(QCString().sprintf("up %d", frameNo - currentFrame_), NOTRUNCMD, INFOCMD));
		if (!stateIsOn(s_fetchLocals)) {
        	queueCmd(new RDBCommand("display", NOTRUNCMD, INFOCMD));
		}
	} else if (frameNo < currentFrame_) {
    	queueCmd(new RDBCommand(QCString().sprintf("down %d", currentFrame_ - frameNo), NOTRUNCMD, INFOCMD));
		if (!stateIsOn(s_fetchLocals)) {
        	queueCmd(new RDBCommand("display", NOTRUNCMD, INFOCMD));
		}
	}

    // Hold on to  this thread/frame so that we know where to put the local
    // variables if generated.
    viewedThread_ = threadNo;
    currentFrame_ = frameNo;

    VarFrameRoot *frame = varTree_->findFrame(frameNo, viewedThread_);
    if (frame == 0) {
        frame = new VarFrameRoot(varTree_, currentFrame_, viewedThread_);
    }
		
	frame->setFrameName(frameName);
	varTree_->setSelected(frame, true);
	
	// Have we already got these details?
	if (frame->needsVariables()) {
		// Ask for the locals
        queueCmd(new RDBCommand("var const self.class", NOTRUNCMD, INFOCMD));
		queueCmd(new RDBCommand("var instance self", NOTRUNCMD, INFOCMD));
		queueCmd(new RDBCommand("var class self.class", NOTRUNCMD, INFOCMD));
		queueCmd(new RDBCommand("var local", NOTRUNCMD, INFOCMD));
		frame->startWaitingForData();
    }
	
	if (currentCmd_ == 0) {
		executeCmd();
	}
	
	return;
}



// **************************************************************************

// This is called when an item needs special processing to show a value.
void RDBController::slotExpandItem(VarItem *item, const QCString &userRequest)
{
	if (stateIsOn(s_appBusy|s_dbgNotStarted|s_shuttingDown))
        return;

    Q_ASSERT(item != 0);

    // Bad user data!!
    if (userRequest.isEmpty())
        return;

    queueCmd(new RDBItemCommand(item, QCString("pp ") + userRequest.data(), false));
	
	if (currentCmd_ == 0) {
		executeCmd();
	}
}

// **************************************************************************

// This method evaluates text selected with the 'Inspect:' context menu
void RDBController::slotRubyInspect(const QString &inspectText)
{
    queueCmd(new RDBCommand(	QCString().sprintf("p %s", inspectText.latin1()), 
								NOTRUNCMD, 
								INFOCMD ), true );
	executeCmd();
}


// **************************************************************************

// Add a new expression to be displayed in the Watch variable tree
void RDBController::slotAddWatchExpression(const QString& expr, bool execute)
{
	queueCmd(new RDBCommand(	QCString().sprintf("display %s", expr.latin1()), 
								NOTRUNCMD, 
								NOTINFOCMD ) );
	if (execute) {
		executeCmd();
	}
}

// **************************************************************************

// Add a new expression to be displayed in the Watch variable tree
void RDBController::slotRemoveWatchExpression(int displayId)
{
    queueCmd(new RDBCommand(	QCString().sprintf("undisplay %d", displayId), 
								NOTRUNCMD, 
								INFOCMD ) );
	executeCmd();
}


// **************************************************************************

// The user will only get globals if the Global frame is open
void RDBController::slotFetchGlobals(bool fetch)
{
    if (fetch) {
        setStateOn(s_fetchGlobals);
		queueCmd(new RDBCommand("var global", NOTRUNCMD, INFOCMD));
		executeCmd();
    } else {
        setStateOff(s_fetchGlobals);
	}

    kdDebug(9012) << (fetch ? "<Globals ON>": "<Globals OFF>") << endl;
}

// **************************************************************************

// Data from the ruby program's stdout gets processed here.
void RDBController::slotDbgStdout(KProcess *, char *buf, int buflen)
{
    QCString msg(buf, buflen+1);
    emit ttyStdout(msg);
}

// **************************************************************************

// Data from the ruby program's stderr gets processed here.
void RDBController::slotDbgStderr(KProcess *, char *buf, int buflen)
{
    QCString msg(buf, buflen+1);
    emit ttyStderr(msg);
}

// **************************************************************************

void RDBController::slotDbgWroteStdin(KProcess *)
{
//    setStateOff(s_waitForWrite);
    //  if (!stateIsOn(s_silent))
    //    emit dbgStatus ("", state_);
//    executeCmd();
}

// **************************************************************************

void RDBController::slotAcceptConnection(int masterSocket)
{
    Q_ASSERT(masterSocket == masterSocket_);
	
	struct sockaddr sockaddr;
	socklen_t fromlen;
	
	if (socketNotifier_ != 0) {
		close(socket_);
		delete socketNotifier_;
	}
	
	socket_ = accept(masterSocket, &sockaddr, &fromlen);
	if (fcntl(socket_, F_SETFL, O_NONBLOCK) == -1) {
    	kdDebug(9012) << "RDBController::slotAcceptConnection can't set nonblocking socket" << errno << endl;
	}
	
	socketNotifier_ = new QSocketNotifier(socket_, QSocketNotifier::Read, 0);
    QObject::connect( socketNotifier_, SIGNAL(activated(int)),
                          this, SLOT(slotReadFromSocket(int)) );	
    
	setStateOff(s_dbgNotStarted);
    emit dbgStatus ("", state_);
    
	cmdList_.clear();
	
	// Organise any breakpoints.
    emit acceptPendingBPs();
		
    queueCmd(new RDBCommand("cont", RUNCMD, NOTINFOCMD));
	
 	// Reset the display id for any watch expressions already in the variable tree
	varTree_->resetWatchVars();
}

// **************************************************************************

// Output from rdb via the Unix socket gets processed here.
void RDBController::slotReadFromSocket(int socket)
{
	Q_ASSERT(socket == socket_);
    
	static bool parsing = false;

	int bytesRead = read(socket, rdbOutput_ + rdbOutputLen_, rdbSizeofBuf_);
	
	rdbOutputLen_ += bytesRead;
    *(rdbOutput_ + rdbOutputLen_) = 0;
	

    // Already parsing? then get out quick.
    if (parsing)
    {
        kdDebug(9012) << "Already parsing" << endl;
        return;
    }

	QRegExp prompt_re("(\\(rdb:(\\d+)\\) )$");
 	int promptPos = prompt_re.search(rdbOutput_, 0);
	
	// Keep appending output to the rbdOutput_ buffer until the
	// ruby debugger writes the next prompt
    if (promptPos == -1) {
		return;
	}
	
  //  kdDebug(9012) << "RDBController::slotReadFromSocket length: " << rdbOutputLen_ << " input: " << rdbOutput_ << endl;
	
	// Save the prompt, and remove it from the buffer
	currentPrompt_ = prompt_re.cap(1).latin1();
	rdbOutputLen_ -= prompt_re.matchedLength();
   	*(rdbOutput_ + rdbOutputLen_) = 0;	
    
	emit rdbStdout(rdbOutput_);

    parsing = true;
    parse(rdbOutput_);
    parsing = false;
	rdbOutputLen_ = 0;
	
	executeCmd();
	
	if (currentCmd_ == 0 && stateIsOn(s_fetchLocals)) {
		if (!varTree_->schedule()) {
			setStateOff(s_fetchLocals);
		}
	}
}

// **************************************************************************

void RDBController::slotDbgProcessExited(KProcess*)
{
    destroyCmds();
    state_ = s_appNotStarted|s_programExited|(state_&(s_shuttingDown));
    emit dbgStatus (i18n("Process exited"), state_);
    emit rdbStdout("(rdb:1) Process exited\n");
	frameStack_->clear();
	varTree_->clear();	
	
	if (socketNotifier_ != 0) {
		delete socketNotifier_;
		socketNotifier_ = 0;
		close(socket_);
	}
	
    delete dbgProcess_;    dbgProcess_ = 0;
    delete tty_;           tty_ = 0;
}


// **************************************************************************

// Takes abbreviated commands and expands them, before passing them on to rdb
//
void RDBController::slotUserRDBCmd(const QString& cmd)
{
    kdDebug(9012) << "Requested user cmd: " << cmd << endl;
    QRegExp break_re("^b(reak)?(\\s.*)?");
    QRegExp watch_re("^wat(ch)?\\s+(.*)");
    QRegExp delete_re("^del(ete)?(\\s.*)?");
    QRegExp display_re("^disp(lay)?(\\s.*)?");
    QRegExp undisplay_re("^undisp(lay)?(\\s.*)?");
    QRegExp step_re("^s(tep)?(\\s.*)?");
    QRegExp next_re("^n(ext)?(\\s.*)?");
    QRegExp varlocal_re("^v(ar)?\\s+l(ocal)?");
    QRegExp varglobal_re("^v(ar)?\\s+g(lobal)?");
    QRegExp varinstance_re("^v(ar)?\\s+i(nstance)?\\s(.*)");
    QRegExp varconst_re("^v(ar)?\\s+c(onst)?\\s(.*)");
    QRegExp threadlist_re("^th(read)?\\s+l(ist)?");
    QRegExp threadcurrent_re("^th(read)?(\\sc(ur(rent)?)?)?$");
    QRegExp threadswitch_re("^th(read)?(\\ssw(itch)?)?(\\s.*)");
    QRegExp thread_re("^th(read)?(\\s+.*)?");
    QRegExp methodinstance_re("^m(ethod)?\\s+i(nstance)?\\s+(.*)");
    QRegExp method_re("^m(ethod)?\\s+(.*)");
    QRegExp list_re("^l(ist)?(\\s+\\d+-\\d+)?$");
    
    if ( break_re.search(cmd) >= 0 ) {
        queueCmd(new RDBCommand(	QCString().sprintf("break%s", break_re.cap(2).latin1()), 
									NOTRUNCMD, 
									INFOCMD ), true );
    } else if ( watch_re.search(cmd) >= 0 ) {
        queueCmd(new RDBCommand(	QCString().sprintf("watch %s", watch_re.cap(2).latin1()), 
									NOTRUNCMD, 
									INFOCMD ), true );
    } else if ( delete_re.search(cmd) >= 0 ) {
        queueCmd(new RDBCommand(	QCString().sprintf("delete%s", delete_re.cap(2).latin1()), 
									NOTRUNCMD, 
									INFOCMD ), true );
    } else if ( display_re.search(cmd) >= 0 ) {
        queueCmd(new RDBCommand(	QCString().sprintf("display%s", display_re.cap(2).latin1()), 
									NOTRUNCMD, 
									INFOCMD ), true );
    } else if ( undisplay_re.search(cmd) >= 0 ) {
        queueCmd(new RDBCommand(	QCString().sprintf("undisplay%s", undisplay_re.cap(2).latin1()), 
									NOTRUNCMD, 
									INFOCMD ), true );
    } else if ( step_re.search(cmd) >= 0 ) {
        queueCmd(new RDBCommand(	QCString().sprintf("step%s", step_re.cap(2).latin1()), 
									RUNCMD, 
									INFOCMD ), true );
    } else if ( next_re.search(cmd) >= 0 ) {
        queueCmd(new RDBCommand(	QCString().sprintf("next%s", next_re.cap(2).latin1()), 
									RUNCMD, 
									INFOCMD ), true );
    } else if ( varlocal_re.search(cmd) >= 0 ) {
        queueCmd(new RDBCommand("var local", NOTRUNCMD, INFOCMD));
    } else if ( varglobal_re.search(cmd) >= 0 ) {
        queueCmd(new RDBCommand("var global", NOTRUNCMD, INFOCMD));
    } else if ( varinstance_re.search(cmd) >= 0 ) {
        queueCmd(new RDBCommand(	QCString().sprintf("var instance %s", varinstance_re.cap(3).latin1()), 
									NOTRUNCMD, 
									INFOCMD ), true );
    } else if ( varconst_re.search(cmd) >= 0 ) {
        queueCmd(new RDBCommand(	QCString().sprintf("var const %s", varconst_re.cap(3).latin1()), 
									NOTRUNCMD, 
									INFOCMD ), true );
    } else if ( methodinstance_re.search(cmd) >= 0 ) {
        queueCmd(new RDBCommand(	QCString().sprintf("method instance %s", methodinstance_re.cap(3).latin1()), 
									NOTRUNCMD, 
									INFOCMD ), true );
    } else if ( method_re.search(cmd) >= 0 ) {
        queueCmd(new RDBCommand(	QCString().sprintf("method %s", method_re.cap(2).latin1()), 
									NOTRUNCMD, 
									INFOCMD ), true );
    } else if ( list_re.search(cmd) >= 0 ) {
        queueCmd(new RDBCommand(	QCString().sprintf("list%s", list_re.cap(2).latin1()), 
									NOTRUNCMD, 
									INFOCMD ), true );
	} else if (cmd == "c" || cmd == "cont") {
        queueCmd(new RDBCommand("cont", RUNCMD, NOTINFOCMD));
	} else if (cmd == "fi" || cmd == "finish") {
        queueCmd(new RDBCommand("finish", RUNCMD, NOTINFOCMD));
    } else if ( threadlist_re.search(cmd) >= 0 ) {
        queueCmd(new RDBCommand("thread list", NOTRUNCMD, INFOCMD), true);
    } else if ( threadcurrent_re.search(cmd) >= 0 ) {
        queueCmd(new RDBCommand("thread current", NOTRUNCMD, INFOCMD), true );
    } else if ( threadswitch_re.search(cmd) >= 0 ) {
        queueCmd(new RDBCommand(	QCString().sprintf("thread switch%s", threadswitch_re.cap(4).latin1()), 
									RUNCMD, 
									INFOCMD ), true );
    } else if ( thread_re.search(cmd) >= 0 ) {
        queueCmd(new RDBCommand(	QCString().sprintf("thread%s", thread_re.cap(2).latin1()), 
									NOTRUNCMD, 
									INFOCMD ), true );
    } else if (cmd == "frame" || cmd == "f" || cmd == "where" || cmd == "w") {
        queueCmd(new RDBCommand("where", NOTRUNCMD, INFOCMD), true);
    } else if (cmd == "q" || cmd == "quit") {
        slotStopDebugger();
        return;
    } else {
    	kdDebug(9012) << "Passing directly to rdb: " << cmd << endl;
    	queueCmd(new RDBCommand(cmd.latin1(), NOTRUNCMD, INFOCMD));
	}
	
    executeCmd();
}

}

// **************************************************************************
// **************************************************************************
// **************************************************************************
#include "rdbcontroller.moc"
