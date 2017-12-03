/************************************************************************
 *                                                                      *
 * Copyright 2010 Andreas Pakulat <apaku@gmx.de>                        *
 *                                                                      *
 * This program is free software; you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation; either version 2 or version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful, but  *
 * WITHOUT ANY WARRANTY; without even the implied warranty of           *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU     *
 * General Public License for more details.                             *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program; if not, see <http://www.gnu.org/licenses/>. *
 ************************************************************************/

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
