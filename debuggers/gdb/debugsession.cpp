/*
 * GDB Debugger Support
 *
 * Copyright 1999-2001 John Birch <jbb@kdevelop.org>
 * Copyright 2001 by Bernd Gehrmann <bernd@kdevelop.org>
 * Copyright 2006 Vladimir Prus <ghost@cs.msu.su>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2009 Niko Sams <niko.sams@gmail.com>
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

#include "debugsession.h"

#include <QtCore/QCoreApplication>

#include <KMessageBox>
#include <KLocalizedString>
#include <KToolBar>
#include <KParts/MainWindow>

#include <interfaces/idocument.h>
#include <interfaces/icore.h>
#include <interfaces/iuicontroller.h>
#include <util/processlinemaker.h>

#include "gdbcontroller.h"
#include "breakpointcontroller.h"
#include <interfaces/idocumentcontroller.h>
#include "stackmanager.h"
#include "stackmanager.h"
#include <QApplication>

namespace GDBDebugger {

DebugSession::DebugSession(GDBController* controller)
    : m_controller(controller)
    , m_gdbState(s_dbgNotStarted|s_appNotStarted)
    , m_sessionState(NotStartedState)
    , justRestarted_(false)
    , m_config(KGlobal::config(), "GDB Debugger")
{
    m_procLineMaker = new KDevelop::ProcessLineMaker(this);

    connect( m_procLineMaker, SIGNAL(receivedStdoutLines(const QStringList&)),
             this, SIGNAL(applicationStandardOutputLines(const QStringList&)));
    connect( m_procLineMaker, SIGNAL(receivedStderrLines(const QStringList&)),
             this, SIGNAL(applicationStandardErrorLines(const QStringList&)) );
    connect( m_controller, SIGNAL(showStepInSource(QString,int,QString)), SLOT(slotShowStepInSource(QString,int,QString)));
    setupController();
}

void DebugSession::slotShowStepInSource(const QString& file, int line, const QString& address)
{
    kDebug() << file << line << address;
    if (!file.isEmpty()) {
        // Debugger counts lines from 1
        emit showStepInSource(KUrl::fromPath(file), line-1);
    } else {
        emit clearExecutionPoint();
    }
}

KDevelop::StackModel* DebugSession::stackModel() const
{
    return m_controller->stackManager();
}

KDevelop::IBreakpointController* DebugSession::breakpointController() const
{
    return m_controller->breakpoints();
}


KDevelop::IDebugSession::DebuggerState DebugSession::state() const {
    return m_sessionState;
}

#define ENUM_NAME(o,e,v) (o::staticMetaObject.enumerator(o::staticMetaObject.indexOfEnumerator(#e)).valueToKey((v)))
void DebugSession::setSessionState(DebuggerState state)
{
    kDebug() << "STATE CHANGED" << state << ENUM_NAME(IDebugSession, DebuggerState, state);
    if (state != m_sessionState) {
        emit stateChanged(state);
        m_sessionState = state;
    }
}

void DebugSession::setupController()
{
    // variableTree -> gdbBreakpointWidget
//     connect( variableTree,          SIGNAL(toggleWatchpoint(const QString &)),
//              gdbBreakpointWidget,   SLOT(slotToggleWatchpoint(const QString &)));

    // controller -> this
    connect( m_controller,            SIGNAL(debuggerAbnormalExit()),
         this,                  SLOT(slotDebuggerAbnormalExit()));

    connect( m_controller, SIGNAL(stateChanged(DBGStateFlags, DBGStateFlags)),
             SLOT(gdbStateChanged(DBGStateFlags,DBGStateFlags)));

    connect(m_controller, SIGNAL(showMessage(const QString&, int)), this, SIGNAL(showMessage(const QString&, int)));

    // controller -> procLineMaker
    connect( m_controller,            SIGNAL(ttyStdout(const QByteArray&)),
             m_procLineMaker,         SLOT(slotReceivedStdout(const QByteArray&)));
    connect( m_controller,            SIGNAL(ttyStderr(const QByteArray&)),
             m_procLineMaker,         SLOT(slotReceivedStderr(const QByteArray&)));

//     connect(statusBarIndicator, SIGNAL(doubleClicked()),
//             controller, SLOT(explainDebuggerStatus()));

    // TODO: reimplement / re-enable
    //connect(this, SIGNAL(addWatchVariable(const QString&)), controller->variables(), SLOT(slotAddWatchVariable(const QString&)));
    //connect(this, SIGNAL(evaluateExpression(const QString&)), controller->variables(), SLOT(slotEvaluateExpression(const QString&)));

    connect(this, SIGNAL(toggleBreakpoint(const KUrl&, const KTextEditor::Cursor&)), m_controller->breakpoints(), SLOT(slotToggleBreakpoint(const KUrl&, const KTextEditor::Cursor&)));
}

void DebugSession::gdbStateChanged(DBGStateFlags oldState, DBGStateFlags newState)
{
    QString message;

    DBGStateFlags changedState = oldState ^ newState;

    if (changedState & s_dbgNotStarted) {
        if (newState & s_dbgNotStarted) {
            message = i18n("Debugger stopped");
            if (floatingToolBar)
                floatingToolBar->hide();

        } else {
            if (m_config.readEntry("Floating Toolbar", false))
            {
            #ifndef QT_MAC
                if (!floatingToolBar) {
                    floatingToolBar = new KToolBar(qApp->activeWindow());
                    floatingToolBar->show();
                }
            #endif
            }
        }

        //core()->running(this, false);
        // TODO enable/disable tool views as applicable
    }

    // As soon as debugger clears 's_appNotStarted' flag, we
    // set 'justRestarted' variable.
    // The other approach would be to set justRestarted in slotRun, slotCore
    // and slotAttach.
    // Note that setting this var in startDebugger is not OK, because the
    // initial state of debugger is exactly the same as state after pause,
    // so we'll always show varaibles view.
    if (changedState & s_appNotStarted) {
        if (newState & s_appNotStarted) {
            setSessionState(StoppedState);
            justRestarted_ = false;

        } else {

            if ( m_config.readEntry("Raise GDB On Start", false ) )
            {
                emit raiseOutputViews();
            }
            else
            {
                emit raiseFramestackViews();
            }

            setSessionState(ActiveState);
            justRestarted_ = true;
        }
    }

    if (changedState & s_explicitBreakInto)
        if (!(newState & s_explicitBreakInto))
            message = i18n("Application interrupted");

    if (changedState & s_programExited) {
        if (newState & s_programExited) {
            message = i18n("Process exited");
            setSessionState(StoppedState);
        }
    }

    if (changedState & s_appRunning) {
        if (newState & s_appRunning) {
            message = i18n("Application is running");
            setSessionState(ActiveState);
        }
        else
        {
            if (!(newState & s_appNotStarted)) {
                message = i18n("Application is paused");
                setSessionState(PausedState);

                // On the first stop, show the variables view.
                // We do it on first stop, and not at debugger start, because
                // a program might just successfully run till completion. If we show
                // the var views on start and hide on stop, this will look like flicker.
                // On the other hand, if application is paused, it's very
                // likely that the user wants to see variables.
                if (justRestarted_)
                {
                    justRestarted_ = false;
                    //mainWindow()->setViewAvailable(variableWidget, true);
                    emit raiseVariableViews();
                }
            }
        }
    }

    if (!(oldState & s_dbgNotStarted) && (newState & s_dbgNotStarted))
    {
        emit finished();
        m_job = 0;
    }

    // And now? :-)
    kDebug(9012) << "Debugger state: " << newState << ": ";
    kDebug(9012) << "   " << message;

    if (!message.isEmpty())
        emit showMessage(message, 3000);
}


bool DebugSession::startProgram(const KDevelop::IRun& run, KJob* job)
{
    m_job = job;
    return m_controller->startProgram(run, job);
}

void DebugSession::examineCoreFile(const KUrl& coreFile)
{
    m_controller->examineCoreFile(coreFile);
}

void DebugSession::attachToProcess(int pid)
{
    m_controller->attachToProcess(pid);
}

void DebugSession::startDebugger()
{
    //TODO, for now use startProgram
}

void DebugSession::run()
{
    m_controller->slotRun();
}
void DebugSession::stepOut()
{
    m_controller->slotStepOut();
}
void DebugSession::restartDebugger()
{
    m_controller->slotRestart();
}
void DebugSession::stopDebugger()
{
    m_controller->stopDebugger();
    emit reset();
}
void DebugSession::interruptDebugger()
{
    m_controller->slotPauseApp();
}
void DebugSession::runToCursor()
{
    if (KDevelop::IDocument* doc = KDevelop::ICore::self()->documentController()->activeDocument()) {
        KTextEditor::Cursor cursor = doc->cursorPosition();
        if (cursor.isValid())
            m_controller->runUntil(doc->url().path(), cursor.line() + 1);
    }
}
void DebugSession::jumpToCursor()
{
    if (KDevelop::IDocument* doc = KDevelop::ICore::self()->documentController()->activeDocument()) {
        KTextEditor::Cursor cursor = doc->cursorPosition();
        if (cursor.isValid())
            m_controller->jumpTo(doc->url().path(), cursor.line() + 1);
    }
}
void DebugSession::stepOver()
{
    m_controller->slotStepOver();
}
void DebugSession::stepOverInstruction()
{
    m_controller->slotStepOverInstruction();
}
void DebugSession::stepInto()
{
    m_controller->slotStepInto();
}
void DebugSession::stepIntoInstruction()
{
    m_controller->slotStepIntoInstruction();
}
void DebugSession::toggleBreakpoint()
{
    if (KDevelop::IDocument* document = KDevelop::ICore::self()->documentController()->activeDocument()) {
      KTextEditor::Cursor cursor = document->cursorPosition();

      if (!cursor.isValid())
        return;

      emit toggleBreakpoint(document->url(), cursor);
    }
}

void DebugSession::slotDebuggerAbnormalExit()
{
    emit raiseOutputViews();

    KMessageBox::information(
        KDevelop::ICore::self()->uiController()->activeMainWindow(),
        i18n("<b>GDB exited abnormally</b>"
             "<p>This is likely a bug in GDB. "
             "Examine the gdb output window and then stop the debugger"),
        i18n("GDB exited abnormally"));

    // Note: we don't stop the debugger here, becuse that will hide gdb
    // window and prevent the user from finding the exact reason of the
    // problem.
}

bool DebugSession::restartAvaliable() const
{
    if (m_controller->stateIsOn(s_attached) || m_controller->stateIsOn(s_core)) {
        return false;
    } else {
        return true;
    }
}


}

#include "debugsession.moc"
