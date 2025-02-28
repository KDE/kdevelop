/*
    SPDX-FileCopyrightText: 2008 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "toolviewdata.h"

#include <QAbstractItemModel>
#include <QAbstractItemDelegate>


OutputData::OutputData( ToolViewData* tv )
: QObject( tv )
, delegate(nullptr)
, model(nullptr)
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
    : QObject(parent)
    , plugin(nullptr)
{
}

ToolViewData::~ToolViewData()
{
}

OutputData* ToolViewData::addOutput( int id, const QString& title,
                                     KDevelop::IOutputView::Behaviours behave )
{
    auto* d = new OutputData( this );
    d->id = id;
    d->title = title;
    d->behaviour = behave;
    outputdata.insert( id, d );
    emit outputAdded( id );
    return d;
}

#include "moc_toolviewdata.cpp"
