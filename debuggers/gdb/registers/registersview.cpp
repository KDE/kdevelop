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
    : QWidget(p), m_registerController(0), m_tablesManager(this), m_registersFormat(Raw)
{
    setupUi(this);

    m_menu = new QMenu(this);
    m_mapper = new QSignalMapper(this);

    connect(m_mapper, SIGNAL(mapped(int)), this, SLOT(formatMenuTriggered(int)));

    m_tablesManager.addTable(Table(registers, 0));
    m_tablesManager.addTable(Table(flags, 0));
    m_tablesManager.addTable(Table(table_1, 1));
    m_tablesManager.addTable(Table(table_2, 2));
    m_tablesManager.addTable(Table(table_3, 3));
    m_tablesManager.addTable(Table(table_4, 4));
    m_tablesManager.load();
}

void RegistersView::updateRegistersInTable(const Table& table, const RegistersGroup& registersGroup)
{
    Table _table = table;

    _table.tableWidget->blockSignals(true);

    _table.tableWidget->setRowCount(registersGroup.registers.size());

    for (int i = 0 ; i < registersGroup.registers.size(); ++i) {
        QTableWidgetItem* newItem = new QTableWidgetItem(registersGroup.registers[i].name);
        newItem->setFlags(Qt::ItemIsEnabled);
        _table.tableWidget->setItem(i, RegisterName, newItem);

        newItem = new QTableWidgetItem(registersGroup.registers[i].value);
        if (registersGroup.flag || !registersGroup.editable) {
            newItem->setFlags(Qt::ItemIsEnabled);
        }
        _table.tableWidget->setItem(i, RegisterValue, newItem);
    }

    int width = _table.tableWidget->columnWidth(RegisterName) + _table.tableWidget->columnWidth(RegisterValue) + 10;
    _table.tableWidget->setMinimumWidth(width);

    _table.tableWidget->blockSignals(false);
}

void RegistersView::registerChangedInternally(QTableWidgetItem* item)
{
    if (item->column() != RegisterValue) {
        return;
    }

    const QTableWidget* table = static_cast<const QTableWidget*>(sender());

    const Register changedRegister(table->item(item->row(), RegisterName)->text(), item->text());

    kDebug() << changedRegister.name << ' ' << changedRegister.value;
    emit registerChanged(changedRegister);
}

void RegistersView::contextMenuEvent(QContextMenuEvent* e)
{
    Q_ASSERT(m_registerController);

    QStringList groups = m_registerController->namesOfRegisterGroups();

    m_menu->clear();

    QAction* a = m_menu->addAction("Update");
    connect(a, SIGNAL(triggered()), this, SLOT(updateMenuTriggered()));

    QMenu* m = m_menu->addMenu("Format");
    addItemToFormatSubmenu(m, QString("Dec"), Decimal);
    addItemToFormatSubmenu(m, QString("Hex"), Hexadecimal);
    addItemToFormatSubmenu(m, QString("Raw"), Raw);
    addItemToFormatSubmenu(m, QString("Oct"), Octal);
    addItemToFormatSubmenu(m, QString("Bin"), Binary);

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
    m_registerController->updateRegisters();
}

void RegistersView::formatMenuTriggered(int format)
{
    m_registersFormat = static_cast<RegistersFormat>(format);
    m_registerController->updateRegisters();
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

RegistersView::Table RegistersView::TablesManager::createTableForGroup(const QString& group)
{
    Table t;
    if (group.isEmpty()) {
        return t;
    }

    for (int i = 0; i < m_tableRegistersAssociation.count(); i++) {
        if (m_tableRegistersAssociation[i].registersGroup.isEmpty()) {
            m_tableRegistersAssociation[i].registersGroup = group;
            t = m_tableRegistersAssociation[i].table;
            setNameForTable(m_tableRegistersAssociation[i]);
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
            return true;
        }
    }
    return false;
}

void RegistersView::TablesManager::addTable(const RegistersView::Table& table)
{
    Table _table = table;
    m_tableRegistersAssociation.push_back(TableRegistersAssociation(_table, ""));

    _table.tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    _table.tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    _table.tableWidget->setMinimumWidth(10);

    connect(_table.tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), m_parent, SLOT(registerChangedInternally(QTableWidgetItem*)));
    connect(_table.tableWidget, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), m_parent, SLOT(flagChangedInternally(QTableWidgetItem*)));
}

int RegistersView::TablesManager::numOfFreeTables() const
{
    int count = 0;
    foreach (const TableRegistersAssociation & a, m_tableRegistersAssociation) {
        if (a.registersGroup.isEmpty()) {
            count++;
        }
    }
    return count;
}

void RegistersView::registersInGroupChanged(const QString& group)
{
    Q_ASSERT(m_registerController);

    kDebug() << "Updating registers";

    const RegistersGroup registersGroup = m_registerController->registersFromGroup(group, m_registersFormat);

    const Table t = m_tablesManager.tableForGroup(registersGroup.groupName);
    if (!t.isNull()) {
        updateRegistersInTable(t, registersGroup);
    }
}

void RegistersView::setController(IRegisterController* controller)
{
    if (controller && controller != m_registerController) {

        connect(controller, SIGNAL(registersInGroupChanged(QString)), this, SLOT(registersInGroupChanged(QString)));

        connect(this, SIGNAL(registerChanged(const Register&)), controller, SLOT(setRegisterValue(const Register&)));

        m_tablesManager.clearAllAssociations();
        const QStringList groups = controller->namesOfRegisterGroups();
        foreach (const QString & g, groups) {
                m_tablesManager.createTableForGroup(g);
        }
    }
    setEnabled(controller ? true : false);
    m_registerController = controller;
}

const QStringList RegistersView::TablesManager::allGroups() const
{
    QStringList groups;
    foreach (const TableRegistersAssociation & a, m_tableRegistersAssociation) {
        if (!a.registersGroup.isEmpty()) {
            groups << a.registersGroup;
        }
    }
    return groups;
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

void RegistersView::flagChangedInternally(QTableWidgetItem* item)
{
    if (item->column() != RegisterValue || item->text().length() != 1 || item->flags() & Qt::ItemIsEditable) {
        return;
    }

    const QTableWidget* table = static_cast<const QTableWidget*>(sender());

    const Register changedRegister(table->item(item->row(), RegisterName)->text(), item->text());

    kDebug() << changedRegister.name << ' ' << changedRegister.value;
    emit registerChanged(changedRegister);
}

RegistersView::Table::Table()
    : tableWidget(0), index(-1) {}

RegistersView::Table::Table(QTableWidget* tableWidget, int idx)
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

}
