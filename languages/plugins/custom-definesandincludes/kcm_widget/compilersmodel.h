/*
 * This file is part of KDevelop
 *
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>
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
 *
 */

#ifndef COMPILERMODEL_H
#define COMPILERMODEL_H

#include <QAbstractTableModel>
#include <QHash>

#include "../compilerprovider/icompiler.h"

class CompilersModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    CompilersModel( QObject* parent = 0 );
    void setCompilers( const QVector<CompilerPointer>& compilers );
    QVector<CompilerPointer> compilers() const;
    void addCompiler(const CompilerPointer& compiler);

    virtual QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const override;
    virtual int rowCount( const QModelIndex& parent = QModelIndex() ) const override;
    virtual Qt::ItemFlags flags( const QModelIndex& index ) const override;
    virtual int columnCount( const QModelIndex& parent = QModelIndex() ) const override;
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    virtual bool setData( const QModelIndex& index, const QVariant& value, int role = Qt::EditRole ) override;
    virtual bool removeRows( int row, int count, const QModelIndex& parent = QModelIndex() ) override;

private:
    QVector<CompilerPointer> m_compilers;
};

#endif // COMPILERMODEL_H
