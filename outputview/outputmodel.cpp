/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2007 Andreas Pakulat <apaku@gmx.de>                         *
 *   Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>                  *
 *   Copyright (C) 2012  Morten Danielsen Volden mvolden2@gmail.com        *
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

#include "outputmodel.h"
#include "filtereditem.h"
#include "outputfilteringstrategies.h"

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>

#include <QtCore/QStringList>
#include <QtCore/QTimer>

#include <KDebug>
#include <kglobalsettings.h>

#include <QFont>

namespace KDevelop
{
    
const int OutputModel::OutputItemTypeRole = Qt::UserRole + 1;

OutputModel::OutputModel( const KUrl& builddir, QObject* parent )
    : QAbstractListModel(parent)
    , m_filter( new NoFilterStrategy )
    , m_buildDir( builddir )
{
}

OutputModel::OutputModel( QObject* parent )
    : QAbstractListModel(parent)
    , m_filter( new NoFilterStrategy )
{
}

QVariant OutputModel::data(const QModelIndex& idx , int role ) const
{
    if( isValidIndex(idx) )
    {
        switch( role )
        {
            case Qt::DisplayRole:
                return m_filteredItems.at( idx.row() ).shortenedText;
                break;
            case OutputModel::OutputItemTypeRole:
                return m_filteredItems.at( idx.row() ).type;
                break;
            case Qt::FontRole:
                return KGlobalSettings::fixedFont();
                break;
            default:
                break;
        }
    }
    return QVariant();
}

int OutputModel::rowCount( const QModelIndex& parent ) const
{
    if( !parent.isValid() )
        return m_filteredItems.count();
    return 0;
}

QVariant OutputModel::headerData( int, Qt::Orientation, int ) const
{
    return QVariant();
}

bool OutputModel::isValidIndex( const QModelIndex& idx ) const
{
    return ( idx.isValid() && idx.row() >= 0 && idx.row() < rowCount() && idx.column() == 0 );
}

void OutputModel::activate( const QModelIndex& index )
{
    if( index.model() != this || !isValidIndex(index) )
    {
        kDebug() << "not my model, returning";
        return;
    }
    kDebug() << "Model activated" << index.row();


    FilteredItem item = m_filteredItems.at( index.row() );
    if( item.isActivatable )
    {
        kDebug() << "activating:" << item.lineNo << item.url;
        KTextEditor::Cursor range( item.lineNo, item.columnNo );
        KDevelop::IDocumentController *docCtrl = KDevelop::ICore::self()->documentController();
        docCtrl->openDocument( item.url, range );
    } else 
    {
        kDebug() << "not an activateable item";
    }
}

QModelIndex OutputModel::nextHighlightIndex( const QModelIndex &currentIdx )
{
    int startrow = isValidIndex(currentIdx) ? currentIdx.row() + 1 : 0;

    if( !m_activateableItems.empty() )
    {
        kDebug() << "searching next error";
        // Jump to the next error item
        std::set< int >::const_iterator next = m_activateableItems.lower_bound( startrow );
        if( next == m_activateableItems.end() )
            next = m_activateableItems.begin();
        
        return index( *next, 0, QModelIndex() );
    }
    
    for( int row = 0; row < rowCount(); ++row ) 
    {
        int currow = (startrow + row) % rowCount();
        if( m_filteredItems.at( currow ).isActivatable )
        {
            return index( currow, 0, QModelIndex() );
        }
    }
    return QModelIndex();
}

QModelIndex OutputModel::previousHighlightIndex( const QModelIndex &currentIdx )
{
    //We have to ensure that startrow is >= rowCount - 1 to get a positive value from the % operation.
    int startrow = rowCount() + (isValidIndex(currentIdx) ? currentIdx.row() : rowCount()) - 1;
    
    if(!m_activateableItems.empty())
    {
        kDebug() << "searching previous error";
        
        // Jump to the previous error item
        std::set< int >::const_iterator previous = m_activateableItems.lower_bound( currentIdx.row() );
        
        if( previous == m_activateableItems.begin() )
            previous = m_activateableItems.end();
        
        --previous;
        
        return index( *previous, 0, QModelIndex() );
    }
    
    for ( int row = 0; row < rowCount(); ++row )
    {
        int currow = (startrow - row) % rowCount();
        if( m_filteredItems.at( currow ).isActivatable )
        {
            return index( currow, 0, QModelIndex() );
        }
    }
    return QModelIndex();
}

void OutputModel::setFilteringStrategy(const OutputFilterStrategy& currentStrategy)
{
    switch( currentStrategy )
    {
        case NoFilter:
            m_filter = QSharedPointer<IFilterStrategy>( new NoFilterStrategy );
            break;
        case CompilerFilter:
            m_filter = QSharedPointer<IFilterStrategy>( new CompilerFilterStrategy );
            break;
        case ScriptErrorFilter:
            m_filter = QSharedPointer<IFilterStrategy>( new ScriptErrorFilterStrategy );
            break;
        default:
            // assert(false);
            m_filter = QSharedPointer<IFilterStrategy>( new NoFilterStrategy );
            break;
    }
}

void OutputModel::appendLines( const QStringList& lines )
{
    if( lines.isEmpty() )
        return;
    
    m_lineBuffer << lines;
    QMetaObject::invokeMethod(this, "addLineBatch", Qt::QueuedConnection);
}

void OutputModel::appendLine( const QString& l )
{
    appendLines( QStringList() << l );
}


void OutputModel::addLineBatch()
{
     // only add this many lines in one batch, then return to the event loop
    // this prevents overly long UI lockup and is simple enough to implement
    const int maxLines = 50;
    const int linesInBatch = qMin(m_lineBuffer.count(), maxLines);

    // If there is nothing to insert we are done.
    if ( linesInBatch == 0 )
            return;
    
    beginInsertRows( QModelIndex(), rowCount(), rowCount() + linesInBatch -  1);

    for(int i = 0; i < linesInBatch; ++i) {
        const QString line = m_lineBuffer.dequeue();
        FilteredItem item( line );
        
        bool matched = m_filter->isErrorInLine(line, item);
        if( !matched )
        {
            matched = m_filter->isActionInLine(line, item);
        }
        
        //kDebug() << "adding item:" << item.shortenedText << itemType;
        if( item.type == QVariant::fromValue( FilteredItem::ErrorItem) )
            m_activateableItems.insert(m_activateableItems.size());

        m_filteredItems << item;
    }
    
    endInsertRows();
    
    if (!m_lineBuffer.isEmpty()) {
        QMetaObject::invokeMethod(this, "addLineBatch", Qt::QueuedConnection);
    }
}

}

#include "outputmodel.moc"
