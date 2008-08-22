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
#include "outputfilters.h"
#include "makeitem.h"
#include <interfaces/icore.h>
#include <interfaces/idocumentcontroller.h>
#include <ktexteditor/cursor.h>
#include <kdebug.h>

MakeOutputModel::MakeOutputModel( QObject* parent )
    : QStandardItemModel(parent), m_actionFilter(new MakeActionFilter(*this)), m_errorFilter(new ErrorFilter(*this))
{
}

MakeActionFilter* MakeOutputModel::actionFilter()
{
    return m_actionFilter;
}

ErrorFilter* MakeOutputModel::errorFilter()
{
    return m_errorFilter;
}

void MakeOutputModel::activate( const QModelIndex& index )
{
    if( index.model() != this )
    {
        kDebug(9037) << "not my model, returning";
        return;
    }
    kDebug(9037) << "Model activated" << index.row();

    QStandardItem *stditem = itemFromIndex( index );
    MakeWarningItem *warn = dynamic_cast<MakeWarningItem*>( stditem );
    if( warn )
    {
        KTextEditor::Cursor range( warn->lineNo, 0);
        KDevelop::IDocumentController *docCtrl = KDevelop::ICore::self()->documentController();
        KUrl url;
        if ( !warn->currDir.isEmpty() ) {
            // if the warning/error file path is relative then make it consider as current
            // directory the build dir at the moment of the warning/error creation
            url = KUrl::fromPath( warn->currDir );
            url.addPath( warn->file );
        } else url = KUrl::fromPath( warn->file );
        docCtrl->openDocument( url, range );
    }
}

QModelIndex MakeOutputModel::nextHighlightIndex( const QModelIndex &currentIdx )
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
        MakeErrorItem *outItem = dynamic_cast<MakeErrorItem*>( stditem );
        if( outItem )
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

QModelIndex MakeOutputModel::previousHighlightIndex( const QModelIndex &currentIdx )
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
        MakeErrorItem *outItem = dynamic_cast<MakeErrorItem*>( stditem );
        if( outItem )
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




// void MakeOutputModel::searchNextError()
// {
// }


#include "makeoutputmodel.moc"
