/*
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef COMPILERSMODEL_H
#define COMPILERSMODEL_H

#include <QAbstractItemModel>
#include <QItemSelection>

#include "../compilerprovider/icompiler.h"

class TreeItem;

class CompilersModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum SpecialRole {
        CompilerDataRole = Qt::UserRole + 1
    };

    explicit CompilersModel( QObject* parent = nullptr );
    ~CompilersModel() override;

    void setCompilers( const QVector<CompilerPointer>& compilers );
    QVector<CompilerPointer> compilers() const;
    QModelIndex addCompiler(const CompilerPointer& compiler);
    void updateCompiler(const QItemSelection& compiler);

Q_SIGNALS:
    /// emitted whenever new compiler added or existing one modified/deleted.
    void compilerChanged();

public:
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const override;
    int rowCount( const QModelIndex& parent = QModelIndex() ) const override;
    Qt::ItemFlags flags( const QModelIndex& index ) const override;
    int columnCount( const QModelIndex& parent = QModelIndex() ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& child) const override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

private:
    TreeItem* m_rootItem;
};

#endif // COMPILERMODEL_H
