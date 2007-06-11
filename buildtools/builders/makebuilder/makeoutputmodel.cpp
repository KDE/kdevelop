/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>                     *
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
#include "makebuilder.h"
#include "icore.h"
#include "idocumentcontroller.h"
#include <ktexteditor/cursor.h>
#include <kdebug.h>

MakeOutputModel::MakeOutputModel( MakeBuilder *builder, QObject* parent )
    : QStandardItemModel(parent), actionFilter(new MakeActionFilter), errorFilter(new ErrorFilter)
    , m_builder( builder )
{
}

void MakeOutputModel::addStandardError( const QStringList& lines )
{
    foreach( QString line, lines)
    {
        QStandardItem* item = errorFilter->processAndCreate(line);
        if( !item )
            item = new QStandardItem(line);
        appendRow(item);
    }
}

void MakeOutputModel::addStandardOutput( const QStringList& lines )
{
    foreach( QString line, lines)
    {
        QStandardItem* item = actionFilter->processAndCreate(line);
        if( !item )
            item = new QStandardItem(line);
        appendRow(item);
    }
}

void MakeOutputModel::activated( const QModelIndex & index )
{
    if( index.model() != this ){
        kDebug(9038) << "not my model, returning" << endl;
        return;
    }
    kDebug(9038) << "Model activated " << index.row() << endl;

    QStandardItem *stditem = itemFromIndex( index );
    MakeWarningItem *warn = dynamic_cast<MakeWarningItem*>( stditem );
    if( warn )
    {
        KTextEditor::Cursor range( warn->lineNo, 0);
        KDevelop::IDocumentController *docCtrl = m_builder->core()->documentController();
        docCtrl->openDocument( warn->file, range );
    }
}

// void MakeOutputModel::searchNextError()
// {
// }

//kate: space-indent on; indent-width 4; replace-tabs on; auto-insert-doxygen on; indent-mode cstyle;

#include "makeoutputmodel.moc"
