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
#include <qcheckbox.h>

#include <kdebug.h>

#include "closer_dialogimpl.h"


class CheckURL : public QCheckListItem
{
public:
    CheckURL( QListView * lv, KURL const & url )
        : QCheckListItem( lv, url.fileName(), QCheckListItem::CheckBox),
        _url( url )
    {}

    KURL const & url() const { return _url; }

    void showPath( bool showPaths )
    {
        if ( showPaths )
        {
            setText( 0, _url.path() );
        }
        else
        {
            setText( 0, _url.fileName() );
        }
    }

private:
    KURL _url;
};


CloserDialogImpl::CloserDialogImpl( KURL::List const & fileList )
    : CloserDialog( 0, 0, false, 0 )
{
    files_listview->addColumn( "" );
    files_listview->header()->hide();

    KURL::List::ConstIterator it = fileList.begin();
    while ( it != fileList.end() )
    {
        QCheckListItem * x = new CheckURL( files_listview, *it );
        x->setOn( true );
        ++it;
    }

    connect( this->path_check, SIGNAL( toggled( bool ) ), this, SLOT( togglePaths( bool ) ) );
}

CloserDialogImpl::~CloserDialogImpl()
{
}

void CloserDialogImpl::togglePaths( bool showPaths )
{
    QListViewItemIterator it( files_listview );
    while ( it.current() )
    {
        CheckURL * item = static_cast<CheckURL*>( it.current() );
        item->showPath( showPaths );
        ++it;
    }
}

KURL::List CloserDialogImpl::getCheckedFiles()
{
    KURL::List checkedFiles;

    CheckURL const * item = static_cast<CheckURL*>( files_listview->firstChild() );
    while ( item )
    {
        if ( item->isOn() )
        {
            checkedFiles << item->url();
        }
        item = static_cast<CheckURL*>( item->nextSibling() );
    }
    return checkedFiles;
}

#include "closer_dialogimpl.moc"

