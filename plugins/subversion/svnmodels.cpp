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

#include "svnmodels.h"

extern "C"{
#include <svn_wc.h>
}

#include <klocale.h>

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

SvnBlameModel::SvnBlameModel( QObject* parent )
    : QAbstractTableModel(parent)
{
}
QVariant SvnBlameModel::data( const QModelIndex& index, int role ) const
{
    if( role != Qt::DisplayRole || !index.isValid() )
        return QVariant();

    int row = index.row();
    if( row < 0 || row >= rowCount() )
        return QVariant();

    switch( index.column() )
    {
        case 0:
            return m_blame[row].lineNo;
            break;
        case 1:
            return m_blame[row].revNo;
            break;
        case 2:
            return m_blame[row].author;
            break;
        case 3:
            return m_blame[row].date;
            break;
        case 4:
            return m_blame[row].contents;
            break;
        default:
            break;
    }
    return QVariant();
}

int SvnBlameModel::columnCount( const QModelIndex& parent ) const
{
    Q_UNUSED(parent)
    return 5;
}

int SvnBlameModel::rowCount( const QModelIndex& parent ) const
{
    Q_UNUSED(parent)
    return m_blame.count();
}
QVariant SvnBlameModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if( orientation != Qt::Horizontal || role != Qt::DisplayRole )
        return QVariant();
    switch( section )
    {
        case 0:
            return i18nc("line number in a text file", "Line");
            break;
        case 1:
            return i18n("Revision");
            break;
        case 2:
            return i18n("Author");
            break;
        case 3:
            return i18n("Date");
            break;
        case 4:
            return i18n("Contents");
            break;
        default:
            break;
    }
    return QVariant();
}

void SvnBlameModel::setBlameList( const QList<SvnBlameHolder>& newlist )
{
    m_blame = newlist;
    reset();
}

SvnLogModel::SvnLogModel( QObject* parent )
    : QAbstractTableModel(parent)
{
}
QVariant SvnLogModel::data( const QModelIndex& index, int role ) const
{
    if( role != Qt::DisplayRole || !index.isValid() )
        return QVariant();

    int row = index.row();
    if( row < 0 || row >= rowCount() )
        return QVariant();

    switch( index.column() )
    {
        case 0:
            return m_log[row].rev;
            break;
        case 1:
            return m_log[row].author;
            break;
        case 2:
            return m_log[row].date;
            break;
        case 3:
            return m_log[row].logmsg;
            break;
        default:
            break;
    }
    return QVariant();
}
int SvnLogModel::columnCount( const QModelIndex& parent ) const
{
    Q_UNUSED(parent)
    return 4;
}
int SvnLogModel::rowCount( const QModelIndex& parent ) const
{
    Q_UNUSED(parent)
    return m_log.count();
}
QVariant SvnLogModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if( orientation != Qt::Horizontal || role != Qt::DisplayRole )
        return QVariant();
    switch( section )
    {
        case 0:
            return i18n("Revision");
            break;
        case 1:
            return i18n("Author");
            break;
        case 2:
            return i18n("Date");
            break;
        case 3:
            return i18n("Logmessage");
            break;
            break;
        default:
            break;
    }
    return QVariant();
}

void SvnLogModel::setLogList( const QList<SvnLogHolder>& newlist )
{
    m_log = newlist;
    reset();
}

SvnLogHolder SvnLogModel::logEntryForIndex( const QModelIndex& index ) const
{
    if( index.isValid() && index.row() >= 0 && index.row() < rowCount() )
    {
        return m_log[index.row()];
    }
    return SvnLogHolder();
}

SvnChangedPathModel::SvnChangedPathModel( QObject* parent )
    : QAbstractTableModel(parent)
{
}
QVariant SvnChangedPathModel::data( const QModelIndex& index, int role ) const
{
    if( role != Qt::DisplayRole || !index.isValid() )
        return QVariant();

    int row = index.row();
    if( row < 0 || row >= rowCount() )
        return QVariant();

    switch( index.column() )
    {
        case 0:
            return m_changedPaths[row].path;
            break;
        case 1:
            return m_changedPaths[row].action;
            break;
        case 2:
            return m_changedPaths[row].copyFromPath;
            break;
        case 3:
            return m_changedPaths[row].copyFromRevision;
            break;
        default:
            break;
    }
    return QVariant();
}
int SvnChangedPathModel::columnCount( const QModelIndex& parent ) const
{
    Q_UNUSED(parent)
    return 4;
}
int SvnChangedPathModel::rowCount( const QModelIndex& parent ) const
{
    Q_UNUSED(parent)
    return m_changedPaths.count();
}
QVariant SvnChangedPathModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if( orientation != Qt::Horizontal || role != Qt::DisplayRole )
        return QVariant();
    switch( section )
    {
        case 0:
            return i18n("Path");
            break;
        case 1:
            return i18n("Action");
            break;
        case 2:
            return i18n("From Path");
            break;
        case 3:
            return i18n("From Revision");
            break;
        default:
            break;
    }
    return QVariant();
}

void SvnChangedPathModel::setChangedPaths( const QList<SvnChangedPath>& newlist )
{
    m_changedPaths = newlist;
    reset();
}

SvnChangedPath SvnChangedPathModel::changedPathForIndex( const QModelIndex& index ) const
{
    if( index.isValid() && index.row() >= 0 && index.row() < rowCount() )
    {
        return m_changedPaths[index.row()];
    }
    return SvnChangedPath();
}


#include "svnmodels.moc"

