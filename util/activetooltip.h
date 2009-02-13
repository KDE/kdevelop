/* This file is part of the KDE project
   Copyright 2007 Vladimir Prus

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#ifndef ACTIVE_TOOLTIP_H
#define ACTIVE_TOOLTIP_H

#include <QtGui/QWidget>
#include "utilexport.h"

namespace KDevelop {

/** This class implements a tooltip that can contain arbitrary
    widgets that the user can interact with.

    There is a two-step activation scheme for the widget.  When
    created, it records the parent widget and the position, and
    arranges for the object to be closed and deleted if mouse
    leaves the region around the original position, or if we click
    in different widget.

    The widget is not originally shown, so we can fetch whatever data
    is necessary to be displayed.  When we are ready to show the
    data, the 'show()' method must be called.  It will show the widget
    and extend the area we can interact with to cover the widget.  */
class KDEVPLATFORMUTIL_EXPORT ActiveToolTip : public QWidget
{
Q_OBJECT
public:
    /* position must be in global coordinates.  */
    ActiveToolTip(QWidget *parent, const QPoint& position);
    ~ActiveToolTip();


    bool eventFilter(QObject *object, QEvent *e);
    
    bool insideThis(QObject* object);

    void showEvent(QShowEvent*);

    void resizeEvent(QResizeEvent*);

    void adjustRect();

private:
    class ActiveToolTipPrivate* const d;
};

}

#endif
