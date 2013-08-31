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

public Q_SLOTS:
    void nameForViewChanged(const QString& oldName, const QString& newName);

private:
    ///Convenient representation of a table.
    struct Table {
        Table();
        Table(QTableView* tableWidget, int idx);
        bool isNull() const;
        QTableView* tableWidget;
        int index;
    };

    ///Association between @p table and @p registers group.
    struct TableRegistersAssociation {
        TableRegistersAssociation();
        TableRegistersAssociation(const Table& table, const QString& registersGroup);
        Table table;
        QString registersGroup;
    };

    /** @brief Manages associations between tables and register groups.*/
    class TablesManager
    {
    public:
        TablesManager(RegistersView* parent);
        ~TablesManager();

        void save();
        void load();
        ///Returns the table associated with the @p group, empty table if there is no association.
        Table tableForGroup(const QString& group) const;

        ///Removes association between @p group and table if any.
        bool removeAssociation(const QString& group);

        ///Adds @p table to the list of available tables
        void addTable(const Table& table);

        void clearAllAssociations();

        ///Sets new @p name for the @p table.
        void changeName(const Table& table, const QString& name);

        bool isEmpty();

    private:
        ///Sets name for the table @p t in the view.
        void setNameForTable(TableRegistersAssociation& t);

        RegistersView* m_parent;
        QVector<TableRegistersAssociation> m_tableRegistersAssociation;
        KConfigGroup m_config;
    };

private slots:
    ///Changes register formates to @p format.
    void formatMenuTriggered(int format);
    ///Updates visible tables
    void updateMenuTriggered(void);

private:
    void addItemToFormatSubmenu(QMenu* m, const QString& name, RegistersFormat format);
private:
    QMenu* m_menu;
    QSignalMapper* m_mapper;

    ///FIXME: not member at all
    ModelsManager* m_modelsManager;
    TablesManager m_tablesManager;
    RegistersFormat m_registersFormat;

    friend class TablesManager;
};

}
#endif
