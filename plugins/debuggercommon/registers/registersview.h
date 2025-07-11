/*
    SPDX-FileCopyrightText: 2013 Vlas Puhov <vlas.puhov@mail.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef REGISTERSVIEW_H
#define REGISTERSVIEW_H

#include "ui_registersview.h"

#include <QVector>

class QMenu;

namespace KDevMI {

class ModelsManager;

/** @brief Class for displaying registers content */
class RegistersView : public QWidget, private Ui::RegistersView
{
    Q_OBJECT

public:
    explicit RegistersView(QWidget* p = nullptr);

    void enable(bool enabled);

    void setModel(ModelsManager* m);

public Q_SLOTS:
    ///Updates registers for active views.
    void updateRegisters();

protected:
    ///Allows to choose register formats/modes.
    void contextMenuEvent(QContextMenuEvent* e) override;

private:
    ///Returns list of active views.
    [[nodiscard]] QStringList activeViews() const;

    ///Adds @p v to the list of views with assigning it a name.
    void addView(QTableView* view, int idx);

    ///Clears names of all views.
    void clear();

    ///Sets name for the table with index @p idx to the @p name.
    void setNameForTable(int idx, const QString& name);

private Q_SLOTS:
    ///Changes register formats/modes to @p formatOrMode.
    void menuTriggered(const QString& formatOrMode);

private:
    void setupActions();
    ///Adds new action into m_actions and to this widget's list of actions.
    void insertAction(const QString& name, Qt::Key k);
    ///Returns action for given @p name.
    QAction* findAction(const QString& name) const;
private Q_SLOTS:
    ///Enables/disables actions based on current view.
    void changeAvaliableActions();

private:
    QMenu* m_menu;

    ModelsManager* m_modelsManager = nullptr;

    QVector<QAction*> m_actions;
};

} // end of namespace KDevMI

#endif
