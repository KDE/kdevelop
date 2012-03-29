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

#include "makeoutputmodel.h"

#include <QFileInfo>
#include <QFont>

#include <ktexteditor/cursor.h>
#include <kurl.h>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <klocale.h>

#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>

#include "outputfilters.h"

FilteredItem::FilteredItem(const QString& line)
: originalLine( line )
, type( QVariant::fromValue( MakeOutputModel::StandardItem ) )
, shortenedText( line )
, isActivatable(false)
, lineNo(-1)
, columnNo(-1)
{
    kDebug() << "created item with type:" << type << type.value<MakeOutputModel::OutputItemType>();
}

const int MakeOutputModel::MakeItemTypeRole = Qt::UserRole + 1;

MakeOutputModel::MakeOutputModel( const KUrl& builddir, QObject* parent )
    : QAbstractListModel(parent), buildDir( builddir )
{
}

QVariant MakeOutputModel::data( const QModelIndex& idx, int role ) const
{
    if( isValidIndex(idx) )
    {
        switch( role )
        {
            case Qt::DisplayRole:
                return items.at( idx.row() ).shortenedText;
                break;
            case MakeOutputModel::MakeItemTypeRole:
                return items.at( idx.row() ).type;
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

int MakeOutputModel::rowCount( const QModelIndex& parent ) const
{
    if( !parent.isValid() )
        return items.count();
    return 0;
}

QVariant MakeOutputModel::headerData( int, Qt::Orientation, int ) const
{
    return QVariant();
}

bool MakeOutputModel::isValidIndex( const QModelIndex& idx ) const
{
    return ( idx.isValid() && idx.row() >= 0 && idx.row() < rowCount() && idx.column() == 0 );
}

void MakeOutputModel::activate( const QModelIndex& index )
{
    if( index.model() != this || !isValidIndex(index) )
    {
        kDebug(9037) << "not my model, returning";
        return;
    }
    kDebug(9037) << "Model activated" << index.row();


    FilteredItem item = items.at( index.row() );
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

QModelIndex MakeOutputModel::nextHighlightIndex( const QModelIndex &currentIdx )
{
    int startrow = isValidIndex(currentIdx) ? currentIdx.row() + 1 : 0;

    if( !errorItems.empty() )
    {
        kDebug() << "searching next error";
        // Jump to the next error item
        std::set< int >::const_iterator next = errorItems.lower_bound( startrow );
        if( next == errorItems.end() )
            next = errorItems.begin();
        
        return index( *next, 0, QModelIndex() );
    }
    
    for( int row = 0; row < rowCount(); ++row ) 
    {
        int currow = (startrow + row) % rowCount();
        if( items.at( currow ).isActivatable )
        {
            return index( currow, 0, QModelIndex() );
        }
    }
    return QModelIndex();
}

QModelIndex MakeOutputModel::previousHighlightIndex( const QModelIndex &currentIdx )
{
    //We have to ensure that startrow is >= rowCount - 1 to get a positive value from the % operation.
    int startrow = rowCount() + (isValidIndex(currentIdx) ? currentIdx.row() : rowCount()) - 1;
    
    if(!errorItems.empty())
    {
        kDebug() << "searching previous error";
        
        // Jump to the previous error item
        std::set< int >::const_iterator previous = errorItems.lower_bound( currentIdx.row() );
        
        if( previous == errorItems.begin() )
            previous = errorItems.end();
        
        --previous;
        
        return index( *previous, 0, QModelIndex() );
    }
    
    for ( int row = 0; row < rowCount(); ++row )
    {
        int currow = (startrow - row) % rowCount();
        if( items.at( currow ).isActivatable )
        {
            return index( currow, 0, QModelIndex() );
        }
    }
    return QModelIndex();
}

KUrl MakeOutputModel::urlForFile( const QString& filename ) const
{
    QFileInfo fi(filename);
    KUrl u;
    if( fi.isRelative() )
    {
        if( currentDirs.isEmpty() ) 
        {
            u = buildDir;
            u.addPath( filename );
            return u;
        }

        QLinkedList<QString>::const_iterator it = currentDirs.constEnd() - 1;
        do {
            u = KUrl( *it );
            u.addPath( filename );
        } while( (it-- !=  currentDirs.constBegin()) && !QFileInfo(u.toLocalFile()).exists() );

        return u;
    } else 
    {
        u = KUrl( filename );
    }
    return u;
}

void MakeOutputModel::addLines( const QStringList& lines )
{
    if( lines.isEmpty() )
        return;
    beginInsertRows( QModelIndex(), rowCount(), rowCount() + lines.count() - 1 );
    foreach( const QString& line, lines )
    {

        FilteredItem item( line );
        bool matched = false;
        
        OutputItemType itemType = StandardItem;
        
        foreach( const ErrorFormat& errFormat, ErrorFormat::errorFormats )
        {
            QRegExp regEx = errFormat.expression;
            if( regEx.indexIn( line ) != -1 && !( line.contains( "Each undeclared identifier is reported only once" ) || line.contains( "for each function it appears in." ) ) )
            {
                kDebug() << "found an error:" << line;
                item.url = urlForFile( regEx.cap( errFormat.fileGroup ) );
                item.lineNo = regEx.cap( errFormat.lineGroup ).toInt() - 1;
                if(errFormat.columnGroup>=0)
                    item.columnNo = regEx.cap( errFormat.columnGroup ).toInt() - 1;
                else
                    item.columnNo = 0;
                
                //item.shortenedText = regEx.cap( errFormat.textGroup );
                QString txt = regEx.cap(errFormat.textGroup);
                
                if(txt.contains("error", Qt::CaseInsensitive))
                    itemType = ErrorItem;
                
                if(txt.contains("warning", Qt::CaseInsensitive))
                    itemType = WarningItem;

                if(txt.contains("note", Qt::CaseInsensitive))
                    itemType = InformationItem;

                // Make the item clickable if it comes with the necessary file & line number information
                if (errFormat.fileGroup > 0 && errFormat.lineGroup > 0)
                    item.isActivatable = true;

                matched = true;
                break;
            }
        }
        if( !matched )
        {
            foreach( const ActionFormat& actFormat, ActionFormat::actionFormats )
            {
                QRegExp regEx = actFormat.expression;
                if( regEx.indexIn( line ) != -1 )
                {
                    kDebug() << "found an action" << line << actFormat.tool << actFormat.toolGroup << actFormat.fileGroup;
                    itemType = MakeOutputModel::ActionItem;
                    if( actFormat.fileGroup != -1 && actFormat.toolGroup != -1 )
                    {
                        item.shortenedText = QString( "%1 %2 (%3)").arg( actFormat.action ).arg( regEx.cap( actFormat.fileGroup ) ).arg( regEx.cap( actFormat.toolGroup ) );
                    }
                    if( actFormat.action == "cd" )
                    {
                        QLinkedList<QString>::iterator pos = currentDirs.insert( currentDirs.end(), regEx.cap( actFormat.fileGroup ) );
                        positionInCurrentDirs.insert( regEx.cap( actFormat.fileGroup ) , pos );
                    }

                    // Special case for cmake: we parse the "Compiling <objectfile>" expression
                    // and use it to find out about the build paths encountered during a build.
                    // They are later searched by urlForFile to find source files corresponding to
                    // compiler errors.
                    if ( actFormat.action == i18n("compiling") && actFormat.tool == "cmake")
                    {
                        KUrl url = buildDir;
                        url.addPath(regEx.cap( actFormat.fileGroup ));
                        QString dirName = url.toLocalFile();
                        // Use map to check for duplicates, to avoid O(n^2) behaviour
                        PositionMap::iterator it = positionInCurrentDirs.find(dirName);
                        // Encountered new build directory?
                        if (it == positionInCurrentDirs.end())
                        {
                            QLinkedList<QString>::iterator pos = currentDirs.insert( currentDirs.end(), dirName );
                            positionInCurrentDirs.insert( dirName, pos );
                        }
                        else
                        {
                            // Build dir already in currentDirs, but move it to back of currentDirs list
                            // (this gives us most-recently-used semantics in urlForFile)
                            currentDirs.erase(it.value());
                            QLinkedList<QString>::iterator pos = currentDirs.insert( currentDirs.end(), dirName );
                            it.value() = pos;
                        }
                    }
                    matched = true;
                    break;
                }
            }
        }
        
        item.type = QVariant::fromValue( itemType );
        
        kDebug() << "adding item:" << item.shortenedText << itemType;
        
        if( itemType == ErrorItem )
            errorItems.insert(items.size());
        
        items << item;
    }
    endInsertRows();
}

void MakeOutputModel::addLine( const QString& l )
{
    addLines( QStringList() << l );
}

#include "makeoutputmodel.moc"
