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

#ifndef LISTDND_H
#define LISTDND_H

#include <qobject.h>
#include <qscrollview.h>

class ListDnd : public QObject
{
    Q_OBJECT
public:
    enum DragMode { None = 0, External = 1, Internal = 2, Both = 3, Move = 4, NullDrop = 8 };
    ListDnd( QScrollView * eventSource, const char * name = 0 );
    void setDragMode( int mode );
    int dragMode() const;
    bool eventFilter( QObject *, QEvent * event );

protected:
    virtual bool dragEnterEvent( QDragEnterEvent * event );
    virtual bool dragLeaveEvent( QDragLeaveEvent * );
    virtual bool dragMoveEvent( QDragMoveEvent * event );
    virtual bool dropEvent( QDropEvent * event );
    virtual bool mousePressEvent( QMouseEvent * event );
    virtual bool mouseMoveEvent( QMouseEvent * event );
    virtual void updateLine( const QPoint & dragPos );
    virtual bool canDecode( QDragEnterEvent * event );

    QScrollView * src;
    QWidget * line;
    QPoint mousePressPos;
    QPoint dragPos;
    bool dragInside;
    bool dragDelete;
    bool dropConfirmed;
    int dMode;
};

#endif // LISTDND_H
