/*
 * Displays registers.
 * Copyright 2013  Vlas Puhov <vlas.puhov@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#ifndef REGISTERSVIEW_H
#define REGISTERSVIEW_H

#include "ui_registersview.h"

#include <QVector>

class QMenu;
class QSignalMapper;

namespace GDBDebugger
{

class ModelsManager;

/** @brief Class for displaying registers content */
class RegistersView : public QWidget, private Ui::RegistersView
{
    Q_OBJECT

public:
    RegistersView(QWidget* p = 0);

    void enable(bool enabled);

    void setModel(ModelsManager* m);

public slots:
    ///Updates registers for active views.
    void updateRegisters();

protected:
    ///Allows to choose register formates/modes.
    virtual void contextMenuEvent(QContextMenuEvent* e) override;

private:
    ///Returns list of active views.
    QStringList activeViews();

    ///Adds @p v to the list of views with assigning it a name.
    void addView(QTableView* view, int idx);

    ///Clears names of all views.
    void clear();

    ///Sets name for the table with index @p idx to the @p name.
    void setNameForTable(int idx, const QString& name);

private slots:
    ///Changes register formates/modes to @p formatOrMode.
    void menuTriggered(const QString& formatOrMode);

private:
    void setupActions();
    ///Adds new action into m_actions and to this widget's list of actions.
    void insertAction(const QString& name, Qt::Key k);
    ///Returns action for given @p name.
    QAction* findAction(const QString& name);
private Q_SLOTS:
    ///Enables/disables actions based on current view.
    void changeAvaliableActions();

private:
    QMenu* m_menu;
    QSignalMapper* m_mapper;

    ModelsManager* m_modelsManager;

    QVector<QAction*> m_actions;
};

}
#endif
