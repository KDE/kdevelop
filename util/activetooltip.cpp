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

#include "activetooltip.h"

#include <QPoint>
#include <QPalette>
#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <limits>
#include <kdebug.h>

namespace KDevelop
{

class ActiveToolTipPrivate
{
public:
    uint previousDistance_;
    QRect rect_;
    int mouseOut_;
};

ActiveToolTip::ActiveToolTip(QWidget *parent, const QPoint& position)
    : QWidget(parent, Qt::ToolTip), d(new ActiveToolTipPrivate)
{
    d->mouseOut_ = 0;
    d->previousDistance_ = std::numeric_limits<uint>::max();
    setAttribute(Qt::WA_DeleteOnClose);
    setMouseTracking(true);
    d->rect_ = QRect(position, position);
    d->rect_.adjust(-10, -10, 10, 10);
    move(position);

    QPalette p;
    p.setColor(backgroundRole(), p.color(QPalette::ToolTipBase));
    p.setColor(QPalette::Base, p.color(QPalette::ToolTipBase));
    setPalette(p);

    qApp->installEventFilter(this);
}

ActiveToolTip::~ActiveToolTip()
{
    delete d;
}

bool ActiveToolTip::eventFilter(QObject *object, QEvent *e)
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
        if (!d->rect_.isNull() 
            && !d->rect_.contains(static_cast<QMouseEvent*>(e)->globalPos())) {
            
            int distance = (d->rect_.center() - static_cast<QMouseEvent*>(e)->globalPos()).manhattanLength();
            
            // On X, when the cursor leaves the tooltip and enters
            // the parent, we sometimes get some wrong Y coordinate.
            // Don't know why, so wait for two out-of-range mouse
            // positions before closing.
            
            //Additional test: When the cursor has been moved towards the tooltip, don't close it.
            if(distance > d->previousDistance_)
                ++d->mouseOut_;
            else
                d->previousDistance_ = distance;
        } else               
            d->mouseOut_ = 0;
        if (d->mouseOut_ == 2) {
            close();
        }
    default:
        break;
    }
    return false;
}

bool ActiveToolTip::insideThis(QObject* object)
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

void ActiveToolTip::showEvent(QShowEvent*)
{        
    adjustRect();
}

void ActiveToolTip::resizeEvent(QResizeEvent*)
{
    adjustRect();
}

void ActiveToolTip::adjustRect()
{
    // For tooltip widget, geometry() returns global coordinates.
    QRect r = geometry();
    r.adjust(-10, -10, 10, 10);
    d->rect_ = r;
}

}

#include "activetooltip.moc"
