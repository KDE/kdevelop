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

#ifndef COMBOBOXDELEGATE_H
#define COMBOBOXDELEGATE_H

#include <QStyledItemDelegate>

namespace KDevelop {

class ComboBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:

    struct Item {
        Item()
        {}
        Item(const QString& text, const QVariant& data)
            : text(text)
            , data(data)
        {}
        QString text;
        QVariant data;
    };

    explicit ComboBoxDelegate(const QVector<Item>& items, QObject* parent = 0);
    virtual ~ComboBoxDelegate();

    virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    virtual void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    virtual void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

private:
    QVector<Item> m_items;
};

}

Q_DECLARE_TYPEINFO(KDevelop::ComboBoxDelegate::Item, Q_MOVABLE_TYPE);

#endif // COMBOBOXDELEGATE_H
