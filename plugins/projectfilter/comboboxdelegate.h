/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

    explicit ComboBoxDelegate(const QVector<Item>& items, QObject* parent = nullptr);
    ~ComboBoxDelegate() override;

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

private:
    QVector<Item> m_items;
};

}

Q_DECLARE_TYPEINFO(KDevelop::ComboBoxDelegate::Item, Q_MOVABLE_TYPE);

#endif // COMBOBOXDELEGATE_H
