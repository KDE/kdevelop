/**********************************************************************
** Copyright (C) 2002 Trolltech AS.  All rights reserved.
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

#ifndef LISTVIEWDND_H
#define LISTVIEWDND_H

#include <qptrlist.h>
#include <qlistview.h>
#include "listdnd.h"

class QWidget;
class QListView;
typedef QPtrList<QListViewItem> ListViewItemList;

class ListViewDnd : public ListDnd
{
    Q_OBJECT
public:
    enum DragMode { Flat = 16 }; // see ListDnd::DragMode

    ListViewDnd( QListView * eventSource, const char * name = 0 );

signals:
    void dropped( QListViewItem * );

public slots:
    void confirmDrop( QListViewItem * );

protected:
    virtual bool dropEvent( QDropEvent * event );
    virtual bool mouseMoveEvent( QMouseEvent * event );
    virtual void updateLine( const QPoint & pos );
    virtual bool canDecode( QDragEnterEvent * event );
private:
    QListViewItem * itemAt( QPoint pos );
    int dropDepth( QListViewItem * item, QPoint pos );
    int buildFlatList( ListViewItemList & list );
    int buildTreeList( ListViewItemList & list );
    void setVisibleItems( bool b );
    ListViewItemList disabledItems;
};

#endif //LISTVIEWDND_H
