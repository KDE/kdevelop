/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "svnmodels.h"
#include "subversionpart.h"
#include "icore.h"
#include "idocumentcontroller.h"

extern "C"{
#include <svn_wc.h>
}

#include <klocale.h>
#include <kcolorscheme.h>

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
QVariant SvnStatusHolder::variant(int /*col*/)
{
    // TODO refactor this function.
    return QVariant();
}

QString SvnStatusHolder::statusToString( int status )
{
    switch( status ){
        case svn_wc_status_none: return QString("NoExist");
        case svn_wc_status_unversioned: return QString("unversioned");
        case svn_wc_status_normal: return QString("up-to-date");
        case svn_wc_status_added: return QString("added");
        case svn_wc_status_missing: return QString("missing");
        case svn_wc_status_deleted: return QString("deleted");
        case svn_wc_status_replaced: return QString("replaced");
        case svn_wc_status_modified: return QString("modified");
        case svn_wc_status_merged: return QString("merged");
        case svn_wc_status_conflicted: return QString("conflict");
        case svn_wc_status_ignored: return QString("ignored");
        case svn_wc_status_obstructed: return QString("obstructed");
        case svn_wc_status_external: return QString("external");
        case svn_wc_status_incomplete: return QString("incomplete");
        default:
            return QString("unknown");
    }
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
void ResultItem<T>::sort( int /*column*/, Qt::SortOrder order )
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
Qt::ItemFlags ParentlessTreeModel::flags(const QModelIndex &/*index*/) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable ;
}
QVariant ParentlessTreeModel::headerData(int /*section*/, Qt::Orientation orientation, int role) const
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
QModelIndex ParentlessTreeModel::parent( const QModelIndex &/*parent*/ ) const
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
BlameTreeModel::BlameTreeModel( BlameItem *item, QObject */*parent*/ )
    : ParentlessTreeModel ()
{
    rootItem = item;
}
BlameTreeModel::~BlameTreeModel()
{}
int BlameTreeModel::rowCount ( const QModelIndex & /*parent*/  ) const
{
    return rootItem->rowCount();
}
int BlameTreeModel::columnCount ( const QModelIndex & /*parent*/ ) const
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
                return QString(i18nc("line number in a text file", "Line"));
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
LogviewTreeModel::LogviewTreeModel( LogItem *item, QObject */*parent*/ )
    : ParentlessTreeModel ( /*item, parent*/ )
{
    rootItem = item;
}
LogviewTreeModel::~LogviewTreeModel()
{}
int LogviewTreeModel::rowCount ( const QModelIndex & /*parent*/  ) const
{
    return rootItem->rowCount();
}
int LogviewTreeModel::columnCount ( const QModelIndex & /*parent*/ ) const
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
    kDebug(9500) << "LogviewDetailedModel::setNewRevision";
    emit layoutAboutToBeChanged();
    m_activeRow = index.row();
    m_pathlist =
        m_item->data( m_activeRow, 4 ).toString().split( "\n", QString::SkipEmptyParts );
    emit layoutChanged();
}

/////////////////////////////////////////////////////

SvnOutputItem::SvnOutputItem( const QString &path, const QString &msg )
    : QStandardItem(msg)
    , m_path(path), m_msg(msg), m_stop(false)
{
    //conflict
    if( m_msg.contains("Conflicted") ){
        //Need to use KCS instead of KStatefulBrush because we don't have a
	//widget or QPalette here
        setForeground(KColorScheme(QPalette::Active).foreground(KColorScheme::NegativeText));
        m_stop = true;
    }
}

SvnOutputItem::~SvnOutputItem()
{}

bool SvnOutputItem::stopHere()
{
    return m_stop;
}

SvnOutputModel::SvnOutputModel( KDevSubversionPart *part, QObject *parent )
    : QStandardItemModel( parent )
    , m_part(part)
{}
SvnOutputModel::~SvnOutputModel()
{}

void SvnOutputModel::activate( const QModelIndex& index )
{
    QStandardItem *item = itemFromIndex( index );
    SvnOutputItem *svnItem = dynamic_cast<SvnOutputItem*>(item);
    if(!item) return;

    if( svnItem->m_path.isEmpty() )
        return;

    m_part->core()->documentController()->openDocument( KUrl(svnItem->m_path) );
}

QModelIndex SvnOutputModel::nextHighlightIndex( const QModelIndex& currentIdx )
{
    int rowCount = this->rowCount();

    bool reachedEnd = false;

    // determine from which index we should start
    int i=0;
    if( currentIdx.isValid() )
        i = currentIdx.row();

    if( i >= rowCount - 1 )
        i = 0;
    else
        i++;

    for( ; i < rowCount; i++ )
    {
        QStandardItem *stditem = item( i );
        SvnOutputItem *outItem = dynamic_cast<SvnOutputItem*>( stditem );
        if( outItem && outItem->stopHere() )
        {
            // yes. found.
            QModelIndex modelIndex = outItem->index();
            return modelIndex;
        }

        if( i >= rowCount - 1 ) // at the last index and couldn't find error yet.
        {
            if( reachedEnd )
            {
                break; // no matching item
            }
            else
            {
                reachedEnd = true;
                i = -1; // search from index 0
            }
        }
    }
    return QModelIndex();
}

QModelIndex SvnOutputModel::previousHighlightIndex( const QModelIndex& currentIdx )
{
    int rowCount = this->rowCount();

    bool reachedFirst = false;

    // determine from which index we should start
    int i = rowCount -1;
    if( currentIdx.isValid() )
        i = currentIdx.row();

    if( ( i > rowCount - 1 ) || ( i == 0 ) )
        i = rowCount-1; // set to last index
    else
        i--;

    for( ; i >= 0; i-- )
    {
        QStandardItem *stditem = item( i );
        SvnOutputItem *outItem = dynamic_cast<SvnOutputItem*>( stditem );
        if( outItem && outItem->stopHere() )
        {
            // yes. found.
            QModelIndex modelIndex = outItem->index();
            return modelIndex;
        }
        if( i <= 0 ) // at the last index and couldn't find error yet.
        {
            if( reachedFirst )
            {
                break; // no matching item
            }
            else
            {
                reachedFirst = true;
                i = rowCount; // search from last index
            }
        }
    }
    return QModelIndex();
}

#include "svnmodels.moc"



