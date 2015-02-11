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

#ifndef INCLUDESMODEL_H
#define INCLUDESMODEL_H

#include <QAbstractListModel>
#include <QStringList>

class IncludesModel : public QAbstractListModel
{
Q_OBJECT
public:
    IncludesModel( QObject* parent = 0 );
    void setIncludes( const QStringList&  );
    QStringList includes() const;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
    virtual bool removeRows( int row, int count, const QModelIndex& parent = QModelIndex() ) override;
    void addInclude( const QString& );
private:
    QStringList m_includes;
    void addIncludeInternal( const QString& includePath );
};

#endif // INCLUDESMODEL_H
