/***************************************************************************
 *   Copyright 2007 by Dukju Ahn                                       *
 *   dukjuahn@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SVN_MODELS_H
#define SVN_MODELS_H

#include <QAbstractItemModel>
#include <QVariant>
#include <QList>
#include <kurl.h>
#include <ioutputviewmodel.h>
#include <QStandardItemModel>

extern "C" {
#include <svn_client.h>
}


class QModelIndex;
class KDevSubversionPart;

class SvnGenericHolder {
public:
    virtual QVariant variant(int col)=0;
};
/** represent one revision for blame-fetched data
 */
class SvnBlameHolder : public SvnGenericHolder {
public:
    virtual QVariant variant( int col );

    long lineNo;
    long int revNo;
    QString author;
    QString date;
    QString contents;
};

/** represent one revision for logview-fetched data
 */
class SvnLogHolder : public SvnGenericHolder {
public:
    virtual QVariant variant(int col);

    long int rev;
    QString author;
    QString date;
    QString logmsg;
    QString pathlist;
};
/// represent the status of one item
class SvnStatusHolder : public SvnGenericHolder {
public:
    virtual QVariant variant(int col);

    static QString statusToString( int status );

    QString wcPath;
    QString entityName;
    long int baseRevision;
/// 0 for absent, 1 for file, 2 for dir, 3 for unknown items.
    int nodeKind;
/// see svn_wc_status_kind at svn_wc.h
    int textStatus;
    int propStatus;
    bool locked;
    bool copied;
    bool switched;
    int reposTextStat;
    int reposPropStat;
};

/// A structure which describes various system-generated metadata about
/// a working-copy path or URL.
typedef struct svn_info_t SvnInfoHolder ;

/** data repository interface that is used by ParentlessModel
 */
class TreeItemIface {
public:
//     virtual int columnCount() = 0;
//     virtual int rowCount() = 0;
//     virtual QVariant data(int row, int col) = 0;
    static bool intLessThan( SvnGenericHolder &h1, SvnGenericHolder &h2 );
    static bool intGreaterThan( SvnGenericHolder &h1, SvnGenericHolder &h2 );
//     virtual void sort( int col, Qt::SortOrder order = Qt::AscendingOrder ) = 0;
};

// class BlameItem : public TreeItemIface {
// public:
//     BlameItem( QList<SvnBlameHolder> datalist );
//     BlameItem();
//     ~BlameItem();
//     void setHolderList( QList<SvnBlameHolder> dataList );
// //     int columnCount();
//     int rowCount();
//     QVariant data( int row, int col );
//     void sort( int column, Qt::SortOrder order = Qt::AscendingOrder );
//
//     QList<SvnBlameHolder> m_itemList;
// };
//
// /** data repository for Logview-fetched data
//  */
// class LogItem : public TreeItemIface {
// public:
//     LogItem(QList<SvnLogHolder> datalist);
//     LogItem();
//     ~LogItem();
//     void setHolderList(QList<SvnLogHolder> dataList);
// //     int columnCount();
//     int rowCount();
//     QVariant data(int row, int col);
// //     static bool intLessThan( SvnGenericHolder &h1, SvnGenericHolder &h2 );
// //     static bool intGreaterThan( SvnGenericHolder &h1, SvnGenericHolder &h2 );
//     void sort(  int column, Qt::SortOrder order = Qt::AscendingOrder );
//
//     QList<SvnLogHolder> m_itemList;
// };
template <class T>
class ResultItem{
public:
    ResultItem( QList<T> datalist ){ m_itemList = datalist; }
    ResultItem(){};
    ~ResultItem(){};
    void setHolderList( QList<T> datalist ) { m_itemList= datalist;}
    int rowCount();
    QVariant data( int row, int col );
    void sort( int column, Qt::SortOrder order = Qt::AscendingOrder );

protected:
    QList< T > m_itemList;
};
typedef ResultItem<SvnBlameHolder> BlameItem;
typedef ResultItem<SvnLogHolder> LogItem;


/** Custom Model for TreeView of KDevSubversionView.
 *  Reimplemented because of the need of sorting by int.
 *  Default sorting of QTreeWidget is QString-based, while logviewer
 *  should sort by revision int number.
 */
class ParentlessTreeModel: public QAbstractItemModel
{
    Q_OBJECT
public:
//     explicit ParentlessTreeModel( TreeItemIface *item, QObject *parent = 0 ):QAbstractItemModel(parent)
//     {
//         rootItem = item;
//     }
    virtual ~ParentlessTreeModel(){}

// implementing pure virtual methods.
    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const = 0;
    virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const = 0 ;
    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const = 0;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    QModelIndex index( int row, int col, const QModelIndex &parent=QModelIndex() ) const;
    QModelIndex parent( const QModelIndex &parent=QModelIndex()) const;

    void sort ( int column, Qt::SortOrder order = Qt::AscendingOrder )= 0;

// item updating methods
    //void setHolderList(QList<SvnGenericHolder> datalist);
    //void setNewItem( TreeItemIface* newItem );
    void prepareItemUpdate();
    void finishedItemUpdate();

protected:
//     TreeItemIface *rootItem;
};
/** implement headerData() data() columnCount() for blame */
class BlameTreeModel : public ParentlessTreeModel {
    Q_OBJECT
public:
    explicit BlameTreeModel( BlameItem *item, QObject *parent=0 );
    virtual ~BlameTreeModel();
    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const ;
    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const ;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    void sort ( int column, Qt::SortOrder order = Qt::AscendingOrder );
protected:
    BlameItem *rootItem;
};


/** reimplemented headerData() data() columnCount for logview widget
 */
class LogviewTreeModel : public ParentlessTreeModel {
    Q_OBJECT
public:
    explicit LogviewTreeModel( LogItem *item, QObject *parent = 0 );
    virtual ~LogviewTreeModel();
    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const ;
    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const ;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    void sort ( int column, Qt::SortOrder order = Qt::AscendingOrder );
    /// on error, return -1
    long revision( const QModelIndex &index ) const;
    QStringList modifiedLists( const QModelIndex &index ) const;
protected:
    LogItem *rootItem;
};
/** Custom ListView model for KDevSubversionView
*/
class LogviewDetailedModel : public QAbstractListModel {
    Q_OBJECT
public:
    explicit LogviewDetailedModel( LogItem *item, QObject *parent = 0 );
    virtual ~LogviewDetailedModel();
    virtual int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const ;

public Q_SLOTS:
    void setNewRevision( const QModelIndex &index );

protected:
    int m_activeRow;
    QList<QString> m_pathlist;
    LogItem *m_item;
};

/**
 * Model that is used by outputview notification.
 * When activated, open the file. Depending on the result of update operation, it paints
 * with corresponding color. For example, regarding conflicted item, it paints red color
 * on that file.
 *
 * TODO Clearing the list by context menu.
 */

class SvnOutputItem : public QStandardItem
{
public:
    explicit SvnOutputItem( const QString &path, const QString &msg );
    virtual ~SvnOutputItem();
    bool stopHere();
    QString m_path;
    QString m_msg;
    bool m_stop;
};

class SvnOutputModel : public QStandardItemModel, public KDevelop::IOutputViewModel
{
    Q_OBJECT
public:
    explicit SvnOutputModel( KDevSubversionPart *part, QObject *parent );
    ~SvnOutputModel();

    // IOutputViewModel interfaces.
    /// Open the file
    void activate( const QModelIndex& index );

    /// Highlight next conflicted item in list.
    QModelIndex nextHighlightIndex( const QModelIndex& currentIndex );

    /// Highlight previous conflicted item in list.
    QModelIndex previousHighlightIndex( const QModelIndex& currentIndex );

private:
    KDevSubversionPart *m_part;
};

#endif
