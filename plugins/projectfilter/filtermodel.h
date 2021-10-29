/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef FILTERMODEL_H
#define FILTERMODEL_H

#include <QAbstractTableModel>

#include "filter.h"

namespace KDevelop {

class FilterModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit FilterModel(QObject* parent = nullptr);
    ~FilterModel() override;

    SerializedFilters filters() const;
    void setFilters(const SerializedFilters& filters);

    void moveFilterUp(int row);
    void moveFilterDown(int row);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    Qt::DropActions supportedDropActions() const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    QMap< int, QVariant > itemData(const QModelIndex& index) const override;
    bool setItemData(const QModelIndex& index, const QMap< int, QVariant >& roles) override;

    enum Columns {
        Pattern,
        Targets,
        Inclusive,
        NUM_COLUMNS
    };

private:
    SerializedFilters m_filters;
    // workaround a strange behavior in Qt when we try to drop after the last item in the list
    bool m_ignoredLastInsert;
};

}

#endif // FILTERMODEL_H
