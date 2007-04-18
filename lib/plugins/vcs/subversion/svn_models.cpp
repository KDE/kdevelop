/***************************************************************************
 *   Copyright (C) 2007 by Dukju Ahn                                       *
 *   dukjuahn@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#include "svn_models.h"

#include <klocale.h>

#include <QtAlgorithms>
#include <QModelIndex>
#include <QStringList>

QVariant SvnBlameHolder::variant( int col )
{
    switch( col ){
        case 0:
            return QString::number(lineNo);
        case 1:
            return QString::number(revNo);
        case 2:
            return author;
        case 3:
            return date.left(10);
        case 4:
            return contents;
        default:
            return QVariant();
    }
}

//////////////////////////////////
QVariant SvnLogHolder::variant(int col)
{
    switch( col ){
        case 0:
            return QString::number(rev);
        case 1:
            return author;
        case 2:{
//             QString prettyDate = date.left(16).replace(10, 1, ' ');
//             return prettyDate;
            return date.left(10);
        }
        case 3:
            return logmsg;
        case 4:
            return pathlist;
        default:
            return QVariant();
    }
}
//////////////////////////////////
QVariant SvnStatusHolder::variant(int col)
{
    // TODO refactor this function.
    return QVariant();
}
//////////////////////////////////
bool TreeItemIface::intLessThan( SvnGenericHolder &h1, SvnGenericHolder &h2 )
{
    int h1Int = h1.variant(0).toInt();
    int h2Int = h2.variant(0).toInt();
    return (h1Int < h2Int) ;
}
bool TreeItemIface::intGreaterThan( SvnGenericHolder &h1, SvnGenericHolder &h2 )
{
    int h1Int = h1.variant(0).toInt();
    int h2Int = h2.variant(0).toInt();
    return (h1Int > h2Int) ;
}
/////////////////////////////////////////////////////
template <class T>
int ResultItem<T>::rowCount()
{
    return m_itemList.count();
}
template <class T>
QVariant ResultItem<T>::data( int row, int col )
{
    if( row < 0 ) return QVariant();
    if( row >= m_itemList.size() ) return QVariant();
    T oneRow = m_itemList.value(row);
    return oneRow.variant(col);
}
template <class T>
void ResultItem<T>::sort( int column, Qt::SortOrder order )
{
    if( order == Qt::AscendingOrder )
        qSort( m_itemList.begin(), m_itemList.end(), TreeItemIface::intLessThan );
    else
        qSort( m_itemList.begin(), m_itemList.end(), TreeItemIface::intGreaterThan );
}

////////////////////////////////////////////////////
// int ParentlessTreeModel::rowCount( const QModelIndex & parent ) const
// {
//     return rootItem->rowCount();
// }
// int ParentlessTreeModel::columnCount( const QModelIndex &  parent )const 
// {
//     return rootItem->columnCount();
// }
// QVariant ParentlessTreeModel::data( const QModelIndex & index, int role )const
// {
//     if( role!= Qt::DisplayRole )
//         return QVariant();
// //     SvnGenericHolder holder = rootItem->itemList.value( index.row() );
// //     return holder.variant( index.column() );
//     return rootItem->data( index.row(), index.column() );
// }
Qt::ItemFlags ParentlessTreeModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable ;
}
QVariant ParentlessTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( orientation == Qt::Horizontal && role == Qt::DisplayRole )
        return QString("");
    return QVariant();

}
QModelIndex ParentlessTreeModel::index( int row, int col, const QModelIndex &parent) const
{
    if ( !parent.isValid() ) // toplevel
        return createIndex(row, col, 0);
    return QModelIndex();
    
}
QModelIndex ParentlessTreeModel::parent( const QModelIndex &parent ) const
{
    return QModelIndex();
}
// void ParentlessTreeModel::sort ( int column, Qt::SortOrder order )
// {
//     emit layoutAboutToBeChanged();
//     rootItem->sort(column, order);
//     emit layoutChanged();
// }
// void ParentlessTreeModel::setHolderList(QList<SvnGenericHolder> datalist)
// {
//     emit layoutAboutToBeChanged();
//     rootItem->setHolderList( datalist );
//     emit layoutChanged();
// }
// void ParentlessTreeModel::setNewItem( TreeItemIface* newItem )
// {
//     emit layoutAboutToBeChanged();
//     rootItem = newItem;
//     emit layoutChanged();
// }
void ParentlessTreeModel::prepareItemUpdate()
{
    emit layoutAboutToBeChanged();
}
void ParentlessTreeModel::finishedItemUpdate()
{
    emit layoutChanged();
}
//////////////////////////////////
BlameTreeModel::BlameTreeModel( BlameItem *item, QObject *parent )
    : ParentlessTreeModel ()
{
    rootItem = item;
}
BlameTreeModel::~BlameTreeModel()
{}
int BlameTreeModel::rowCount ( const QModelIndex & parent  ) const
{
    return rootItem->rowCount();
}
int BlameTreeModel::columnCount ( const QModelIndex & parent ) const
{
    return 5;
}
QVariant BlameTreeModel::data( const QModelIndex & index, int role )const
{
    if( role!= Qt::DisplayRole || !index.isValid() )
        return QVariant();
    if( index.column() == 0 ){
        return QString("   ") + rootItem->data( index.row(), index.column() ).toString();
    }
    //if revision is same with previous one, do not print revision number again
    if( index.column() == 1 || index.column() == 2 || index.column() == 3){
        
        if( index.row() == 0 )
            return rootItem->data( 0, index.column() ).toString();
        
        QString prevRev = rootItem->data( index.row() - 1, 1 ).toString();
        QString currentRev = rootItem->data( index.row(), 1 ).toString();
        
        if( prevRev == currentRev ){
            return QVariant();
        }
        else{
            return rootItem->data( index.row(), index.column() ).toString();
        }
        
    }
    // case of lineNo and Contents
    return rootItem->data( index.row(), index.column() );
}
QVariant BlameTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( orientation == Qt::Horizontal && role == Qt::DisplayRole )
        switch (section){
            case 0:
                return QString(i18n("Line"));
            case 1:
                return QString(i18n("Rev"));
            case 2:
                return QString(i18n("Author"));
            case 3:
                return QString(i18n("Date"));
            case 4:
                return QString(i18n("Contents"));
            default:
                return QVariant();
        };
        return QVariant();
}
void BlameTreeModel::sort ( int column, Qt::SortOrder order )
{
    emit layoutAboutToBeChanged();
    rootItem->sort(column, order);
    emit layoutChanged();
}
/////////////////////////////////////////////////////////////////
LogviewTreeModel::LogviewTreeModel( LogItem *item, QObject *parent )
    : ParentlessTreeModel ( /*item, parent*/ )
{
    rootItem = item;
}
LogviewTreeModel::~LogviewTreeModel()
{}
int LogviewTreeModel::rowCount ( const QModelIndex & parent  ) const
{
    return rootItem->rowCount();
}
int LogviewTreeModel::columnCount ( const QModelIndex & parent ) const
{
    return 4;
}
QVariant LogviewTreeModel::data( const QModelIndex & index, int role )const
{
    if( role!= Qt::DisplayRole )
        return QVariant();
    if( !index.isValid() )
        return QVariant();
    
    if( index.column() == 3 ){
        //case of whitespace-stripped comment
        return rootItem->data( index.row(), index.column() ).toString().simplified();
    } else if ( index.column() == 0 ){
        return QString("  ") + rootItem->data( index.row(), 0 ).toString();
    }
    
    return rootItem->data( index.row(), index.column() );
}
QVariant LogviewTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if( orientation == Qt::Horizontal && role == Qt::DisplayRole )
        switch (section){
            case 0:
                return QString(i18n("Rev"));
            case 1:
                return QString(i18n("Author"));
            case 2:
                return QString(i18n("Date"));
            case 3:
                return QString(i18n("Comment"));
            default:
                return QVariant();
        };
    return QVariant();
}
void LogviewTreeModel::sort ( int column, Qt::SortOrder order )
{
    emit layoutAboutToBeChanged();
    rootItem->sort(column, order);
    emit layoutChanged();
}
long LogviewTreeModel::revision( const QModelIndex &index ) const
{
    bool conversionOk = true;
    long revNo = rootItem->data( index.row(), 0 ).toString().toLong( &conversionOk );
    if( !conversionOk )
        return -1;
    else
        return revNo;
}
QStringList LogviewTreeModel::modifiedLists( const QModelIndex &index ) const
{
    QString pathlist = rootItem->data( index.row(), 4 ).toString();
    QStringList ret = pathlist.split( "\n", QString::SkipEmptyParts );
    return ret;
}
//////////////////////////////////
LogviewDetailedModel::LogviewDetailedModel( LogItem *item, QObject *parent )
    : QAbstractListModel(parent), m_item(item)
{}
LogviewDetailedModel::~LogviewDetailedModel()
{}
int LogviewDetailedModel::rowCount( const QModelIndex &index ) const
{
    Q_UNUSED(index)
    return m_pathlist.count() + 1; // modified paths + log message
}
QVariant LogviewDetailedModel::data( const QModelIndex &index, int role ) const
{
    if( role!= Qt::DisplayRole )
        return QVariant();
    if( index.row() == rowCount()-1 ){
        // case of last index. return commit message prepended with \n
        return ( QString("\n") + m_item->data( m_activeRow, 3 ).toString() );
    }
    if( index.isValid() ){
        // return modified path
        return m_pathlist.at( index.row() );
    }
    return QVariant();
}

void LogviewDetailedModel::setNewRevision( const QModelIndex &index )
{
    kDebug() << " LogviewDetailedModel::setNewRevision " << endl;
    emit layoutAboutToBeChanged();
    m_activeRow = index.row();
    m_pathlist =
        m_item->data( m_activeRow, 4 ).toString().split( "\n", QString::SkipEmptyParts );
    emit layoutChanged();
}

#include "svn_models.moc"



