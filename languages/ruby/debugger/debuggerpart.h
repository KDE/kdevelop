/***************************************************************************
 *   Copyright (C) 1999-2001 by John Birch                                 *
 *   jbb@kdevelop.org                                                      *
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *	                                                                       *
 *                         Adapted for ruby debugging                      *
 *                         --------------------------                      *
 *  begin                : Mon Nov 1 2004                                  *
 *  copyright            : (C) 2004 by Richard Dale                        *
 *  email                : Richard_Dale@tipitina.demon.co.uk               *
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

namespace KParts { class Part; }

class QLabel;
class QPopupMenu;
class KDialogBase;
class ProcessWidget;
class ProcessLineMaker;
class KDevAppFrontend;
class KDevDebugger;

namespace RDBDebugger
{

class RDBBreakpointWidget;
class FramestackWidget;
class Breakpoint;
class DbgController;
class DbgToolBar;
class VariableWidget;
class RDBOutputWidget;

class RubyDebuggerPart : public KDevPlugin
{
    Q_OBJECT

public:
    RubyDebuggerPart( QObject *parent, const char *name, const QStringList & );
    ~RubyDebuggerPart();
    virtual void restorePartialProjectSession(const QDomElement* el);
    virtual void savePartialProjectSession(QDomElement* el);

private slots:
    void guiClientAdded(KXMLGUIClient*);
    void contextMenu(QPopupMenu *popup, const Context *context);
    void toggleBreakpoint();
    void contextWatch();
    void contextRubyInspect();
//    void projectOpened();
    void projectClosed();
    void slotActivePartChanged(KParts::Part*);

    void slotRun();
    void slotStopDebugger();
    void slotStop(KDevPlugin* which = 0);
    void slotPause();
    void slotRunToCursor();
    void slotStepOver();
    void slotStepInto();
    void slotStepOut();

    void slotRefreshBPState(const Breakpoint&);
    void slotStatus(const QString &msg, int state);
    void slotShowStep(const QString &fileName, int lineNum);
    void slotGotoSource(const QString &fileName, int lineNum);

signals:
    void rubyInspect(const QString&);

private:
    KDevAppFrontend *appFrontend();
    KDevDebugger *debugger();
    
	bool startDebugger();
    void setupController();

    QGuardedPtr<VariableWidget> variableWidget;
    QGuardedPtr<RDBBreakpointWidget> rdbBreakpointWidget;
    QGuardedPtr<FramestackWidget> framestackWidget;
    QGuardedPtr<RDBOutputWidget> rdbOutputWidget;
    DbgController *controller;
    QGuardedPtr<QLabel> statusBarIndicator;
    QGuardedPtr<DbgToolBar> floatingToolBar;
    ProcessLineMaker* procLineMaker;
    ProcessLineMaker* rdbLineMaker;

    QString m_contextIdent;
    QCString m_drkonqi;
	
    KDevDebugger *m_debugger;
};

}

#endif
