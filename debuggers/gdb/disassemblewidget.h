/*
 * GDB Debugger Support
 *
 * Copyright 1999 John Birch <jbb@kdevelop.org>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2013 Vlas Puhov <vlas.puhov@mail.ru>
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

#include <QUrl>
#include <KDialog>

#include <KConfigGroup>

#include "ui_selectaddress.h"

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class QSplitter;

namespace KDevelop {
    class IDebugSession;
}

namespace GDBDebugger
{

class RegistersManager;

class SelectAddrDialog: public KDialog
{
    Q_OBJECT
    
public:
    SelectAddrDialog(QWidget *parent = 0);
    
    QString getAddr() const
    { return hasValidAddress() ? m_ui.comboBox->currentText() : QString(); }

    void setAddress(const QString& address);
    bool hasValidAddress() const;
    void updateOkState();
     
private Q_SLOTS:
    void validateInput();
    void itemSelected();
    
private:
    Ui::SelectAddress m_ui;
};

class DisassembleWidget;

class DisassembleWindow : public QTreeWidget
{
public:
    DisassembleWindow(QWidget *parent, DisassembleWidget* widget);

protected:
   virtual void contextMenuEvent(QContextMenuEvent *e);

private:
    QAction* m_selectAddrAction;
    QAction* m_jumpToLocation;
    QAction* m_runUntilCursor;
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
    void slotShowStepInSource(const QUrl &fileName, int lineNum, const QString &address);
    void slotChangeAddress();
    ///Disassembles code at @p address and updates registers
    void update(const QString &address);

private Q_SLOTS:
    void currentSessionChanged(KDevelop::IDebugSession* session);
    void jumpToCursor();
    void runToCursor();

protected:
    virtual void showEvent(QShowEvent*);
    virtual void hideEvent(QHideEvent*);
    void enableControls(bool enabled);

private:
    bool displayCurrent();
    
    /// Disassembles memory region from..to
    /// if from is empty current execution position is used
    /// if to is empty, 256 bytes range is taken
    void disassembleMemoryRegion(const QString& from=QString(),
        const QString& to=QString() );

    /// callbacks for GDBCommands
    void disassembleMemoryHandler(const GDBMI::ResultRecord& r);
    void updateExecutionAddressHandler(const GDBMI::ResultRecord& r);

    //for str to uint conversion.
    bool ok;
    bool    active_;
    unsigned long    lower_;
    unsigned long    upper_;
    unsigned long    address_;

    RegistersManager* m_registersManager ;

    DisassembleWindow * m_disassembleWindow;
    
    static const QIcon icon_;
    SelectAddrDialog* m_dlg;

    KConfigGroup m_config;
    QSplitter *m_splitter;
};

}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

#endif
