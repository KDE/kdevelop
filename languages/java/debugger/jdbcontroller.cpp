// **************************************************************************
//                          jdbcontroller.cpp  -  description
//                             -------------------
//    begin                : Mon Apr 16 2001
//    copyright            : (C) 2001 by Oliver Strutynski
//    email                : olistrut@gmx.net
//    This code is heavily based on John Birch's original JDB Controller.
// **************************************************************************
// *                                                                        *
// *   This program is free software; you can redistribute it and/or modify *
// *   it under the terms of the GNU General Public License as published by *
// *   the Free Software Foundation; either version 2 of the License, or    *
// *   (at your option) any later version.                                  *
// *                                                                        *
// **************************************************************************


#include "jdbcontroller.h"

#include "javadebuggerpart.h"
#include "breakpoint.h"
#include "framestackwidget.h"
#include "variablewidget.h"
#include "jdbcommand.h"
#include "stty.h"

#include <kapplication.h>
#include <kconfig.h>
#include <kmessagebox.h>

#include <kglobal.h>
#include <klocale.h>

#include <kprocess.h>
#include <kdebug.h>

#include <qregexp.h>
#include <qstring.h>
#include <qtimer.h>
#include <qurl.h>

#include <iostream>
#include <ctype.h>

using namespace std;

#define BREAKPOINT_HIT 1
#define MARK_FOUND 2
#define JDB_MONITOR 1



#if defined(DBG_MONITOR)
  #define JDB_MONITOR
  #define DBG_DISPLAY(X)          {kdDebug() << QString(X) << endl;}
#else
  #define DBG_DISPLAY(X)          {kdDebug() << QString(X) << endl;}
#endif

#if defined(JDB_MONITOR)
  #define JDB_DISPLAY(X)          {kdDebug() << QString(X) << endl;}
#else
  #define JDB_DISPLAY(X)          {kdDebug() << QString(X) << endl;}
#endif


namespace JAVADebugger
{

/**
 *
 * Does all the communication between jdb and the kdevelop's debugger code.
 * Significatant classes being used here are
 *
 * VarTree    - where the variable data will end up
 * FrameStack - tracks the program frames and allows the user to switch between
 *              and therefore view the calling funtions and their data
 * Breakpoint - Where and what to do with breakpoints.
 * STTY       - the tty that the _application_ will run on.
 *
 * Significant variables
 * state_     - be very careful setting this. The controller is totally
 *              dependent on this reflecting the correct state. For instance,
 *              if the app is busy but we don't think so, then we lose control
 *              of the app. The only way to get out of these situations is to
 *              delete (stop) the controller.
 * currentFrame_
 *            - Holds the frame number where and locals/variable information will
 *              go to
 *
 *
*/
JDBController::JDBController(VariableTree *varTree, FramestackWidget *frameStack, QString projectDirectory, QString mainProgram)
    : DbgController(),

	  classpath_(projectDirectory + "/src:" + (getenv("CLASSPATH") == 0 ? "." : getenv("CLASSPATH"))),
	  mainclass_(mainProgram),
	  projectDirectory_(projectDirectory + "/src"),
      frameStack_(frameStack),
      varTree_(varTree),
      currentFrame_(0),
      state_(s_dbgNotStarted|s_appNotStarted|s_silent),
      jdbSizeofBuf_(2048),
      jdbOutputLen_(0),
      jdbOutput_(new char[2048]),
      currentCmd_(0),
      tty_(0),
      programHasExited_(false),
      config_forceBPSet_(true),
      config_displayStaticMembers_(false),
      config_dbgTerminal_(false),
      config_jdbPath_()
{
    KConfig *config = JavaDebuggerFactory::instance()->config();
    config->setGroup("Debug");
    Q_ASSERT(!config->readBoolEntry("Use external debugger", false));

    config_displayStaticMembers_  = config->readBoolEntry("Display static members", false);
    config_forceBPSet_            = config->readBoolEntry("Allow forced BP set", true);
    config_jdbPath_               = config->readPathEntry("JDB path");
    config_dbgTerminal_           = config->readBoolEntry("Debug on separate tty console", false);

#if defined (JDB_MONITOR)
    kdDebug() << "Connect\n";
    connect(  this,   SIGNAL(dbgStatus(const QString&, int)),
              SLOT(slotDbgStatus(const QString&, int)));
#endif

#if defined (DBG_MONITOR)
    connect(  this,   SIGNAL(showStepInSource(const QString&, int, const QString&)),
              SLOT(slotStepInSource(const QString&,int)));
#endif

    cmdList_.setAutoDelete(true);
}

// **************************************************************************

// Deleting the controller involves shutting down jdb nicely.
// When were attached to a process, we must first detach so that the process
// can continue running as it was before being attached. jdb is quite slow to
// detach from a process, so we must process events within here to get a "clean"
// shutdown.
JDBController::~JDBController()
{
    setStateOn(s_shuttingDown);
    destroyCmds();

    if (dbgProcess_) {
        setStateOn(s_silent);
        pauseApp();
        setStateOn(s_waitTimer);

        QTimer *timer;

        timer = new QTimer(this);
        connect(timer, SIGNAL(timeout()), this, SLOT(slotAbortTimedEvent()) );


        setStateOn(s_waitTimer|s_appBusy);
        const char *quit="quit\n";
        dbgProcess_->writeStdin(quit, strlen(quit));
        JDB_DISPLAY(quit);
            timer->start(3000, TRUE);
        DBG_DISPLAY("<quit wait>\n");
            while (stateIsOn(s_waitTimer)) {
                if (stateIsOn(s_programExited))
                    break;
                kapp->processEvents(20);
            }

        // We cannot wait forever.
        if (stateIsOn(s_shuttingDown))
            dbgProcess_->kill(SIGKILL);
    }

    delete tty_; tty_ = 0;
    delete[] jdbOutput_;

    emit dbgStatus (i18n("Debugger stopped"), state_);
}

// **************************************************************************

void JDBController::reConfig()
{
    KConfig *config = JavaDebuggerFactory::instance()->config();
    config->setGroup("Debug");
    Q_ASSERT(!config->readBoolEntry("Use external debugger", false));

    bool old_displayStatic        = config_displayStaticMembers_;
    config_displayStaticMembers_  = config->readBoolEntry("Display static members", false);


    if (( old_displayStatic != config_displayStaticMembers_)&&
        dbgProcess_) {
        bool restart = false;
        if (stateIsOn(s_appBusy)) {
            setStateOn(s_silent);
            pauseApp();
            restart = true;
        }


        if (restart)
            queueCmd(new JDBCommand("cont", RUNCMD, NOTINFOCMD, 0));
    }
}

// **************************************************************************

// Fairly obvious that we'll add whatever command you give me to a queue
// If you tell me to, I'll put it at the head of the queue so it'll run ASAP
// Not quite so obvious though is that if we are going to run again. then any
// information requests become redundent and must be removed.
// We also try and run whatever command happens to be at the head of
// the queue.
void JDBController::queueCmd(DbgCommand *cmd, bool executeNext)
{
    // Output command info for debugging purposes
    // DBG_DISPLAY("Queue: " + cmd->command_);

    // We remove any info command or _run_ command if we are about to
    // add a run command.
    if (cmd->isARunCmd())
        removeInfoRequests();

    if (executeNext)
        cmdList_.insert(0, cmd);
    else
        cmdList_.append (cmd);

    executeCmd();
}

// **************************************************************************
// If the appliction can accept a command and we've got one waiting
// then send it.
// Commands can be just request for data (or change jdbs state in someway)
// or they can be "run" commands. If a command is sent to jdb our internal
// state will get updated.
void JDBController::executeCmd()
{

    if (stateIsOn(s_dbgNotStarted|s_waitForWrite|s_appBusy)) {
        return;
   }


    if (!currentCmd_) {
        if (cmdList_.isEmpty()) {
            DBG_DISPLAY("Commandlist empty...\n");
            return;
        }

        currentCmd_ = cmdList_.take(0);
    } else {

      if (!currentCmd_->moreToSend()) {
          if (currentCmd_->expectReply()) {
              return;
          }

          delete currentCmd_;
          if (cmdList_.isEmpty()) {
              currentCmd_ = 0;

              return;
          }

          currentCmd_ = cmdList_.take(0);
      }
    }

    Q_ASSERT(currentCmd_ && currentCmd_->moreToSend());

    // Output command info for debugging purposes
    dbgProcess_->writeStdin(currentCmd_->cmdToSend().data(), currentCmd_->cmdLength());
    setStateOn(s_waitForWrite);

    if (currentCmd_->isARunCmd()) {
        setStateOn(s_appBusy);
        setStateOff(s_appNotStarted|s_programExited|s_silent);
    }

    JDB_DISPLAY("Written command: " + currentCmd_->cmdToSend());
    if (!stateIsOn(s_silent))
        emit dbgStatus ("", state_);
}

// **************************************************************************

void JDBController::destroyCmds()
{
    if (currentCmd_) {
        delete currentCmd_;
        currentCmd_ = 0;
    }

    while (!cmdList_.isEmpty())
        delete cmdList_.take(0);
}

// **********************************************************************

void JDBController::removeInfoRequests()
{
    int i = cmdList_.count();
    while (i) {
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
void JDBController::pauseApp()
{
}

// **********************************************************************

// Whenever the program pauses we need to refresh the data visible to
// the user. The reason we've stooped may be passed in to be emitted.
void JDBController::actOnProgramPause(const QString &msg)
{
    // We're only stopping if we were running, of course.
    if (stateIsOn(s_appBusy)) {
       DBG_DISPLAY("Acting on program paused: " + msg);
       setStateOff(s_appBusy);
       // We're always at frame zero when the program stops
       // and we must reset the active flag
       currentFrame_ = 0;
       varTree_->setActiveFlag(); //               ????????????????????????????????????
       // These two need to be actioned immediately. The order _is_ important
       emit dbgStatus("", state_);

       stackLineCount = 0;

       frameStack_->clearList();

       setStateOn(s_parsingOutput);
       queueCmd(new JDBCommand("where", NOTRUNCMD, NOTINFOCMD, BACKTRACE), TRUE);

       executeCmd();

       varLineCount = 0;
       // delete localData
       localData.clear();
       nameQueue.clear(); // should actually be empty already


       setStateOn(s_parsingOutput);
       parsedThis = FALSE;
       queueCmd(new JDBCommand("locals", NOTRUNCMD, INFOCMD, LOCALS), FALSE);
       executeCmd();

    } else { kdDebug() << "Not running\n";}
}

// **************************************************************************

// There is no app anymore. This can be caused by program exiting
// an invalid program specified or ...
// jdb is still running though, but only the run command (may) make sense
// all other commands are disabled.
void JDBController::programNoApp(const QString &, bool)
{
}

// **************************************************************************

enum lineStarts
    {
// Note that these values will differ according to the endianess of the architecture.
// A bad idea surely?
// On Intel:
//      START_Brea  = 0x61657242,
// On PowerPC:
        START_Brea  = 0x42726561,

        START_Step  = 0x70657453
    };


// Any data that isn't "wrapped", arrives here. Rather than do multiple
// string searches until we find (or don't find!) the data,
// we break the data up, depending on the first 4 four bytes, treated as an
// int. Hence those big numbers you see above.
char* JDBController::parseLine(char *buf)
{

    /// @todo ignore empty lines


    // Doing this copy should remove any alignment problems that
    // some systems have (eg Solaris).
	// - jbb?

	// I don't believe this will work on SPARC as the bytes are round the other way, it
	// certainly doesn't work on a PowerPC machine.
	//	- Richard Dale

//    int sw;
//    memcpy (&sw, buf, sizeof(int));

	if (memcmp(buf, "Brea", 4) == 0) {
            kdDebug() << "Checking for breakpoint\n";
            if ((strncmp(buf, "Breakpoint hit: thread", 22) == 0)) {
                QRegExp ex( "Breakpoint hit: thread=\\\"(.*)\\\", (.*\\)), line=([0-9]*), bci\\=[0-9]*.*\\n[^\\[]*\\[[0-9]*\\] ");
                if (ex.search( buf) != -1 ) {
                    DBG_DISPLAY(QString("Breakpoint hit in line ") + ex.cap(3));
                    if (stateIsOn(s_appStarting)) {
                        setStateOff(s_appStarting);
                    }
                    curMethod = ex.cap(2),
                    curLine = ex.cap(3);


                    if (currentCmd_ && currentCmd_->isARunCmd()) {
                        delete currentCmd_;
                        currentCmd_ = 0;
                        kdDebug() << "Deleting step command\n";
                    }


                    emit showStepInSource(QString(classpath_ + "/" + mainclass_ + ".java").latin1(),
                                          ex.cap(3).toInt(), "");
                    actOnProgramPause(QString("Reached Breakpoint in line ")+ex.cap(3));

                    char *retStr;
		    QString retQString =  QString( buf + ex.cap(ex.numCaptures()) );
		    memcpy( retStr, retQString.latin1(), retQString.length() );
                    return retStr;
                }

            }
    } else if (memcmp(buf, "Step", 4) == 0) {
            if ((strncmp(buf, "Step completed:", 15) == 0)) {
                kdDebug() << "STEP: " << buf << endl;
                QRegExp ex( " thread=\\\"(.*)\\\", (.*\\)), line=([0-9]*)");
                if (ex.search( buf) != -1 ) {
                    kdDebug() << "MATCH\n";
                    curMethod = ex.cap(2),
                    curLine = ex.cap(3);


                    if (currentCmd_ && currentCmd_->typeMatch(STEP)) {
                        delete currentCmd_;
                        currentCmd_ = 0;
                        DBG_DISPLAY("Deleting step command");
                    }

                    QString curClass = QString(ex.cap(2)).left(QString(ex.cap(2)).findRev("."));
                    QString curFile = getFile(curClass);
                    kdDebug() << "Filename: " << curFile <<endl;
                    emit showStepInSource(curFile, atoi(ex.cap(3).latin1() ), "");

                    actOnProgramPause(QString("step completed, stopped in ") + ex.cap(2));

                    return buf + QString(buf).length();
                }

            }

    }

	return 0;
}

// **********************************************************************

char* JDBController::parseInfo(char *buf)
{

    // Every string we are looking for has to match at the
    // beginning of buf!
    if (currentCmd_ && currentCmd_->typeMatch(BACKTRACE)) {
        return parseBacktrace(buf);
    } else if (currentCmd_ && currentCmd_->typeMatch(LOCALS)) {
        setStateOn(s_parsingLocals);
        return parseLocalVars(buf);
    } else if (currentCmd_ && currentCmd_->typeMatch(DATAREQUEST)) {
        setStateOn(s_parsingLocals);
        if (char* tmp = parseDump(buf)) { return tmp; }
    }

    return 0;
}


// **********************************************************************

char* JDBController::parseBacktrace(char* buf) {
    QRegExp* exp = 0;
    // Check for a new line of stacktrace output first
    exp = new QRegExp( "^ \\[[0-9]+\\][^\\)]+\\)");
    if (exp->search( buf) != -1) {
        DBG_DISPLAY(QString("Found some stacktrace output"));
        frameStack_->addItem(exp->cap(0).latin1() );
        stackLineCount++;
       
	QString retQString =  QString( buf + exp->cap(exp->numCaptures()) );
        memcpy( buf, retQString.latin1(), retQString.length() );

        delete exp;
        return buf;
    }

    // If that fails we check if the standard prompt is displayed
    if (stackLineCount > 0) {
    kdDebug() << ">" << *buf<<endl;
    exp->setPattern("^[^ ]+\\[[0-9]+\\]");
    if (exp->search( buf) != -1) {
        DBG_DISPLAY(QString("Found end of stacktrace (prompt)"));

        if (currentCmd_ && currentCmd_->typeMatch(BACKTRACE)) {
            delete currentCmd_;
            currentCmd_ = 0;
        }

        setStateOff(s_parsingOutput);

        frameStack_->updateDone();

        QString retQString =  QString( buf + exp->cap(exp->numCaptures()) );
        memcpy( buf, retQString.latin1(), retQString.length() );

        delete exp;
        return buf;

    }
    }

    // we know there will be a stack trace, so we just need to wait for
    // more data flowing in
    delete exp;
    return 0;
}

// **********************************************************************


char* JDBController::parseLocalVars(char* buf) {
    QRegExp* exp = 0;
    exp = new QRegExp( "^Local variable information not available. Compile with -g to generate variable information\n");
    if (exp->search( buf) != -1) {
        DBG_DISPLAY(QString("No var info available"));
        if (currentCmd_ && currentCmd_->typeMatch(LOCALS)) {
            delete currentCmd_;
            currentCmd_ = 0;
        }
        varUpdateDone();

        QString retQString =  QString( buf + exp->cap(exp->numCaptures()) );
        memcpy( buf, retQString.latin1(), retQString.length() );

        delete exp;
        return buf;
    }

    exp->setPattern( "^No local variables");
    if (exp->search( buf) != -1) {
        DBG_DISPLAY(QString("No locals"));

        // wait for prompt
        QString retQString =  QString( buf + exp->cap(exp->numCaptures()) );
        memcpy( buf, retQString.latin1(), retQString.length() );

        delete exp;
        return buf;
    }

    // Seems as if Java outputs some very strange spaces sometimes
    // or \s is partly broken in QRegExp
    exp->setPattern( "^  ([^ ]+) \\= ([^\\(\n]+)\\s*\\(id\\=[0-9]*\\)");
    if (exp->search( buf) != -1 ) {
        DBG_DISPLAY(QString("Var info:"));
        varLineCount++;
        kdDebug() << "Name: " << exp->cap(1) << endl;
        kdDebug() << "Type: " << exp->cap(2) << endl; // Remove possible trailing whitespace

        // Queue current var for processing.
        // kdDebug() << "APPENDING: " << exp->cap(1) << endl;
        nameQueue.append(exp->cap(1));

        QString retQString =  QString( buf + exp->cap(exp->numCaptures()) );
        memcpy( buf, retQString.latin1(), retQString.length() );

        delete exp;
        return buf;

    }

    exp->setPattern("^  ([^ ]+) \\= ([^\n]+)");
    if (exp->search( buf) != -1) {
        DBG_DISPLAY(QString("Local Var info:"));

        varLineCount++;
        kdDebug() << "Name: " << exp->cap(1) << endl;
        kdDebug() << "Type: " << exp->cap(2) << endl; // Remove possible trailing whitespace

        // primitive type, add directly
        analyzeDump(exp->cap(0));

        QString retQString =  QString( buf + exp->cap(exp->numCaptures()) );
        memcpy( buf, retQString.latin1(), retQString.length() );

        delete exp;
        return buf;

    }


    exp->setPattern("^([^ ]+)\\[[0-9]+\\] ");
    if (exp->search(buf) != -1 ) {
        DBG_DISPLAY(QString("Found end of var dump (prompt)"));
        kdDebug() << ">" << exp->cap(0) << "<\n";
        if (currentCmd_ && currentCmd_->typeMatch(LOCALS)) {
            delete currentCmd_;
            currentCmd_ = 0;
        }

        if (currentCmd_ && currentCmd_->typeMatch(LOCALS)) {
            delete currentCmd_;
            currentCmd_ = 0;
        }
        QString retQString =  QString( buf + exp->cap(exp->numCaptures()) );
        memcpy( buf, retQString.latin1(), retQString.length() );

        delete exp;
        return buf;


    }
    delete exp;

    return 0;
}

// **********************************************************************


char* JDBController::parseDump(char* buf) {
    // Looking for dump output
    QRegExp *exp;

    // compound object
    exp = new QRegExp( "^([^ ]+) \\= ([^\\(]+)\\s*\\(id\\=[0-9]*\\) \\{([^\\}]+)\\}");
    if (exp->search( buf) != -1 ) {
        DBG_DISPLAY(QString("Found dump info"));

        analyzeDump(exp->cap(0));

        if (currentCmd_ && currentCmd_->typeMatch(DATAREQUEST)) {
            delete currentCmd_;
            currentCmd_ = 0;
        }

        QString retQString =  QString( buf + exp->cap(exp->numCaptures()) );
        memcpy( buf, retQString.latin1(), retQString.length() );
        delete exp;
        return buf;
    }

    // Array element
    exp->setPattern("^ ([^\\[]+\\[[0-9]+\\]) \\= ([^\n]+)");
    if (exp->search( buf) != -1 ) {
        DBG_DISPLAY(QString("Found dump info"));
        kdDebug() << "Array element: " << exp->cap(1) << " - " << exp->cap(2) << endl;
        analyzeDump(exp->cap(0));

        if (currentCmd_ && currentCmd_->typeMatch(DATAREQUEST)) {
            delete currentCmd_;
            currentCmd_ = 0;
        }

        QString retQString =  QString( buf + exp->cap(exp->numCaptures()) );
        memcpy( buf, retQString.latin1(), retQString.length() );

        delete exp;
        return buf;
    }

    exp->setPattern("^No 'this'.  In native or static method\n");
    if (exp->search( buf) != -1) {

        if (currentCmd_ && currentCmd_->typeMatch(DATAREQUEST)) {
            delete currentCmd_;
            currentCmd_ = 0;
        }

        QString retQString =  QString( buf + exp->cap(exp->numCaptures()) );
        memcpy( buf, retQString.latin1(), retQString.length() );

        delete exp;
        return buf;
    }

    delete exp;
    return 0;
}




// **************************************************************************

/*
 * There aren't too many possibilities here:
 *  a) Object is of primitive type: data should match something like name: value(id=xxx)
 *  b) Object is an array element of primitive type: name[index] = value
 *  b) Object is of compound type: name: instance of package.classname(id=xxx) { properties }
 *     where properties can be variables of primitive type (a) or are in the form
 *     name: instance of type name or are arrays of either primitive or non primitive types
 *  c) Object is an array of primitive type objects: name: instance of type[dimension] (id=xxx)
 *  d) Object is an array of compound objects:
*/
void JDBController::analyzeDump(QString data)
{

  kdDebug() << "Parsing dump: " << data << endl;

  // case a
  // if we have a primitive type we add it to the list of locals directly
  QRegExp *exp = new QRegExp( "^  ([^ \\[]+) \\= ([^\n]+)"); // dup if it's really a var of primitive type
  if (exp->search(data.latin1()) != -1 ) {
      QString name = exp->cap(1);
      QString value = exp->cap(2);
      JDBVarItem *item = new JDBVarItem();
      item->value = value;
      item->name = name;
      if (!localData[name]) {
          kdDebug() << "inserting local var" << endl;
          localData.insert(name, item);
      } else { /* The object is already being referred to as a property */ }
      delete exp;
      return;

  }

  exp->setPattern( " ([^ \\[]+)\\[([0-9]+)\\] \\= ([^\n]+)");
  if (exp->search(data.latin1()) != -1) {
      kdDebug() << "Array element: " << exp->cap(1) << "[" << exp->cap(2)<< "] = " << exp->cap(3) << endl;

      QString name = exp->cap(1);

      JDBVarItem *item;
      JDBVarItem *subItem = new JDBVarItem();
      subItem->name = QString(exp->cap(1)) + "[" + QString(exp->cap(2)) + "]";
      subItem->value = exp->cap(3);
      item = localData[name];
      Q_ASSERT((name != 0));

      //kdDebug() << "->Appending to: " << name << endl;
      //kdDebug() << "Which is at: " << (int)item << endl;

      // item.insertSibling(subItem);
      item->siblings.append(subItem);

      delete exp;
      return;
  }



  exp->setPattern( "^([^ ]+) \\= instance of ([^[]+)\\[([0-9])+] \\(id\\=[0-9]+\\) {");
  if (exp->search(data.latin1()) != -1 ) {
      kdDebug() << "Array...\n";
      kdDebug() << "Name: " << exp->cap(1) << endl;
      kdDebug() << "Type: " << exp->cap(2) << endl;
      kdDebug() << "Dimension: " << exp->cap(3) << endl;

      kdDebug() << "Adding array to var tree... \n";
      JDBVarItem *item = new JDBVarItem();
      item->name = exp->cap(1);
      QString name = exp->cap(1);
      if (atoi(exp->cap(3).latin1()) == 0) {
          item->value="null";
      }
      if (!localData[name]) {
          kdDebug() << "inserting local var " << name << " at " << (long)item << endl;
          localData.insert(name, item);
      } else { /* The object is already being referred to as a property */ }

      for (int i=0; i<atoi(exp->cap(3).latin1()); i++) {
          kdDebug() <<  QString(exp->cap(1)) + QString("[") + QString::number((i)) + QString("]") << endl;
          nameQueue.append(QString(exp->cap(1)) + QString("[") + QString::number((i)) + QString("]"));
      }

      delete exp;
      return;
  }

  // case b
  // otherwise we need to extract all properties and add the name of the current var
  // and link it with its properties
  exp->setPattern( "^([^ ]+) \\= ([^\\(]+)\\s*\\(id\\=[0-9]*\\) \\{([^\\}]+)\\}");
  if (exp->search(data.latin1()) != -1) {
      kdDebug() << "COMPOUND DATA" << endl;
      // create a new jdbvaritem for the name of the object and for each property it
      // contains. the object's jdbvaritem has siblings then
      QString name = exp->cap(1);
      JDBVarItem *item;
      if (!localData[name]) {
          /** oops, we should already have added that object @todo insert assertion */
          item = new JDBVarItem();
          kdDebug() << "NAME: " << name << " - " << (long)item << endl;
          item->name = name;
          localData.insert(name, item);
      } else {
          /* The object is already being referred to as a property */
          item = localData[name];
      }



      unsigned int i = data.find("{")+1;
      exp = new QRegExp("^([^ \\:]+): ([^\n]+)");
      QString tmp;
      while (i<data.length()) {
         // I guess this is really slow. Using a char* would be better here
         if (exp->search(data.mid(i).latin1()) != -1) {
             if (strncmp(exp->cap(2).latin1(), "instance of", 11) != 0) {
                 // property of primitive type
                 kdDebug() << "Primitive type..." << endl;
                 QString fullName = name + QString(".") + QString(exp->cap(1));

                 // create new item
                 JDBVarItem *subItem = new JDBVarItem();
                 subItem->value = exp->cap(2);
                 subItem->name = fullName;
                 if (!localData[fullName]) {
                     localData.insert(fullName, subItem);
                 } else { /* Oops */ }  /// @todo insert assertion

                 // item.insertSibling(subItem);
                 item->siblings.append(subItem);

                 //DBG_DISPLAY(QString("Appending Name: ") + name + QString(".") + exp->cap(1));
                 //DBG_DISPLAY(QString("Value: ") + exp->cap(2));
             } else if (QString(exp->cap(2)).contains("[")) {
                 // Array property
                 // not parsed yet. just insert some dummy data

                 QString fullName = name + QString(".") + QString(exp->cap(1));

                 JDBVarItem *subItem = new JDBVarItem();
                 subItem->name = fullName;
                 if (!localData[fullName]) {
                     localData.insert(fullName, subItem);
                 } else { /* Oops */ }  /// @todo insert assertion

                 //kdDebug() << "->Appending Name: " << name << "." << exp->cap(1) << endl;
                 //kdDebug() << "Value: " << exp->cap(2) << " as " << (int)subItem << endl;

                 // get array dimension andn queue elements for parsing
                 QRegExp *exp2 = new QRegExp("\\[([0-9]+)\\]");
                 if (exp2->search(exp->cap(2)) != -1 ) {
                     int dimension = atoi(exp2->cap(1).latin1());
                     kdDebug() << "Array dimension: " << dimension << endl;
                     for (int i=0; i<dimension; i++) {
                         nameQueue.append(fullName + "[" + QString::number((i)) + "]");
                     }

                     // item.insertSibling(subItem);
                     item->siblings.append(subItem);
                 }

             } else {
                 // property of non-primitive type, we will request additional
                 // information later
                 kdDebug() << "complex..." << endl;
                 QString fullName = name + QString(".") + QString(exp->cap(1));
                 nameQueue.append(fullName);

                 JDBVarItem *subItem = new JDBVarItem();
                 subItem->name = fullName;
                 if (!localData[fullName]) {
                     localData.insert(fullName, subItem);
                 } else { /* Oops */ }  /// @todo insert assertion

                 kdDebug() << "appending: " << fullName << " as " << (long)subItem << endl;

                 // item.insertSibling(subItem);
                 item->siblings.append(subItem);


             }
             i += atoi( exp->cap(exp->numCaptures()).latin1() );
         } else {
             i++;
         }
      }

  }

  delete exp; exp = 0;
}


// If there are elements left in the queue we remove the first one and issue
// a dump command
void JDBController::parseLocals()
{
    if (!stateIsOn(s_parsingLocals)) { return; }
    if (currentCmd_) { return; }
    DBG_DISPLAY("Trying to continue with locals");
    if (!nameQueue.isEmpty()) {
        DBG_DISPLAY("Issueing newdump command");
        QString varName = nameQueue.first();
        nameQueue.remove(nameQueue.begin());
        // kdDebug() << nameQueue.count() << endl;
        queueCmd(new JDBCommand(QString("dump " + varName).latin1(), NOTRUNCMD, INFOCMD, DATAREQUEST), FALSE);
    } else if (!parsedThis) {
        parsedThis = TRUE;
        queueCmd(new JDBCommand(QCString("dump this"), NOTRUNCMD, INFOCMD, DATAREQUEST), FALSE);

    } else {
       parsedThis = FALSE;
       setStateOff(s_parsingLocals);

       varUpdateDone();
    }
}

// **************************************************************************

// parsing the backtrace list will cause the vartree to be refreshed
void JDBController::parseBacktraceList(char *)
{
}

// **************************************************************************

// If the user gives us a bad program, catch that here.
//void JDBController::parseFileStart(char *buf)
//{
//  if (strstr(buf, "not in executable format:") ||
//      strstr(buf, "No such file or directory."))
//  {
//    programNoApp(QString(buf), true);
//    DBG_DISPLAY("Bad file start <" + QString(buf) + ">");
//  }
//}

// **************************************************************************

// Select a different frame to view. We need to get and (maybe) display
// where we are in the program source.
void JDBController::parseFrameSelected(char *)
{
}

// **************************************************************************



char *JDBController::parse(char *buf)
{

    if (stateIsOn(s_dbgNotStarted)) {
        kdDebug() << "dbgnotstarted" << endl;
        // Check for first prompt
        kdDebug() << QString(buf).left(20) << endl;
        if (QString(buf).left(20) == "Initializing jdb...\n") { return buf+20; }
        if (QString(buf) == "> ") {
            setStateOff(s_dbgNotStarted);
            emit debuggerStarted();
            return buf + 2;
        }
        curLine = "";
        return buf++;
    }


    if (stateIsOn(s_appStarting)) {
        kdDebug() << "appstarting" << endl;
        char* unparsed = buf;
        char* orig = buf;
        while (*buf) {
            if ( (buf = parseLine(buf)) ) {
                return buf;
            } else {
                buf = ++unparsed;
            }
        }
        return orig;
    }


    // If the app is currently running eat all output
    // until we recognize something
    if (stateIsOn(s_appBusy)) {
        kdDebug() << "\nApp busy:" << endl;
        char* unparsed = buf;
        char* orig = buf;
        while (*buf) {
            if ( (buf = parseLine(unparsed)) ) {
                return buf; // application is stopped now
                            // additional output will be ignored for
                            // now. we parse it later on
            } else {
                buf = ++unparsed;
            }
        }
        return orig;

    } else {
        // assuming app is paused
        kdDebug() << "\nApp is paused:" << endl;
        char* unparsed = buf;
        char* orig = buf;
        while (*buf) {
            if ( (buf = parseInfo(buf)) ) {
                unparsed = orig = buf;
            } else {
                buf = ++unparsed;
            }
        }
        // Check if there are more vars to parse, otherwise update vartree widget
        parseLocals();

        return orig;

    }

    /*
    while (*unparsed) {
        char *parsed;
        if (*unparsed == (char)BLOCK_START)
            parsed = parseCmdBlock(unparsed);
        else
            parsed = parseOther(unparsed);

        if (!parsed)
            break;

        // Move one beyond the end of the parsed data
        unparsed = parsed+1;
    }
    */
//    return (unparsed==buf) ? 0 : unparsed;
}

// **************************************************************************

void JDBController::setBreakpoint(const QCString &/*BPSetCmd*/, int /*key*/)
{
//    queueCmd(new JDBSetBreakpointCommand("", key));
}

// **************************************************************************

void JDBController::clearBreakpoint(const QCString &/*BPClearCmd*/)
{
//    queueCmd(new JDBCommand("info breakpoints", NOTRUNCMD, NOTINFOCMD, BPLIST));
}

// **************************************************************************

void JDBController::modifyBreakpoint(Breakpoint *)
{
}

// **************************************************************************
//                                SLOTS
//                                *****
// For most of these slots data can only be sent to jdb when it
// isn't busy and it is running.
// **************************************************************************

/**
 * Start a JDB debugging session
*/
void JDBController::slotStart(const QString &/*application*/, const QString &args, const QString &sDbgShell)
{
    Q_ASSERT (!dbgProcess_ && !tty_);

    // Remove .class suffix and leading path information from appname
    // (should not be there anyway)

    tty_ = new STTY(config_dbgTerminal_, "konsole");
    if (!config_dbgTerminal_) {
        connect( tty_, SIGNAL(OutOutput(const char*)), SIGNAL(ttyStdout(const char*)) );
        connect( tty_, SIGNAL(ErrOutput(const char*)), SIGNAL(ttyStderr(const char*)) );
    }

    QString tty(tty_->getSlave());

    if (tty.isEmpty()) {
        KMessageBox::error(0, i18n("JDB cannot use the tty* or pty* devices.\n"
                                   "Check the settings on /dev/tty* and /dev/pty*\n"
                                   "As root you may need to \"chmod ug+rw\" tty* and pty* devices "
                                   "and/or add the user to the tty group using "
                                   "\"usermod -G tty username\""));

        delete tty_;
        tty_ = 0;
        return;
    }

	JDB_DISPLAY("\nStarting JDB - app:["+mainclass_+"] classpath:["+classpath_+"] args:["+args+"] sDbgShell:["+sDbgShell+"]\n");
	dbgProcess_ = new KProcess;

    connect( dbgProcess_, SIGNAL(receivedStdout(KProcess *, char *, int)),
             this,        SLOT(slotDbgStdout(KProcess *, char *, int)) );

    connect( dbgProcess_, SIGNAL(receivedStderr(KProcess *, char *, int)),
             this,        SLOT(slotDbgStderr(KProcess *, char *, int)) );

    connect( dbgProcess_, SIGNAL(wroteStdin(KProcess *)),
             this,        SLOT(slotDbgWroteStdin(KProcess *)) );

    connect( dbgProcess_, SIGNAL(processExited(KProcess*)),
             this,        SLOT(slotDbgProcessExited(KProcess*)) );


    if (!sDbgShell.isEmpty())
        *dbgProcess_<<"/bin/sh"<<"-c"<<sDbgShell+" "+config_jdbPath_+
            QString("jdb")<<"-classpath"<<classpath_<<mainclass_<<args;
    else
        *dbgProcess_<<config_jdbPath_+QString("jdb")<<"-classpath"<<classpath_<<mainclass_<<args;

    if (! dbgProcess_->start(	KProcess::NotifyOnExit,
                        		KProcess::Communication(KProcess::All) ) )
	{
		JDB_DISPLAY("\nFailed to start JDB - app:["+mainclass_+"]\n");
	}

    // JDB takes some time to start up
    setStateOn(s_dbgNotStarted);
    emit dbgStatus ("", state_);

    // Initialise jdb. At this stage jdb is sitting wondering what to do,
    // and to whom. Organise a few things, then set up the tty for the application,
    // and the application itself

}

// **************************************************************************

void JDBController::slotCoreFile(const QString&)
{
}


void JDBController::slotAttachTo(int)
{
}

// **************************************************************************

void JDBController::slotDebuggerStarted() {
    kdDebug() << "slotRun()" << endl;
    if (stateIsOn(s_appBusy|s_dbgNotStarted|s_shuttingDown))
        return;
    bool first_flag = FALSE;

    kdDebug() << "slotRun()" << endl;
    if (stateIsOn(s_appNotStarted)) {
        first_flag = TRUE;
        queueCmd(new JDBCommand(QString("stop in " + mainclass_ + ".main(java.lang.String[])" ).latin1() , NOTRUNCMD, NOTINFOCMD, 0 ) );
    }

    queueCmd(new JDBCommand(stateIsOn(s_appNotStarted) ? "run" : "cont", RUNCMD, NOTINFOCMD, 0));

    // Signal we are waiting for the first breakpoint to be reached
    setStateOn(s_appStarting);
}

void JDBController::slotRun()
{
    if (stateIsOn(s_dbgNotStarted)) {
        // Connect to debuggerStarted slot to wait for jdb to finish
        // initialization
        connect(this, SIGNAL(debuggerStarted()), SLOT(slotDebuggerStarted()));
    } else {
//        emit debuggerStarted();
    }

}

// **************************************************************************

void JDBController::slotRunUntil(const QString &, int)
{
}

// **************************************************************************

void JDBController::slotStepInto()
{
    kdDebug() << "slotStepInstruction" << endl;
    if (stateIsOn(s_dbgNotStarted) || stateIsOn(s_appBusy) || stateIsOn(s_parsingOutput)) { return; }
    queueCmd(new JDBCommand("stepi", RUNCMD, NOTINFOCMD, 0));
}

// **************************************************************************

void JDBController::slotStepIntoIns()
{
}

// **************************************************************************

void JDBController::slotStepOver() {
    kdDebug() << "slotStepOver" << endl;
    if (stateIsOn(s_appStarting) || stateIsOn(s_dbgNotStarted) || stateIsOn(s_appBusy) || stateIsOn(s_parsingOutput)) { return; }
    queueCmd(new JDBCommand("step", RUNCMD, NOTINFOCMD, 0));
}

// **************************************************************************

void JDBController::slotStepOverIns()
{
}

// **************************************************************************

void JDBController::slotStepOutOff()
{
}

// **************************************************************************

// Only interrupt a running program.
void JDBController::slotBreakInto()
{
}

// **************************************************************************

// See what, if anything needs doing to this breakpoint.
void JDBController::slotBPState(Breakpoint *)
{
}

// **************************************************************************

void JDBController::slotClearAllBreakpoints()
{
}

// **************************************************************************

void JDBController::slotDisassemble(const QString &, const QString &)
{
}

// **************************************************************************

void JDBController::slotMemoryDump(const QString &, const QString &)
{
}

// **************************************************************************

void JDBController::slotRegisters()
{
}

// **************************************************************************

void JDBController::slotLibraries()
{
}

// **************************************************************************

void JDBController::slotSelectFrame(int)
{
}

// **************************************************************************

// This is called when the user desires to see the details of an item, by
// clicking open an varItem on the varTree.
void JDBController::slotExpandItem(VarItem *)
{
}

// **************************************************************************

// This is called when an item needs special processing to show a value.
// Example = QStrings. We want to display the QString string against the var name
// so the user doesn't have to open the qstring to find it. Here's where that happens
void JDBController::slotExpandUserItem(VarItem *, const QCString& )
{
}

// **************************************************************************

// The user will only get locals if one of the branches to the local tree
// is open. This speeds up stepping through code a great deal.
/// @todo
void JDBController::slotSetLocalViewState(bool)
{
}

// **************************************************************************

// Data from jdb gets processed here.
void JDBController::slotDbgStdout(KProcess *, char *buf, int buflen)
{
    // Allocate some buffer space, if adding to this buffer will exceed it
    if (jdbOutputLen_+buflen+1 > jdbSizeofBuf_) {
        jdbSizeofBuf_ = jdbOutputLen_+buflen+1;
        char *newBuf = new char[jdbSizeofBuf_];     // ??? shoudn't this be malloc ???
        if (jdbOutputLen_)
            memcpy(newBuf, jdbOutput_, jdbOutputLen_+1);
        delete[] jdbOutput_;                        // ??? and free ???
        jdbOutput_ = newBuf;
    }

    // Copy the data out of the KProcess buffer before it gets overwritten
    // and fake a string so we can use the string fns on this buffer
    memcpy(jdbOutput_+jdbOutputLen_, buf, buflen);
    jdbOutputLen_ += buflen;
    *(jdbOutput_+jdbOutputLen_) = 0;

    if (char *nowAt = parse(jdbOutput_)) {
        Q_ASSERT(nowAt <= jdbOutput_+jdbOutputLen_+1);
        jdbOutputLen_ = strlen(nowAt);
        // Some bytes that weren't parsed need to be moved to the head of the buffer
        if (jdbOutputLen_)
            memmove(jdbOutput_, nowAt, jdbOutputLen_);     // Overlapping data

    }
    // check the queue for any commands to send
    executeCmd();
    kdDebug() << "stdout" << endl;
}

// **************************************************************************

void JDBController::slotDbgStderr(KProcess */*proc*/, char *buf, int/* buflen*/)
{
    // At the moment, just drop a message out
    // dont and redirect
    kdDebug() << "STDERR\n";
    DBG_DISPLAY(QString("\nSTDERR: ")+QString(buf));
//    slotDbgStdout(proc, buf, buflen);
}

// **************************************************************************

void JDBController::slotDbgWroteStdin(KProcess *)
{
    setStateOff(s_waitForWrite);
    executeCmd();
    kdDebug() << "dbgwrotestdin" << endl;
}

// **************************************************************************

void JDBController::slotDbgProcessExited(KProcess*)
{
    destroyCmds();
    state_ = s_appNotStarted|s_programExited|(state_&s_viewLocals);
    emit dbgStatus (i18n("Process exited"), state_);

    JDB_DISPLAY(QString("\n(jdb) Process exited"));
}

// **************************************************************************


/**
 * This function returns the absolute position of the source file
 * for the class specified by the parameter className.
 * @todo Make this function work by using the java sourcepath or
 * the KDevelop class store.
*/
QString JDBController::getFile(QString className)
{
	return QString(projectDirectory_ + "/" + className + ".java");
}

// **********************************************************************

void JDBController::varUpdateDone()
{
    kdDebug() << "VarUpdateDone" << endl;

    QString locals = "";
    QDictIterator<JDBVarItem> it(localData); // iterator for dict
    if (!it.toFirst()) { return; }
    // make sure we dont visit nodes more than once
    while (it.current()) {

       if (!it.currentKey().contains(".")) {
           locals += it.current()->toString() + QString(",");
       }
       ++it;
    }

    locals[locals.length()-1] = ' '; // remove trailing comma
    char* _l = new char[locals.length()];
    strcpy(_l, locals.latin1());
    kdDebug() << "\nLocals: "<< _l << endl;


    varTree_->trim();

    FrameRoot *frame;
    // The locals are always attached to the currentFrame
    // so make sure we have one of those.
    if (!(frame = varTree_->findFrame(currentFrame_)))
        frame = new FrameRoot(varTree_, currentFrame_);

    Q_ASSERT(frame);
    frame->setFrameName(frameStack_->getFrameName(currentFrame_));

    frame->setLocals(_l);

    varTree_->viewport()->setUpdatesEnabled(true);
    varTree_->repaint();
    localData.clear();
    setStateOff(s_parsingOutput);

}


// The time limit has expired so set the state off.
void JDBController::slotAbortTimedEvent()
{
    setStateOff(s_waitTimer);
    DBG_DISPLAY(QString("Timer aborted\n"));
}

// **************************************************************************

void JDBController::slotStepInSource(const QString &fileName, int lineNum)
{
    DBG_DISPLAY((QString("(Show step in source) ")+fileName+QString(":")
                 +QString().setNum(lineNum)).local8Bit().data());
}

// **************************************************************************

void JDBController::slotDbgStatus(const QString &status, int state)
{
    QString s("(status) ");
    if (!state)
        s += QString("<program paused>");
    if (state & s_dbgNotStarted)
        s += QString("<dbg not started>");
    if (state & s_appNotStarted)
        s += QString("<app not started>");
    if (state & s_appBusy)
        s += QString("<app busy>");
    if (state & s_waitForWrite)
        s += QString("<wait for write>");
    if (state & s_programExited)
        s += QString("<program exited>");
    if (state & s_silent)
        s += QString("<silent>");
    if (state & s_viewLocals)
        s += QString("<viewing locals>");

    DBG_DISPLAY((s+status).local8Bit().data());
}


JDBVarItem::JDBVarItem() {
    value = "";
    name = "";
}

QString JDBVarItem::toString() {
    if (!value.isEmpty()) {
        kdDebug() << value <<" - ";
        return name + " = " + value;
    } else {
        // iterate over siblings and build return string
        QString tmp;
        JDBVarItem *item;

        for (item = this->siblings.first(); item != 0; item = this->siblings.next()) {
            tmp += item->toString() + ",";
            delete item;
        }

        tmp = name + " = {" +tmp;
        tmp[tmp.length()-1] = '}'; // remove trailing comma
        return tmp;
    }
}

}

// **************************************************************************
#include "jdbcontroller.moc"
