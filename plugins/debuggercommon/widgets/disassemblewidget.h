/*
    SPDX-FileCopyrightText: 1999 John Birch <jbb@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2013 Vlas Puhov <vlas.puhov@mail.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _DISASSEMBLEWIDGET_H_
#define _DISASSEMBLEWIDGET_H_

#include "mi/mi.h"

#include <QActionGroup>
#include <QTreeWidget>
#include <QUrl>

#include <KConfigGroup>

#include "ui_selectaddressdialog.h"

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class QSplitter;

namespace KDevelop {
    class IDebugSession;
}

namespace KDevMI {

class RegistersManager;

class SelectAddressDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SelectAddressDialog(QWidget *parent = nullptr);

    QString address() const;
    void setAddress(const QString& address);
    bool hasValidAddress() const;
    void updateOkState();

private Q_SLOTS:
    void validateInput();
    void itemSelected();

private:
    Ui::SelectAddressDialog m_ui;
};

class DisassembleWidget;

enum DisassemblyFlavor {
    DisassemblyFlavorUnknown = -1,
    DisassemblyFlavorATT = 0,
    DisassemblyFlavorIntel,
};

class DisassembleWindow : public QTreeWidget
{
    Q_OBJECT

public:
    DisassembleWindow(QWidget *parent, DisassembleWidget* widget);

    void setDisassemblyFlavor(DisassemblyFlavor flavor);

protected:
   void contextMenuEvent(QContextMenuEvent *e) override;

private:
    QAction* m_selectAddrAction;
    QAction* m_jumpToLocation;
    QAction* m_runUntilCursor;
    QAction* m_disassemblyFlavorAtt;
    QAction* m_disassemblyFlavorIntel;
    QActionGroup* m_disassemblyFlavorActionGroup;
};

class MIDebuggerPlugin;


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

    explicit DisassembleWidget( MIDebuggerPlugin* plugin, QWidget *parent=nullptr );
    ~DisassembleWidget() override;

public Q_SLOTS:
    void slotActivate(bool activate);
    void slotShowStepInSource(const QUrl &fileName, int lineNum, const QString &address);
    void slotChangeAddress();
    ///Disassembles code at @p address and updates registers
    void update(const QString &address);
    void jumpToCursor();
    void runToCursor();
    void setDisassemblyFlavor(QAction* action);

protected:
    void showEvent(QShowEvent*) override;
    void hideEvent(QHideEvent*) override;
    void enableControls(bool enabled);

private:
    void currentSessionChanged(KDevelop::IDebugSession* iSession, KDevelop::IDebugSession* iPreviousSession);
    bool displayCurrent();
    void updateDisassemblyFlavor();

    /// Disassembles memory region from..to
    /// if from is empty current execution position is used
    /// if to is empty, 256 bytes range is taken
    void disassembleMemoryRegion(const QString& from=QString(),
        const QString& to=QString() );

    /// callbacks for GDBCommands
    void disassembleMemoryHandler(const MI::ResultRecord& r);
    void updateExecutionAddressHandler(const MI::ResultRecord& r);
    void setDisassemblyFlavorHandler(const MI::ResultRecord& r);
    void showDisassemblyFlavorHandler(const MI::ResultRecord& r);

    //for str to uint conversion.
    bool ok;
    bool    active_;
    unsigned long    lower_;
    unsigned long    upper_;
    unsigned long    address_;

    RegistersManager* m_registersManager ;

    DisassembleWindow * m_disassembleWindow;

    SelectAddressDialog* m_dlg;

    KConfigGroup m_config;
    QSplitter *m_splitter;
};

} // end of namespace KDevMI

#endif
