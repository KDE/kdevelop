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

#include <KUrl>
#include <KIcon>
#include <KDialog>

#include "ui_selectaddress.h"

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

namespace KDevelop {
    class IDebugSession;
}

namespace GDBDebugger
{

class SelectAddrDialog: public KDialog
{
    Q_OBJECT
    
public:
    SelectAddrDialog(QWidget *parent = 0);
    
    QString getAddr() const
    { return hasValidAddress() ? m_ui.comboBox->currentText() : QString(); }
    
    bool hasValidAddress() const;
    void updateOkState();
     
private Q_SLOTS:
    void validateInput();
    void itemSelected();
    
private:
    Ui::SelectAddress m_ui;
};

class DisassembleWindow : public QTreeWidget
{
public:
    DisassembleWindow(QWidget *parent = 0);

protected:
   virtual void contextMenuEvent(QContextMenuEvent *e);

private:
    QAction* m_selectAddrAction;
    QAction* m_jumpToLocation;
};


class Breakpoint;
class DebugSession;
class CppDebuggerPlugin;


class DisassembleWidget : public QWidget
{
    Q_OBJECT

public:
    enum Columns {
        Icon,
        Address,
        Function,
        Offset,
        Instruction,
        ColumnCount
    };

    DisassembleWidget( CppDebuggerPlugin* plugin, QWidget *parent=0 );
    virtual ~DisassembleWidget();

Q_SIGNALS:
    void requestRaise();

public Q_SLOTS:
    void slotActivate(bool activate);
    void slotDeactivate();
    void slotShowStepInSource(const KUrl &fileName, int lineNum, const QString &address);
    void slotValidateEdits();
    void slotChangeAddress();
    void slotShowAddrRange();

private Q_SLOTS:
    void currentSessionChanged(KDevelop::IDebugSession* session);
    void jumpToCursor();

protected:
    virtual void showEvent(QShowEvent*);
    virtual void hideEvent(QHideEvent*);
    bool hasValidAddrRange();
    void enableControls(bool enabled);


private:
    bool displayCurrent();
    
    // Disassemble memory region addr1..addr2
    // if addr2 is empty, 128 bytes range taken
    // if addr1 is empty, $pc is used
    void getAsmToDisplay(const QString& addr1=QString(),
        const QString& addr2=QString() );

    /// callback for GDBCommand
    void memoryRead(const GDBMI::ResultRecord& r);

    bool    active_;
    unsigned long    lower_;
    unsigned long    upper_;
    unsigned long    address_;
    QString m_currentAddress;
    
    DisassembleWindow * m_disassembleWindow;
    QComboBox* m_startAddress;
    QComboBox* m_endAddress;
    QPushButton* m_evalButton;
    
    static const KIcon icon_;
    SelectAddrDialog* m_dlg;
};

}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

#endif
