/* This file is part of KDevelop
    Copyright 2019 Daniel Mensinger <daniel@mensinger-ka.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "mesonlisteditor.h"

#include "ui_mesonlisteditor.h"

QListWidgetItem* genItem(QString const& label)
{
    QListWidgetItem* item = new QListWidgetItem(label);
    item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
    return item;
}

MesonListEditor::MesonListEditor(QStringList content, QWidget* parent)
    : QDialog(parent)
{
    m_ui = new Ui::MesonListEditor;
    m_ui->setupUi(this);

    for (auto const& i : content) {
        m_ui->array->addItem(genItem(i));
    }

    currentItemChanged();
}

MesonListEditor::~MesonListEditor()
{
    if (m_ui) {
        delete m_ui;
    }
}

QStringList MesonListEditor::content() const
{
    QStringList content;
    content.reserve(m_ui->array->count());
    for (int i = 0; i < m_ui->array->count(); ++i) {
        content << m_ui->array->item(i)->text();
    }
    return content;
}

void MesonListEditor::add()
{
    auto* item = genItem(i18n("<NEW>"));
    m_ui->array->addItem(item);
    m_ui->array->setCurrentItem(item);
    m_ui->array->editItem(item);
}

void MesonListEditor::moveItem(int src, int dst)
{
    auto* item = m_ui->array->takeItem(src);
    if (!item) {
        return;
    }

    m_ui->array->insertItem(dst, item);
    m_ui->array->setCurrentItem(item);
}

void MesonListEditor::remove()
{
    qDeleteAll(m_ui->array->selectedItems());
}

void MesonListEditor::first()
{
    int row = m_ui->array->currentRow();
    moveItem(row, 0);
}

void MesonListEditor::up()
{
    int row = m_ui->array->currentRow();
    moveItem(row, row - 1);
}

void MesonListEditor::down()
{
    int row = m_ui->array->currentRow();
    moveItem(row, row + 1);
}

void MesonListEditor::last()
{
    int row = m_ui->array->currentRow();
    moveItem(row, m_ui->array->count() - 1);
}

void MesonListEditor::currentItemChanged()
{
    auto* current = m_ui->array->currentItem();
    if (!current || m_ui->array->count() == 0) {
        m_ui->b_first->setDisabled(true);
        m_ui->b_up->setDisabled(true);
        m_ui->b_down->setDisabled(true);
        m_ui->b_last->setDisabled(true);
        m_ui->b_del->setDisabled(true);
        return;
    }

    int row = m_ui->array->row(current);

    m_ui->b_del->setDisabled(false);

    if (m_ui->array->count() < 2) {
        m_ui->b_first->setDisabled(true);
        m_ui->b_up->setDisabled(true);
        m_ui->b_down->setDisabled(true);
        m_ui->b_last->setDisabled(true);
    } else if (row == 0) {
        m_ui->b_first->setDisabled(true);
        m_ui->b_up->setDisabled(true);
        m_ui->b_down->setDisabled(false);
        m_ui->b_last->setDisabled(false);
    } else if (row >= m_ui->array->count() - 1) {
        m_ui->b_first->setDisabled(false);
        m_ui->b_up->setDisabled(false);
        m_ui->b_down->setDisabled(true);
        m_ui->b_last->setDisabled(true);
    } else {
        m_ui->b_first->setDisabled(false);
        m_ui->b_up->setDisabled(false);
        m_ui->b_down->setDisabled(false);
        m_ui->b_last->setDisabled(false);
    }
}
