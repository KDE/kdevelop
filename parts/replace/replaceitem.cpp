/***************************************************************************
 *   Copyright (C) 2003 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "replaceitem.h"

bool ReplaceItem::s_listview_done = false;


bool ReplaceItem::hasCheckedChildren() const
{
    ReplaceItem const * item = firstChild();
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

    if ( s_listview_done && justClicked() )
    {
        _block = true;
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
                parent()->_clicked = true;
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
            item->_clicked = true;
            item->setOn( checked );
        }
        item = item->nextSibling();
    }
}


