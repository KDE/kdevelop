/***************************************************************************
    begin                : Sun Aug 8 1999
    copyright            : (C) 1999 by John Birch
    email                : jbb@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DISASSEMBLEWIDGET_H_
#define _DISASSEMBLEWIDGET_H_

#include "mi/gdbmi.h"

#include <q3textedit.h>
//Added by qt3to4:
#include <QShowEvent>
#include <QHideEvent>
#include <kvbox.h>

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

namespace GDBDebugger
{

class Breakpoint;
class GDBController;
class CppDebuggerPlugin;

class DisassembleWidget : public Q3TextEdit
{
    Q_OBJECT

public:
    DisassembleWidget( CppDebuggerPlugin* plugin, GDBController* controller, QWidget *parent=0 );
    virtual ~DisassembleWidget();

public Q_SLOTS:
    void slotActivate(bool activate);
    void slotDeactivate();
    void slotShowStepInSource(const QString &fileName, int lineNum, const QString &address);

private:
    virtual void showEvent(QShowEvent*);
    virtual void hideEvent(QHideEvent*);

    bool displayCurrent();
    void getNextDisplay();

    /// callback for GDBCommand
    void memoryRead(const GDBMI::ResultRecord& r);

    GDBController* controller_;
    bool    active_;
    unsigned long    lower_;
    unsigned long    upper_;
    unsigned long    address_;
    QString currentAddress_;
};

}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

#endif
