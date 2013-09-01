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
    : QWidget(p), m_modelsManager(0), m_tablesManager(this)
{
    setupUi(this);

    m_menu = new QMenu(this);
    m_mapper = new QSignalMapper(this);

    connect(m_mapper, SIGNAL(mapped(QString)), this, SLOT(formatMenuTriggered(QString)));

    connect(this, SIGNAL(needToUpdateRegisters()), m_modelsManager, SLOT(updateRegisters()));
}

void RegistersView::contextMenuEvent(QContextMenuEvent* e)
{
    m_menu->clear();

    QAction* a = m_menu->addAction("Update");
    connect(a, SIGNAL(triggered()), this, SLOT(updateMenuTriggered()));

    QString group = currentView();

    const QStringList formats = m_modelsManager->formats(group);
    if (formats.size() > 1) {
        QMenu* m = m_menu->addMenu("Format");
        foreach (const QString& fmt, formats) {
            addItemToFormatSubmenu(m, fmt);
        }
    }

    m_menu->exec(e->globalPos());
}

void RegistersView::addItemToFormatSubmenu(QMenu* m, const QString& format)
{
    QAction* a = m->addAction(format);
    a->setCheckable(true);

    const QString view = currentView();

    if (format == m_modelsManager->formats(view).first()) {
        a->setChecked(true);
    }

    m_mapper->setMapping(a, a->text());
    connect(a, SIGNAL(triggered()), m_mapper, SLOT(map()));
}

void RegistersView::updateMenuTriggered(void)
{
    emit needToUpdateRegisters();
}

void RegistersView::formatMenuTriggered(const QString& format)
{
    m_modelsManager->setFormat(currentView(), format);
    m_modelsManager->updateRegisters(currentView());

    m_tablesManager.save();
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

    m_tables.push_back(_table);
    setNameForTable(_table, name);
}

void RegistersView::enable(bool enabled)
{
    setEnabled(enabled);
    if (enabled) {

        m_tablesManager.clear();

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
    foreach (const Table & t, m_tables) {
        m_config.writeEntry(t.name, m_parent->m_modelsManager->formats(t.name).first());
    }
}

void RegistersView::TablesManager::load()
{
    foreach (const Table & t, m_tables) {
        QString format = m_config.readEntry(t.name, m_parent->m_modelsManager->formats(t.name).first());

        m_parent->m_modelsManager->setFormat(t.name, format);
    }
}

RegistersView::TablesManager::TablesManager(RegistersView* v): m_parent(v)
{
    m_config = KGlobal::config()->group("Tables manager");
}

RegistersView::Table::Table()
    : tableWidget(0), index(-1) {}

RegistersView::Table::Table(QTableView* tableWidget, int idx)
    : tableWidget(tableWidget), index(idx) {}

RegistersView::TablesManager::~TablesManager(){}

void RegistersView::TablesManager::setNameForTable(Table& t, const QString& name)
{
    Q_ASSERT(t.index != -1);
    int idx = t.index;

    for (int i = 0; i < m_tables.count(); i++) {
        if (m_tables[i].tableWidget == t.tableWidget) {
            m_tables[i].name = name;
            break;
        }
    }

    kDebug() << name << " " << t.index;
    const QString text = m_parent->tabWidget->tabText(idx);
    if (!text.contains(name)) {
        m_parent->tabWidget->setTabText(idx, text.isEmpty() ? name : text + '/' + name);
    }
}

void RegistersView::setModel(ModelsManager* m)
{
    m_modelsManager = m;
}

QString RegistersView::currentView()
{
    return tabWidget->tabText(tabWidget->currentIndex()).split('/').first();
}

void RegistersView::TablesManager::clear()
{
    m_tables.clear();
}

}
