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

#ifndef SIZEHANDLE_H
#define SIZEHANDLE_H

#include <qwidget.h>
#include <q3intdict.h>
#include <q3ptrdict.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QPaintEvent>

class QMouseEvent;
class FormWindow;
class WidgetSelection;
class QPaintEvent;

class SizeHandle : public QWidget
{
    Q_OBJECT

public:
    enum Direction { LeftTop, Top, RightTop, Right, RightBottom, Bottom, LeftBottom, Left };

    SizeHandle( FormWindow *parent, Qt::Orientation d, WidgetSelection *s );
    void setWidget( QWidget *w );
    void setActive( bool a );
    void updateCursor();

    void setEnabled( bool ) {}

protected:
    void paintEvent( QPaintEvent *e );
    void mousePressEvent( QMouseEvent *e );
    void mouseMoveEvent( QMouseEvent *e );
    void mouseReleaseEvent( QMouseEvent *e );

private:
    void trySetGeometry( QWidget *w, int x, int y, int width, int height );
    void tryResize( QWidget *w, int width, int height );

private:
    QWidget *widget;
    Direction dir;
    QPoint oldPressPos;
    FormWindow *formWindow;
    WidgetSelection *sel;
    QRect geom, origGeom;
    bool active;

};

class WidgetSelection
{
public:
    WidgetSelection( FormWindow *parent, Q3PtrDict<WidgetSelection> *selDict );

    void setWidget( QWidget *w, bool updateDict = TRUE );
    bool isUsed() const;

    void updateGeometry();
    void hide();
    void show();
    void update();
    
    QWidget *widget() const;

protected:
    Q3IntDict<SizeHandle> handles;
    QWidget *wid;
    FormWindow *formWindow;
    Q3PtrDict<WidgetSelection> *selectionDict;

};

#endif
