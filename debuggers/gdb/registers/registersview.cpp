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
namespace
{
const int TABLES_COUNT = 5;
}

RegistersView::RegistersView(QWidget* p)
    : QWidget(p), m_modelsManager(0)
{
    setupUi(this);

    m_menu = new QMenu(this);
    m_mapper = new QSignalMapper(this);

    connect(m_mapper, SIGNAL(mapped(QString)), this, SLOT(formatMenuTriggered(QString)));

    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(updateMenuTriggered(int)));
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
        foreach (const QString & fmt, formats) {
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

void RegistersView::updateMenuTriggered(int /*idx*/)
{
    if (!currentView().isEmpty()) {
        m_modelsManager->updateRegisters(currentView());
    }
}

void RegistersView::formatMenuTriggered(const QString& format)
{
    m_modelsManager->setFormat(currentView(), format);
    m_modelsManager->updateRegisters(currentView());
}

void RegistersView::addView(QTableView* view, int idx)
{
    view->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    view->horizontalHeader()->hide();
    view->verticalHeader()->hide();
    view->setSelectionMode(QAbstractItemView::SingleSelection);
    view->setMinimumWidth(10);
    view->verticalHeader()->setDefaultSectionSize(15);

    QString name = m_modelsManager->addView(view);

    setNameForTable(idx, name);
}

void RegistersView::enable(bool enabled)
{
    setEnabled(enabled);
    if (enabled) {

        clear();

        addView(registers, 0);
        addView(flags, 0);
        addView(table_1, 1);
        addView(table_2, 2);
        addView(table_3, 3);
    }
}

void RegistersView::setNameForTable(int idx, const QString& name)
{
    kDebug() << name << " " << idx;
    const QString text = tabWidget->tabText(idx);
    if (!text.contains(name)) {
        tabWidget->setTabText(idx, text.isEmpty() ? name : text + '/' + name);
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

void RegistersView::clear()
{
    for (int i = 0; i < TABLES_COUNT; i++) {
        tabWidget->setTabText(i, "");
    }
}

}
