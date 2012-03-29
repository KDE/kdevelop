/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef MAKEOUTPUTMODEL_H
#define MAKEOUTPUTMODEL_H

#include <QtCore/QAbstractListModel>
#include <QString>
#include <kurl.h>
#include <outputview/ioutputviewmodel.h>
#include <set>
#include <QLinkedList>
#include <QMap>
#include <QVector>

class FilteredItem
{
public:
    FilteredItem( const QString& line );
    QString originalLine;
    QVariant type;
    QString shortenedText;
    bool isActivatable;
    KUrl url;
    int lineNo, columnNo;
};

class MakeOutputModel : public QAbstractListModel, public KDevelop::IOutputViewModel
{
    Q_OBJECT
public:
    enum OutputItemType{
        ErrorItem = 1,
        WarningItem = 2,
        ActionItem = 3,
        CustomItem = 4,
        StandardItem = 5,
        InformationItem = 6
    };

    static const int MakeItemTypeRole;

    explicit MakeOutputModel( const KUrl& builddir , QObject* parent = 0 );

    // IOutputViewModel interfaces
    void activate( const QModelIndex& index );
    QModelIndex nextHighlightIndex( const QModelIndex &current );
    QModelIndex previousHighlightIndex( const QModelIndex &current );

    QVariant data( const QModelIndex&, int = Qt::DisplayRole ) const;
    int rowCount( const QModelIndex& = QModelIndex() ) const;
    QVariant headerData( int, Qt::Orientation, int = Qt::DisplayRole ) const;

    void addLines( const QStringList& );
    void addLine( const QString& );

private:
    KUrl urlForFile( const QString& ) const;
    bool isValidIndex( const QModelIndex& ) const;
    QList<FilteredItem> items;
    // We use std::set because that is ordered
    std::set<int> errorItems; // Indices of all items that are errors

    QLinkedList<QString> currentDirs;
    typedef QMap<QString, QLinkedList<QString>::iterator> PositionMap;
    PositionMap positionInCurrentDirs;

    KUrl buildDir;
};

Q_DECLARE_METATYPE( MakeOutputModel::OutputItemType )

#endif

