/***************************************************************************
 *   Copyright (C) 2003 by Jens Dagerbo                                    *
 *   jens@krypton.supernet                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "replaceitem.h"

bool ReplaceItem::s_listview_done = false;


bool ReplaceItem::hasCheckedChildren()
{
    ReplaceItem * item = firstChild();
    while ( item )
    {
        if ( item->isOn() )
        {
            return true;
        }
        item = item->nextSibling();
    }
    return false;
}

void ReplaceItem::stateChange( bool state )
{
    kdDebug(0) << " ****** ReplaceItem::stateChanged() - " << state << " : " << text() << endl;

    if ( s_listview_done && _clicked )
    {
        _block.set();
        setChecked( state );
    }
}

void ReplaceItem::setChecked( bool checked )
{
    kdDebug(0) << " ****** ReplaceItem::setChecked() - " << checked << " : " << text() << endl;

    if ( !isFile() )	// this is a child item
    {
        if ( checked || !(parent()->hasCheckedChildren()))
        {
            if ( parent()->isOn() != checked )
            {
                parent()->_clicked.unset();  // stops setOn from hiding the next click
                parent()->setOn( checked );
            }
        }
        return;
    }

    // this is a parent item, set self and children
    ReplaceItem * item = firstChild();
    while ( item )
    {
        if ( item->isOn() != checked )
        {
            item->_clicked.unset();	// stops setOn from hiding the next click
            item->setOn( checked );
        }
        item = item->nextSibling();
    }
}

/**
QPtrList<ReplaceItem> ReplaceItem::getChildren( bool only_checked )
{
    QPtrList<ReplaceItem> list;

    ReplaceItem * item = firstChild();
    while ( item )
    {
        if ( only_checked )
        {
            if ( item->isOn() )
            {
                list.append( item );
            }
        }
        else
        {
            list.append( item );
        }

        item = item->nextSibling();
    }
    return list;
}
/**/

/**
virtual void paintCell( QPainter * p, const QColorGroup & cg, int column, int width, int align )
{
    if ( isFile() )
    {
        QColorGroup cg2(cg);
        cg2.setColor( QColorGroup::Text, Qt::darkGreen );
        QCheckListItem::paintCell( p, cg2, column, width, align );
    }
    else
    {
        QColorGroup cg2(cg);
        cg2.setColor( QColorGroup::Text, Qt::blue );
        QCheckListItem::paintCell( p, cg2, column, width, align );
    }
}
/**/

