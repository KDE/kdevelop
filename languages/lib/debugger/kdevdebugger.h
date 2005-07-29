/* This file is part of the KDE project
   Copyright (C) 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef _KDEVDEBUGGER_H_
#define _KDEVDEBUGGER_H_


#include <qobject.h>
#include <qpixmap.h>


#include <kurl.h>

/**
* Base class to handle signals from the editor that relate to breakpoints 
* and the execution point of the debugger.
*/
class KDevDebugger : public QObject
{
  Q_OBJECT
    
public:
    
  KDevDebugger(QObject *parent=0, const char *name=0);
  ~KDevDebugger();

  /**
   * Sets a breakpoint in the editor document belong to fileName.
   * If id==-1, the breakpoint is deleted.
   */
  virtual void setBreakpoint(const QString &fileName, int lineNum,
                             int id, bool enabled, bool pending) = 0;
   
  /**
   * Goes to a given location in a source file and marks the line.
   * This is used by the debugger to mark the location where the
   * the debugger has stopped.
   */
  virtual void gotoExecutionPoint(const KURL &url, int lineNum=0) = 0;

  /**
   * Clear the execution point. Usefull if debugging has ended.
   */
  virtual void clearExecutionPoint() = 0;

  static const QPixmap* inactiveBreakpointPixmap();
  static const QPixmap* activeBreakpointPixmap();
  static const QPixmap* reachedBreakpointPixmap();
  static const QPixmap* disabledBreakpointPixmap();
  static const QPixmap* executionPointPixmap();
  
signals:

  /**
   * The user has toggled a breakpoint.
   */
  void toggledBreakpoint(const QString &fileName, int lineNum);

  /*
   * The user wants to edit the properties of a breakpoint.
   */
  void editedBreakpoint(const QString &fileName, int lineNum);
  
  /**
   * The user wants to enable/disable a breakpoint.
   */
  void toggledBreakpointEnabled(const QString &fileName, int lineNum);
    
};


#endif
