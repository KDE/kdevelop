/***************************************************************************
 *   Copyright (C) 2005 by Benjamin Meyer                                  *
 *   ben@meyerhome.net                                                     *
 *   Copyright (C) 2006 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "drilldownview.h"

#include <kdebug.h>

DrillDownView::DrillDownView(QWidget *parent)
    :QListView(parent)
{
    connect(&animation, SIGNAL(frameChanged(int)), this, SLOT(slide(int)));
    animation.setDuration(200);
}

void DrillDownView::paintEvent(QPaintEvent *event)
{
    if (animation.state() == QTimeLine::Running)
    {
        QPainter painter(viewport());
        if (animation.direction() == QTimeLine::Backward)
        {
            painter.drawPixmap(-animation.currentFrame(), 0, newView);
            painter.drawPixmap(-animation.currentFrame() + animation.endFrame(), 0, oldView);
        }
        else
        {
            painter.drawPixmap(-animation.currentFrame(), 0, oldView);
            painter.drawPixmap(-animation.currentFrame() + animation.endFrame(), 0, newView);
        }
    }
    else
    {
        QListView::paintEvent(event);
    }
}

void DrillDownView::slide(int x)
{
    viewport()->scroll(lastPosition - x, 0);
    lastPosition = x;
}
/*
QModelIndex DrillDownView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    kDebug() << k_funcinfo << endl;
    if (animation.state() == QTimeLine::Running)
        return QListView::moveCursor(cursorAction, modifiers);
    QModelIndex current = currentIndex();
    if (cursorAction == MoveLeft && current.parent().isValid())
    {
        oldView = QPixmap::grabWidget(viewport());
        return current.parent();
    }
    if (cursorAction == MoveRight && model() && model()->hasChildren(current))
    {
        kDebug() << "moving right" << endl;
        oldView = QPixmap::grabWidget(viewport());
//         model()->fetchMore(current);
        return model()->index(0, 0, current);
    }
    return QListView::moveCursor(cursorAction, modifiers);
}

void DrillDownView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    kDebug() << k_funcinfo << endl;
    if ((current.isValid() && previous.isValid())
        && (current.parent() == previous || previous.parent() == current))
    {
        setUpdatesEnabled(false);
        setRootIndex(currentIndex().parent());
        setCurrentIndex(currentIndex());
        executeDelayedItemsLayout();
        // Force the hiding/showing of scrollbars
        setVerticalScrollBarPolicy(verticalScrollBarPolicy());
        newView = QPixmap::grabWidget(viewport());
        setUpdatesEnabled(true);
        int length = qMax(oldView.width(), newView.width());
        lastPosition = (previous.parent() == current) ? length : 0;
        animation.setFrameRange(0, length);
        animation.stop();
        animation.setDirection(previous.parent() == current ? QTimeLine::Backward : QTimeLine::Forward);
        animation.start();
    }
    else
    {
        QListView::currentChanged(current, previous);
    }
}
*/
void DrillDownView::keyPressEvent(QKeyEvent *event)
{
    QModelIndex current = currentIndex();
    if (current.isValid())
    {
        if ((event->key() == Qt::Key_Right) && (model()->hasChildren(current)))
        {
            setUpdatesEnabled(false);
            setRootIndex(current);
            if (model()->canFetchMore(current))
                model()->fetchMore(current);
            setUpdatesEnabled(true);
        }
        else if (event->key() == Qt::Key_Left)
        {
            setUpdatesEnabled(false);
            QModelIndex root = rootIndex();
            if (root.isValid())
                setRootIndex(root.parent());
            setUpdatesEnabled(true);
        }
    }
    QListView::keyPressEvent(event);
}

#include "drilldownview.moc"
