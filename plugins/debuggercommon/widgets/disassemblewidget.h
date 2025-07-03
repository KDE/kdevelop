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

    /**
     * Clear the current address and the history of previously selected addresses.
     */
    void clearHistory();

private:
    bool hasValidAddress() const;
    void updateOkState();

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
    using HandlerMethod = void (DisassembleWidget::*)(const MI::ResultRecord&);

    void currentSessionChanged(KDevelop::IDebugSession* iSession, KDevelop::IDebugSession* iPreviousSession);

    /**
     * @return whether any memory region is currently displayed
     */
    [[nodiscard]] bool isMemoryRegionDisplayed() const;
    bool displayCurrent();
    void updateDisassemblyFlavor();

    /**
     * Disassemble the memory region [@p from, @p to] including the bounding addresses.
     *
     * @param from a valid memory address
     * @param to a valid memory address greater than @p from, or an empty string (the default) to signify @p from + 255
     * @param handlerMethod a member function that shall handle the result of the disassemble MI command
     */
    void disassembleMemoryRegion(QStringView from, QStringView to = {},
                                 HandlerMethod handlerMethod = &DisassembleWidget::disassembleMemoryHandler);

    /// callbacks for GDBCommands
    void disassembleMemoryHandler(const MI::ResultRecord& r);
    void setDisassemblyFlavorHandler(const MI::ResultRecord& r);
    void refreshRegionDisassemblyFlavorHandler(const MI::ResultRecord& r);
    void showDisassemblyFlavorHandler(const MI::ResultRecord& r);

    using AddressInteger = unsigned long;

    /**
     * This class stores a memory address in two formats - string and integer.
     */
    class StoredAddress
    {
    public:
        /**
         * Create an invalid (empty/0) address.
         */
        StoredAddress() = default;

        /**
         * Assign a given valid address to this object.
         */
        StoredAddress& operator=(const QString& address);

        /**
         * Reset this object to an invalid (empty/0) address.
         */
        void reset();

        /**
         * @return whether this object currently stores a valid (nonempty) address
         */
        [[nodiscard]] bool isValid() const;

        [[nodiscard]] const QString& string() const;
        [[nodiscard]] AddressInteger integer() const;

        [[nodiscard]] bool operator==(const StoredAddress& other) const;
        [[nodiscard]] bool operator!=(const StoredAddress& other) const;
        [[nodiscard]] bool operator<(const StoredAddress& other) const;
        [[nodiscard]] bool operator>(const StoredAddress& other) const;

    private:
        QString m_string;
        AddressInteger m_integer = 0;
    };

    bool    active_;
    /**
     * Whether the currently displayed memory region is up to date.
     * If @c false, the current address will be displayed in a disassembled memory region
     * and the registers will be updated as soon as this widget becomes active.
     */
    bool m_upToDate = true;
    /**
     * Whether the disassembly flavor of the currently displayed memory region is up to date.
     */
    bool m_regionDisassemblyFlavorUpToDate = true;
    StoredAddress m_currentAddress; ///< the address, at which the debugger last stopped/paused, or invalid address
    StoredAddress m_regionFirst; ///< the first address of the currently displayed memory region or invalid address
    StoredAddress m_regionLast; ///< the last address of the currently displayed memory region or invalid address

    RegistersManager* m_registersManager ;

    DisassembleWindow * m_disassembleWindow;

    SelectAddressDialog* m_dlg;

    KConfigGroup m_config;
    QSplitter *m_splitter;
};

} // end of namespace KDevMI

#endif
