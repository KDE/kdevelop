/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
*/

#ifndef DEFINESMODEL_H
#define DEFINESMODEL_H

#include <QAbstractTableModel>

#include "idefinesandincludesmanager.h"

class DefinesModel : public QAbstractTableModel
{
Q_OBJECT
public:
    explicit DefinesModel( QObject* parent = nullptr );
    void setDefines( const KDevelop::Defines& defines );
    KDevelop::Defines defines() const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool removeRows( int row, int count, const QModelIndex& parent = QModelIndex() ) override;
private:
    QVector<QPair<QString, QString>> m_defines;
};

#endif // DEFINESMODEL_H
