/***************************************************************************
 *   Copyright (C) 1999-2001 by John Birch                                 *
 *   jbb@kdevelop.org                                                      *
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DEBUGGERPART_H_
#define _DEBUGGERPART_H_

#include <qguardedptr.h>
#include "kdevplugin.h"
#include "kdevcore.h"

namespace KParts { class Part; };

class QLabel;
class QPopupMenu;
class KDialogBase;
class ProcessWidget;
class ProcessLineMaker;

namespace GDBDebugger
{

class GDBBreakpointWidget;
class FramestackWidget;
class DisassembleWidget;
class Breakpoint;
class DbgController;
class DbgToolBar;
class VariableWidget;
class GDBOutputWidget;

class DebuggerPart : public KDevPlugin
{
    Q_OBJECT

public:
    DebuggerPart( QObject *parent, const char *name, const QStringList & );
    ~DebuggerPart();
    virtual void restorePartialProjectSession(const QDomElement* el);
    virtual void savePartialProjectSession(QDomElement* el);

private slots:
    void guiClientAdded(KXMLGUIClient*);
    void contextMenu(QPopupMenu *popup, const Context *context);
    void toggleBreakpoint();
    void contextWatch();
//    void projectOpened();
//    void projectClosed();
    void projectConfigWidget(KDialogBase *dlg);
    void slotActivePartChanged(KParts::Part*);

    void slotRun();
    void slotExamineCore();
    void slotAttachProcess();
    void slotStop(KDevPlugin* which = 0);
    void slotPause();
    void slotRunToCursor();
    void slotStepOver();
    void slotStepOverInstruction();
    void slotStepIntoInstruction();
    void slotStepInto();
    void slotStepOut();
    void slotMemoryView();

    void slotRefreshBPState(const Breakpoint&);
    void slotStatus(const QString &msg, int state);
    void slotShowStep(const QString &fileName, int lineNum);
    void slotGotoSource(const QString &fileName, int lineNum);

private:
    void startDebugger();
    void stopDebugger();
    void setupController();

    QGuardedPtr<VariableWidget> variableWidget;
    QGuardedPtr<GDBBreakpointWidget> gdbBreakpointWidget;
    QGuardedPtr<FramestackWidget> framestackWidget;
    QGuardedPtr<DisassembleWidget> disassembleWidget;
    QGuardedPtr<GDBOutputWidget> gdbOutputWidget;
    DbgController *controller;
    QLabel *statusBarIndicator;
    QGuardedPtr<DbgToolBar> floatingToolBar;
    ProcessLineMaker* procLineMaker;
    ProcessLineMaker* gdbLineMaker;

    QString m_contextIdent;
};

}

#endif
