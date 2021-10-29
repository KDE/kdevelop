/*
    SPDX-FileCopyrightText: 2010 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later
*/

#ifndef INCLUDESMODEL_H
#define INCLUDESMODEL_H

#include <QAbstractListModel>
#include <QStringList>

class IncludesModel : public QAbstractListModel
{
Q_OBJECT
public:
    explicit IncludesModel( QObject* parent = nullptr );
    void setIncludes( const QStringList&  );
    QStringList includes() const;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool removeRows( int row, int count, const QModelIndex& parent = QModelIndex() ) override;
    void addInclude( const QString& );
private:
    QStringList m_includes;
    void addIncludeInternal( const QString& includePath );
};

#endif // INCLUDESMODEL_H
