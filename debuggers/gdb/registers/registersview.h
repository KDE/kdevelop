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

#include "../ui_registersview.h"

#include "modelsmanager.h"

#include <KConfigGroup>

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

signals:
    void needToUpdateRegisters();

protected:
    ///Allows to choose tables/register formates.
    virtual void contextMenuEvent(QContextMenuEvent* e);

private:
    QString currentView();

    ///Convenient representation of a table.
    struct Table {
        Table();
        Table(QTableView* tableWidget, int idx);
        QTableView* tableWidget;
        int index;///unique index on the screen
        QString name;
    };

    /** @brief Tables view manager.*/
    class TablesManager
    {
    public:
        TablesManager(RegistersView*);
        ~TablesManager();

        void save();
        void load();

        ///Adds @p table to the list of tables with assigning it a name.
        void addTable(const Table& table);

        void clear();

    private:
        ///Sets name for the table @p t in the view.
        void setNameForTable(Table& t, const QString& name);

        RegistersView* m_parent;
        QVector<Table> m_tables;
        KConfigGroup m_config;
    };

private slots:
    ///Changes register formates to @p format.
    void formatMenuTriggered(const QString& format);
    ///Updates visible tables
    void updateMenuTriggered(void);

private:
    void addItemToFormatSubmenu(QMenu* m, const QString& format);

private:
    QMenu* m_menu;
    QSignalMapper* m_mapper;

    ModelsManager* m_modelsManager;
    TablesManager m_tablesManager;

    friend class TablesManager;
};

}
#endif
