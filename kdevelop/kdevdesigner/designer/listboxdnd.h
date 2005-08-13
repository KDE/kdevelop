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

#ifndef LISTBOXDND_H
#define LISTBOXDND_H

#include <qptrlist.h>
#include <qlistbox.h>
#include "listdnd.h"

typedef QPtrList<QListBoxItem> ListBoxItemList;

class ListBoxDnd : public ListDnd
{
    Q_OBJECT
public:
    // dragModes are enumerated in ListDnd
    ListBoxDnd( QListBox * eventSource, const char * name = 0 );
    
signals:
    void dropped( QListBoxItem * );
    void dragged( QListBoxItem * );

public slots:
    void confirmDrop( QListBoxItem * );

protected:
    virtual bool dropEvent( QDropEvent * event );
    virtual bool mouseMoveEvent( QMouseEvent * event );
    virtual void updateLine( const QPoint & pos );
    virtual bool canDecode( QDragEnterEvent * event );

private:
    QListBoxItem * itemAt( QPoint pos );
    int buildList( ListBoxItemList & list );
    void insertList( ListBoxItemList & list );
    void removeList( ListBoxItemList & list );
};

#endif //LISTBOXDND_H
