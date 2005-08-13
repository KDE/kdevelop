/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "actiondnd.h"
#include "actionlistview.h"
#include <qheader.h>

#include <klocale.h>

ActionListView::ActionListView( QWidget *parent, const char *name )
    : QListView( parent, name )
{
    setShowSortIndicator( TRUE );
    setResizeMode( LastColumn );
    setRootIsDecorated( TRUE );
    connect( this, SIGNAL( contextMenuRequested( QListViewItem *, const QPoint &, int ) ),
	     this, SLOT( rmbMenu( QListViewItem *, const QPoint & ) ) );
}

ActionItem::ActionItem( QListView *lv, QAction *ac )
    : QListViewItem( lv ), a( 0 ), g( 0 )
{
    g = ::qt_cast<QDesignerActionGroup*>(ac);
    if ( !g )
	a = ::qt_cast<QDesignerAction*>(ac);
    setDragEnabled( TRUE );
}

ActionItem::ActionItem( QListViewItem *i, QAction *ac )
    : QListViewItem( i ), a( 0 ), g( 0 )
{
    g = ::qt_cast<QDesignerActionGroup*>(ac);
    if ( !g )
	a = ::qt_cast<QDesignerAction*>(ac);
    setDragEnabled( TRUE );
    moveToEnd();
}

void ActionItem::moveToEnd()
{
    QListViewItem *i = this;
    while ( i->nextSibling() )
	i = i->nextSibling();
    if ( i != this )
	moveItem( i );
}

QDragObject *ActionListView::dragObject()
{
    ActionItem *i = (ActionItem*)currentItem();
    if ( !i )
	return 0;
    QStoredDrag *drag = 0;
    if ( i->action() ) {
	drag = new ActionDrag( i->action(), viewport() );
	drag->setPixmap( i->action()->iconSet().pixmap() );
    } else {
	drag = new ActionDrag( i->actionGroup(), viewport() );
	drag->setPixmap( i->actionGroup()->iconSet().pixmap() );
    }
    return drag;
}

void ActionListView::rmbMenu( QListViewItem *i, const QPoint &p )
{
    QPopupMenu *popup = new QPopupMenu( this );
    popup->insertItem( i18n( "New &Action" ), 0 );
    popup->insertItem( i18n( "New Action &Group" ), 1 );
    popup->insertItem( i18n( "New &Dropdown Action Group" ), 2 );
    if ( i ) {
	popup->insertSeparator();
	popup->insertItem( i18n( "&Connect Action..." ), 3 );
	popup->insertSeparator();
	popup->insertItem( i18n( "Delete Action" ), 4 );
    }
    int res = popup->exec( p );
    if ( res == 0 )
	emit insertAction();
    else if ( res == 1 )
	emit insertActionGroup();
    else if ( res == 2 )
	emit insertDropDownActionGroup();
    else if ( res == 3 )
	emit connectAction();
    else if ( res == 4 )
	emit deleteAction();
}
