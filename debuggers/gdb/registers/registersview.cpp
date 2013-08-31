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

#include "registersview.h"

#include <QMenu>
#include <QContextMenuEvent>
#include <QSignalMapper>

#include <KDebug>

namespace GDBDebugger
{

RegistersView::RegistersView(QWidget* p)
    : QWidget(p), m_modelsManager(0), m_tablesManager(this), m_registersFormat(Raw)
{
    setupUi(this);

    m_menu = new QMenu(this);
    m_mapper = new QSignalMapper(this);

    connect(m_mapper, SIGNAL(mapped(int)), this, SLOT(formatMenuTriggered(int)));
}

void RegistersView::contextMenuEvent(QContextMenuEvent* e)
{
    m_menu->clear();

    QAction* a = m_menu->addAction("Update");
    connect(a, SIGNAL(triggered()), this, SLOT(updateMenuTriggered()));

    //Format changing makes sense only for general registers (maybe for segment too),
    if (tabWidget->currentIndex() == 0) {
        QMenu* m = m_menu->addMenu("Format");
        addItemToFormatSubmenu(m, QString("Dec"), Decimal);
        addItemToFormatSubmenu(m, QString("Hex"), Hexadecimal);
        addItemToFormatSubmenu(m, QString("Raw"), Raw);
        addItemToFormatSubmenu(m, QString("Oct"), Octal);
        addItemToFormatSubmenu(m, QString("Bin"), Binary);
    }

    m_menu->exec(e->globalPos());
}

void RegistersView::addItemToFormatSubmenu(QMenu* m, const QString& name, RegistersFormat format)
{
    QAction* a = m->addAction(name);
    a->setData(format);
    if (format == m_registersFormat) {
        a->setCheckable(true);
        a->setChecked(true);
    }
    m_mapper->setMapping(a, a->data().toInt());
    connect(a, SIGNAL(triggered()), m_mapper, SLOT(map()));
}

void RegistersView::updateMenuTriggered(void)
{
    emit needToUpdateRegisters();
}

void RegistersView::formatMenuTriggered(int format)
{
    m_registersFormat = static_cast<RegistersFormat>(format);
    m_modelsManager->updateRegisters();
}

RegistersView::Table RegistersView::TablesManager::tableForGroup(const QString& group) const
{
    Table t;

    if (group.isEmpty()) {
        return t;
    }

    foreach (const TableRegistersAssociation & a, m_tableRegistersAssociation) {
        if (a.registersGroup == group) {
            t = a.table;
            break;
        }
    }

    return t;
}

bool RegistersView::TablesManager::removeAssociation(const QString& group)
{
    if (group.isEmpty()) {
        return false;
    }

    for (int i = 0; i < m_tableRegistersAssociation.count(); i++) {
        if (m_tableRegistersAssociation[i].registersGroup == group) {
            m_tableRegistersAssociation[i].registersGroup.clear();
            int idx = m_tableRegistersAssociation[i].table.index;
            m_parent->tabWidget->setTabText(idx, "");
            return true;
        }
    }
    return false;
}

void RegistersView::TablesManager::addTable(const RegistersView::Table& table)
{
    Table _table = table;

    _table.tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    _table.tableWidget->horizontalHeader()->hide();
    _table.tableWidget->verticalHeader()->hide();
    _table.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    _table.tableWidget->setMinimumWidth(10);
    _table.tableWidget->verticalHeader()->setDefaultSectionSize(15);

    QString name = m_parent->m_modelsManager->addView(table.tableWidget);

    m_tableRegistersAssociation.push_back(TableRegistersAssociation(_table, name));
    changeName(_table, name);
}

void RegistersView::enable(bool enabled)
{
    setEnabled(enabled);
    if (enabled) {
        m_tablesManager.clearAllAssociations();

        m_tablesManager.addTable(Table(registers, 0));
        m_tablesManager.addTable(Table(flags, 0));
        m_tablesManager.addTable(Table(table_1, 1));
        m_tablesManager.addTable(Table(table_2, 2));
        m_tablesManager.addTable(Table(table_3, 3));
        m_tablesManager.load();
    }
}

void RegistersView::TablesManager::save()
{
    m_config.writeEntry("format", static_cast<int>(m_parent->m_registersFormat));
}

void RegistersView::TablesManager::load()
{
    m_parent->m_registersFormat = static_cast<RegistersFormat>(m_config.readEntry("format", static_cast<int>(m_parent->m_registersFormat)));
}

RegistersView::TablesManager::TablesManager(RegistersView* parent) : m_parent(parent)
{
    m_config = KGlobal::config()->group("Tables manager");
}

void RegistersView::TablesManager::clearAllAssociations()
{
    foreach (const TableRegistersAssociation & a, m_tableRegistersAssociation) {
        removeAssociation(a.registersGroup);
    }
}

RegistersView::Table::Table()
    : tableWidget(0), index(-1) {}

RegistersView::Table::Table(QTableView* tableWidget, int idx)
    : tableWidget(tableWidget), index(idx) {}

bool RegistersView::Table::isNull() const
{
    return !tableWidget;
}

RegistersView::TableRegistersAssociation::TableRegistersAssociation() {}

RegistersView::TableRegistersAssociation::TableRegistersAssociation(const RegistersView::Table& table, const QString& registersGroup)
    : table(table), registersGroup(registersGroup) {}

RegistersView::TablesManager::~TablesManager()
{
    save();
}

void RegistersView::TablesManager::setNameForTable(RegistersView::TableRegistersAssociation& t)
{
    Q_ASSERT(t.table.index != -1);
    int idx = t.table.index;

    kDebug() << t.registersGroup << " " << t.table.index;
    const QString text = m_parent->tabWidget->tabText(idx);
    if (!text.contains(t.registersGroup)) {
        const QString name = t.registersGroup;
        m_parent->tabWidget->setTabText(idx, text.isEmpty() ? name : text + '/' + name);
    }
}

void RegistersView::nameForViewChanged(const QString& oldName, const QString& newName)
{
    Table t = m_tablesManager.tableForGroup(oldName);
    if (!t.isNull()) {
        m_tablesManager.changeName(t, newName);
    }
}

void RegistersView::TablesManager::changeName(const RegistersView::Table& table, const QString& name)
{
    for (int i = 0; i < m_tableRegistersAssociation.count(); i++) {
        if (m_tableRegistersAssociation[i].table.tableWidget == table.tableWidget) {
            m_tableRegistersAssociation[i].registersGroup = name;
            setNameForTable(m_tableRegistersAssociation[i]);
        }
    }
}

bool RegistersView::TablesManager::isEmpty()
{
    return m_tableRegistersAssociation.isEmpty();
}

void RegistersView::setModel(ModelsManager* m)
{
    m_modelsManager = m;
}

}
