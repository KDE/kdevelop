/***************************************************************************
 *   Copyright (C) 2007 by Dukju Ahn                                       *
 *   dukjuahn@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ioutputviewitem.h"

IOutputViewItem::IOutputViewItem( const QString& text )
    : QStandardItem( text )
{}

IOutputViewItem::~IOutputViewItem()
{}

int IOutputViewItem::type() const
{
    return OUTPUTVIEWITEM_TYPE;
}

//////////////////////////////////////////////////////////////////////

DefaultOutputItem::DefaultOutputItem( const QString& text )
    : IOutputViewItem( text )
{}

DefaultOutputItem::~DefaultOutputItem()
{}

void DefaultOutputItem::activated()
{}

QList<QAction*> DefaultOutputItem::contextMenuActions()
{
    QList<QAction*> nullist;
    return nullist;
}
