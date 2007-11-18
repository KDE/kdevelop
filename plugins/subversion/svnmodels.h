/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SVN_MODELS_H
#define SVN_MODELS_H

#include <kurl.h>
#include <QString>
#include <QModelIndex>
#include <QDateTime>
#include <QVariant>

/**
 * Data classes to hold information from some svn-structures
 *
 * Also models on top of these classes
 *
 * @TODO: might be removed later by making the model class a template and providing data() functions on the individual Holder classes
 *
 */

/** represent one revision for blame-fetched data
 */
class SvnBlameHolder {
public:
    qlonglong lineNo;
    qlonglong revNo;
    QString author;
    QDateTime date;
    QString contents;
};

class SvnBlameModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    SvnBlameModel( QObject* );
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
    int columnCount( const QModelIndex& parent = QModelIndex() ) const;
    int rowCount( const QModelIndex& parent = QModelIndex() ) const;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    void setBlameList( const QList<SvnBlameHolder>& newlist );
private:
    QList<SvnBlameHolder> m_blame;
};

class SvnChangedPath
{
public:
    QString path;
    KUrl copyFromPath;
    qlonglong copyFromRevision;
    QChar action;
};


/** represent one revision for logview-fetched data
 */
class SvnLogHolder {
public:
    qlonglong rev;
    QString author;
    QDateTime date;
    QString logmsg;
    QList<SvnChangedPath> pathlist;
};

class SvnLogModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    SvnLogModel( QObject* );
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
    int columnCount( const QModelIndex& parent = QModelIndex() ) const;
    int rowCount( const QModelIndex& parent = QModelIndex() ) const;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    void setLogList( const QList<SvnLogHolder>& newlist );
    SvnLogHolder logEntryForIndex( const QModelIndex& ) const;
private:
    QList<SvnLogHolder> m_log;
};

class SvnChangedPathModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    SvnChangedPathModel( QObject* );
    QVariant data( const QModelIndex& index, int role = Qt::DisplayRole ) const;
    int columnCount( const QModelIndex& parent = QModelIndex() ) const;
    int rowCount( const QModelIndex& parent = QModelIndex() ) const;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    void setChangedPaths( const QList<SvnChangedPath>& newlist );
    SvnChangedPath changedPathForIndex( const QModelIndex& index ) const;
private:
    QList<SvnChangedPath> m_changedPaths;
};

/// represent the status of one item
class SvnStatusHolder {
public:
    static QString statusToString( int status );

    QString wcPath;
    QString entityName;
    qlonglong baseRevision;
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

#endif
