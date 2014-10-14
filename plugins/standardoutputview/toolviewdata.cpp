/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 Andreas Pakulat <apaku@gmx.de>                         *
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

#include "toolviewdata.h"

#include <QAbstractItemModel>
#include <QAbstractItemDelegate>


OutputData::OutputData( ToolViewData* tv )
: QObject( tv )
, delegate(0)
, model(0)
, toolView(tv)
, id(-1)
{
}

void OutputData::setModel( QAbstractItemModel* model_ )
{
    model = model_;

    if (model) {
        model->setParent(this);
    }

    emit modelChanged( id );
}

void OutputData::setDelegate( QAbstractItemDelegate* del )
{
    delegate = del;

    if (delegate) {
        delegate->setParent(this);
    }

    emit delegateChanged( id );
}

ToolViewData::ToolViewData( QObject* parent )
    : QObject( parent ), plugin(0), toolViewId(-1)
{
}

ToolViewData::~ToolViewData()
{
}

OutputData* ToolViewData::addOutput( int id, const QString& title,
                                     KDevelop::IOutputView::Behaviours behave )
{
    OutputData* d = new OutputData( this );
    d->id = id;
    d->title = title;
    d->behaviour = behave;
    d->toolView = this;
    outputdata.insert( id, d );
    emit outputAdded( id );
    return d;
}

