/***************************************************************************
 *   Copyright (C) 2003 by Jens Dagerbo                                    *
 *   jens.dagerbo@swipnet.se                                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qstringlist.h>
#include <qlistview.h>
#include <qheader.h>

#include "closer_dialogimpl.h"


CloserDialogImpl::CloserDialogImpl( QStringList const & fileList )
    : CloserDialog( 0, 0, false, 0 )
{
    files_listview->addColumn( "" );
    files_listview->header()->hide();

    QStringList::ConstIterator it = fileList.begin();
    while ( it != fileList.end() )
    {
        QCheckListItem * x = new QCheckListItem( files_listview, *it, QCheckListItem::CheckBox );
        x->setOn( true );
        ++it;
    }
}

CloserDialogImpl::~CloserDialogImpl()
{
}

QStringList CloserDialogImpl::getCheckedFiles()
{
    QStringList checkedFiles;

    QCheckListItem const * item = static_cast<QCheckListItem*>( files_listview->firstChild() );
    while ( item )
    {
        if ( item->isOn() )
        {
            checkedFiles << item->text();
        }
        item = static_cast<QCheckListItem*>( item->nextSibling() );
    }
    return checkedFiles;
}

#include "closer_dialogimpl.moc"

