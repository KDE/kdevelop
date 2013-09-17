/*
 * This file is part of KDevelop
 * Copyright 2013 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "comboboxdelegate.h"

#include <QComboBox>

using namespace KDevelop;

ComboBoxDelegate::ComboBoxDelegate(const QVector<Item>& items, QObject* parent)
    : QStyledItemDelegate(parent)
    , m_items(items)
{

}

ComboBoxDelegate::~ComboBoxDelegate()
{

}

QWidget* ComboBoxDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    QComboBox* ret = new QComboBox(parent);
    ret->setEditable(false);
    return ret;
}

void ComboBoxDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    Q_ASSERT(qobject_cast<QComboBox*>(editor));
    QComboBox* box = static_cast<QComboBox*>(editor);
    box->clear();
    const QString& current = index.data().toString();
    int currentIndex = -1;
    int i = 0;
    foreach(const Item& item, m_items) {
        if (item.text == current) {
            currentIndex = i;
        }
        box->addItem(item.text);
        i++;
    }
    if (currentIndex != -1) {
        box->setCurrentIndex(currentIndex);
    }
}

void ComboBoxDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    Q_ASSERT(qobject_cast<QComboBox*>(editor));
    QComboBox* box = static_cast<QComboBox*>(editor);
    model->setData(index, m_items.at(box->currentIndex()).data);
}

#include "comboboxdelegate.moc"
