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

#include <QWidget>
#include <QPoint>
#include <QPalette>
#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include "utilexport.h"
#include <limits>
#include <kdebug.h>

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
public:
    /* position must be in global coordinates.  */
    ActiveToolTip(QWidget *parent, const QPoint& position)
    : QWidget(parent, Qt::ToolTip), mouseOut_(0)
    {
        previousDistance_ = std::numeric_limits<uint>::max();
        setAttribute(Qt::WA_DeleteOnClose);
        setMouseTracking(true);
        rect_ = QRect(position, position);
        rect_.adjust(-10, -10, 10, 10);
        move(position);

        QPalette p;
        p.setColor(backgroundRole(), p.color(QPalette::ToolTipBase));
        p.setColor(QPalette::Base, p.color(QPalette::ToolTipBase));
        setPalette(p);

        qApp->installEventFilter(this);
    }

    bool eventFilter(QObject *object, QEvent *e)
    {
        switch (e->type()) {

        case QEvent::WindowActivate:
        case QEvent::WindowDeactivate:
            close();
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::Wheel:
            /* If the click is within tooltip, it's fine.
               Clicks outside close it.  */
            if (!insideThis(object))
                close();

        // FIXME: revisit this code later.
#if 0
        case QEvent::FocusIn:
        case QEvent::FocusOut:
            close();
            break;
#endif
        case QEvent::MouseMove:
            if (!rect_.isNull() 
                && !rect_.contains(static_cast<QMouseEvent*>(e)->globalPos())) {
                
                int distance = (rect_.center() - static_cast<QMouseEvent*>(e)->globalPos()).manhattanLength();
                
                // On X, when the cursor leaves the tooltip and enters
                // the parent, we sometimes get some wrong Y coordinate.
                // Don't know why, so wait for two out-of-range mouse
                // positions before closing.
                
                //Additional test: When the cursor has been moved towards the tooltip, don't close it.
                if(distance > previousDistance_)
                    ++mouseOut_;
                else
                    previousDistance_ = distance;
            } else               
                mouseOut_ = 0;
            if (mouseOut_ == 2) {
                close();
            }
        default:
            break;
        }
        return false;
    }

    bool insideThis(QObject* object)
    {
        while (object)
        {
            if (object == this)
            {
                return true;
            }
            object = object->parent();
        }
        return false;
    }

    void showEvent(QShowEvent*)
    {        
        adjustRect();
    }

    void resizeEvent(QResizeEvent*)
    {
        adjustRect();
    }

    void adjustRect()
    {
        // For tooltip widget, geometry() returns global coordinates.
        QRect r = geometry();
        r.adjust(-10, -10, 10, 10);
        rect_ = r;
    }

private:
    uint previousDistance_;
    QRect rect_;
    int mouseOut_;
};

}

#endif
