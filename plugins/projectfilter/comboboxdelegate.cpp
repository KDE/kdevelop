/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
    auto* ret = new QComboBox(parent);
    ret->setEditable(false);
    return ret;
}

void ComboBoxDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    Q_ASSERT(qobject_cast<QComboBox*>(editor));
    auto* box = static_cast<QComboBox*>(editor);
    box->clear();
    const QString& current = index.data().toString();
    int currentIndex = -1;
    int i = 0;
    for (const Item& item : m_items) {
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
    auto* box = static_cast<QComboBox*>(editor);
    model->setData(index, m_items.at(box->currentIndex()).data);
}

#include "moc_comboboxdelegate.cpp"
