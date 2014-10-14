/* This file is part of the KDE project
   Copyright 2007 Vladimir Prus
   Copyright 2009-2010 David Nolden

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
#include "debug.h"

#include <QPoint>
#include <QPalette>
#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QPointer>
#include <limits>
#include <qdesktopwidget.h>
#include <qmenu.h>
#include <qstylepainter.h>
#include <qstyleoption.h>

namespace KDevelop
{

class ActiveToolTipPrivate
{
public:
    uint previousDistance_;
    QRect rect_;
    QRegion rectExtensions_;
    QList<QPointer<QObject> > friendWidgets_;
};

ActiveToolTip::ActiveToolTip(QWidget *parent, const QPoint& position)
    : QWidget(parent, Qt::ToolTip), d(new ActiveToolTipPrivate)
{
    Q_ASSERT(parent);
    d->previousDistance_ = std::numeric_limits<uint>::max();
    setMouseTracking(true);
    d->rect_ = QRect(position, position);
    d->rect_.adjust(-10, -10, 10, 10);
    move(position);

    QPalette p;

    // adjust background color to use tooltip colors
    p.setColor(backgroundRole(), p.color(QPalette::ToolTipBase));
    p.setColor(QPalette::Base, p.color(QPalette::ToolTipBase));

    // adjust foreground color to use tooltip colors
    p.setColor(foregroundRole(), p.color(QPalette::ToolTipText));
    p.setColor(QPalette::Text, p.color(QPalette::ToolTipText));
    setPalette(p);

    setWindowFlags(Qt::WindowDoesNotAcceptFocus | windowFlags());

    qApp->installEventFilter(this);
}

ActiveToolTip::~ActiveToolTip()
{
    delete d;
}

bool ActiveToolTip::eventFilter(QObject *object, QEvent *e)
{
    switch (e->type()) {
    case QEvent::MouseMove:
        if (underMouse() || insideThis(object)) {
            return false;
        } else {
            QPoint globalPos = static_cast<QMouseEvent*>(e)->globalPos();
            int distance = (d->rect_.center() - globalPos).manhattanLength();

            if(distance > (int)d->previousDistance_) {
                // Close if the widget under the mouse is not a child widget of the tool-tip
                qCDebug(UTIL) << "closing because of mouse move outside the widget";
                close();
            } else {
                d->previousDistance_ = distance;
            }
        }
        break;

    case QEvent::WindowActivate:
        if (insideThis(object)) {
            return false;
        }
        close();
        break;

    case QEvent::WindowBlocked:
        // Modal dialog activated somewhere, it is the only case where a cursor
        // move may be missed and the popup has to be force-closed
        close();
        break;

    default:
        break;
    }
    return false;
}

void ActiveToolTip::addFriendWidget(QWidget* widget)
{
    d->friendWidgets_.append((QObject*)widget);
}

bool ActiveToolTip::insideThis(QObject* object)
{
    while (object)
    {
        if(dynamic_cast<QMenu*>(object))
            return true;

        if (object == this || object == (QObject*)this->windowHandle() || d->friendWidgets_.contains(object))
        {
            return true;
        }
        object = object->parent();
    }

    // If the object clicked is inside a QQuickWidget, its parent is null even
    // if it is part of a tool-tip. This check ensures that a tool-tip is never
    // closed while the mouse is in it
    return underMouse();
}

void ActiveToolTip::showEvent(QShowEvent*)
{
    adjustRect();
}

void ActiveToolTip::updateMouseDistance()
{
    d->previousDistance_ = (d->rect_.center() - QCursor::pos()).manhattanLength();
}

void ActiveToolTip::moveEvent(QMoveEvent* ev)
{
    QWidget::moveEvent(ev);

    updateMouseDistance();
}

void ActiveToolTip::resizeEvent(QResizeEvent*)
{
    adjustRect();

    // set mask from style
    QStyleOptionFrame opt;
    opt.init(this);

    QStyleHintReturnMask mask;
    if( style()->styleHint( QStyle::SH_ToolTip_Mask, &opt, this, &mask ) && !mask.region.isEmpty() )
    { setMask( mask.region ); }

    emit resized();

    updateMouseDistance();
}

void ActiveToolTip::paintEvent(QPaintEvent* event)
{
    QStylePainter painter( this );
    painter.setClipRegion( event->region() );
    QStyleOptionFrame opt;
    opt.init(this);
    painter.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
}

void ActiveToolTip::addExtendRect(const QRect& rect)
{
    d->rectExtensions_ += rect;
}

void ActiveToolTip::adjustRect()
{
    // For tooltip widget, geometry() returns global coordinates.
    QRect r = geometry();
    r.adjust(-10, -10, 10, 10);
    d->rect_ = r;
    updateMouseDistance();
}

void ActiveToolTip::setBoundingGeometry(const QRect& geometry) {
    d->rect_ = geometry;
    d->rect_.adjust(-10, -10, 10, 10);
}

namespace {
    typedef QMultiMap<float, QPair<QPointer<ActiveToolTip>, QString> > ToolTipPriorityMap;
    static ToolTipPriorityMap registeredToolTips;
    ActiveToolTipManager manager;

    QWidget* masterWidget(QWidget* w) {
    while(w && w->parent() && qobject_cast<QWidget*>(w->parent()))
        w = qobject_cast<QWidget*>(w->parent());
    return w;
    }
}

void ActiveToolTipManager::doVisibility() {
    bool exclusive = false;
    int lastBottomPosition = -1;
    int lastLeftPosition = -1;
    QRect fullGeometry; //Geometry of all visible tooltips together

    for(ToolTipPriorityMap::const_iterator it = registeredToolTips.constBegin(); it != registeredToolTips.constEnd(); ++it) {
        QPointer< ActiveToolTip > w = (*it).first;
        if(w) {
            if(exclusive) {
                (w.data())->hide();
            }else{
                QRect geom = (w.data())->geometry();
                if((w.data())->geometry().top() < lastBottomPosition) {
                    geom.moveTop(lastBottomPosition);
                }
                if(lastLeftPosition != -1)
                    geom.moveLeft(lastLeftPosition);

                (w.data())->setGeometry(geom);
//                 (w.data())->show();

                lastBottomPosition = (w.data())->geometry().bottom();
                lastLeftPosition = (w.data())->geometry().left();

                if(it == registeredToolTips.constBegin())
                    fullGeometry = (w.data())->geometry();
                else
                    fullGeometry = fullGeometry.united((w.data())->geometry());
            }
            if(it.key() == 0) {
                exclusive = true;
            }
        }
    }
    if(!fullGeometry.isEmpty()) {
        QRect oldFullGeometry = fullGeometry;
        QRect screenGeometry = QApplication::desktop()->screenGeometry(fullGeometry.topLeft());
        if(fullGeometry.bottom() > screenGeometry.bottom()) {
            //Move up, avoiding the mouse-cursor
            fullGeometry.moveBottom(fullGeometry.top()-10);
            if(fullGeometry.adjusted(-20, -20, 20, 20).contains(QCursor::pos()))
                fullGeometry.moveBottom(QCursor::pos().y() - 20);
        }
        if(fullGeometry.right() > screenGeometry.right()) {
            //Move to left, avoiding the mouse-cursor
            fullGeometry.moveRight(fullGeometry.left()-10);
            if(fullGeometry.adjusted(-20, -20, 20, 20).contains(QCursor::pos()))
                fullGeometry.moveRight(QCursor::pos().x() - 20);
        }
        // Now fit this to screen
        if (fullGeometry.left() < 0) {
            fullGeometry.setLeft(0);
        }
        if (fullGeometry.top() < 0) {
            fullGeometry.setTop(0);
        }

        QPoint offset = fullGeometry.topLeft() - oldFullGeometry.topLeft();
        if(!offset.isNull()) {
            for(ToolTipPriorityMap::const_iterator it = registeredToolTips.constBegin(); it != registeredToolTips.constEnd(); ++it)
                if((*it).first) {
                    (*it).first.data()->move((*it).first.data()->pos() + offset);
                }
        }
    }

    //Set bounding geometry, and remove old tooltips
    for(ToolTipPriorityMap::iterator it = registeredToolTips.begin(); it != registeredToolTips.end(); ) {
        if(!(*it).first) {
            it = registeredToolTips.erase(it);
        }else{
            (*it).first.data()->setBoundingGeometry(fullGeometry);
            ++it;
        }
    }

    //Final step: Show tooltips
    for(ToolTipPriorityMap::const_iterator it = registeredToolTips.constBegin(); it != registeredToolTips.constEnd(); ++it) {
        if(it->first.data() && masterWidget(it->first.data())->isActiveWindow())
            (*it).first.data()->show();
        if(exclusive)
            break;
    }
}

void ActiveToolTip::showToolTip(KDevelop::ActiveToolTip* tooltip, float priority, QString uniqueId) {

    if(!uniqueId.isEmpty()) {
        for(QMap< float, QPair< QPointer< ActiveToolTip >, QString > >::const_iterator it = registeredToolTips.constBegin(); it != registeredToolTips.constEnd(); ++it) {
            if((*it).second == uniqueId)
                delete (*it).first.data();
        }
    }

    registeredToolTips.insert(priority, qMakePair(QPointer<KDevelop::ActiveToolTip>(tooltip), uniqueId));

    connect(tooltip, SIGNAL(resized()), &manager, SLOT(doVisibility()));
    QMetaObject::invokeMethod(&manager, "doVisibility", Qt::QueuedConnection);
}


void ActiveToolTip::closeEvent(QCloseEvent* event)
{
    QWidget::closeEvent(event);
    deleteLater();
}

}



