/*
 * GDB Debugger Support
 *
 * Copyright 1999 John Birch <jbb@kdevelop.org>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
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

#ifndef _DISASSEMBLEWIDGET_H_
#define _DISASSEMBLEWIDGET_H_

#include "mi/gdbmi.h"

#include <QTreeWidget>

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

namespace GDBDebugger
{

class Breakpoint;
class GDBController;
class CppDebuggerPlugin;

class DisassembleWidget : public QTreeWidget
{
    Q_OBJECT

public:
    enum Columns {
        Address,
        Function,
        Offset,
        Instruction,
        ColumnCount
    };

    DisassembleWidget( CppDebuggerPlugin* plugin, GDBController* controller, QWidget *parent=0 );
    virtual ~DisassembleWidget();

Q_SIGNALS:
    void requestRaise();

public Q_SLOTS:
    void slotActivate(bool activate);
    void slotDeactivate();
    void slotShowStepInSource(const QString &fileName, int lineNum, const QString &address);

protected:
    virtual void showEvent(QShowEvent*);
    virtual void hideEvent(QHideEvent*);

private:
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
