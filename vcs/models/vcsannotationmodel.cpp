/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
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

#include "vcsannotationmodel.h"

#include <QDateTime>
#include <QtGlobal>

#include <kurl.h>
#include <klocale.h>
#include <kdebug.h>

#include "vcsrevision.h"

VcsAnnotationModel::VcsAnnotationModel( const KUrl& url )
{
    m_annotation.setLocation( url );
    qsrand( QDateTime().toTime_t() );
}
VcsAnnotationModel::~VcsAnnotationModel()
{
}

int VcsAnnotationModel::rowCount( const QModelIndex& ) const
{
    return m_annotation.lineCount();
}

int VcsAnnotationModel::columnCount( const QModelIndex& ) const
{
    return 3;
}

QVariant VcsAnnotationModel::data( const QModelIndex& idx, int role ) const
{
    if( !idx.isValid() ||
         ( role != Qt::DisplayRole && role != Qt::ToolTipRole && role != Qt::BackgroundRole ) )
        return QVariant();

    if( idx.row() < 0 || idx.row() >= rowCount() || idx.column() < 0 || idx.column() >= columnCount() )
        return QVariant();

    KDevelop::VcsAnnotationLine line = m_annotation.line( idx.row() );
    if( role == Qt::BackgroundRole )
    {
        return QVariant( m_brushes[line.revision()] );
    }else
    {
        switch( idx.column() )
        {
            case 0:
                if( role == Qt::DisplayRole )
                    return QVariant( line.lineNumber() );
                break;
            case 1:
                if( role == Qt::ToolTipRole )
                {
                    return QVariant( i18n("Author:%1\nDate:%2", line.author(), line.date().toString() ) );
                }else
                {
                    return QVariant( line.revision().revisionValue() );
                }
                break;
            case 2:
                if( role == Qt::ToolTipRole )
                {
                    return QVariant( i18n("Author:%1\nDate:%2", line.author(), line.date().toString() ) );
                }else
                {
                    return QVariant( line.text() );
                }
                break;
            default:
                break;
        }
    }
    return QVariant();
}

QVariant VcsAnnotationModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
    if( section < 0 || section >= columnCount() || orientation != Qt::Horizontal || role != Qt::DisplayRole )
        return QVariant();
    switch( section )
    {
        case 0:
            return QVariant( i18nc("number of a line in a file", "Line") );
        case 1:
            return QVariant( i18n("Revision") );
            break;
        case 2:
            return QVariant( i18nc("content of a line in a file", "Text") );
            break;
        default:
            break;
    }
    return QVariant();
}

void VcsAnnotationModel::addLines( const QList<KDevelop::VcsAnnotationLine>& list )
{
    if( list.isEmpty() )
        return;
    if( rowCount() > 0 )
        beginInsertRows( QModelIndex(), rowCount(), rowCount()+list.count()-1 );
    else
        beginInsertRows( QModelIndex(), rowCount(), list.count() );
    foreach( KDevelop::VcsAnnotationLine l, list )
    {
        if( !m_brushes.contains( l.revision() ) )
        {
            int r = ( float(qrand()) / RAND_MAX ) * 255;
            int g = ( float(qrand()) / RAND_MAX ) * 255;
            int b = ( float(qrand()) / RAND_MAX ) * 255;
            m_brushes.insert( l.revision(), QBrush( QColor( r, g, b, 80 ) ) );
        }
        m_annotation.insertLine( l.lineNumber(), l );
    }
    endInsertRows();
}

KDevelop::VcsAnnotation VcsAnnotationModel::annotation() const
{
    return m_annotation;
}

#include "vcsannotationmodel.moc"
