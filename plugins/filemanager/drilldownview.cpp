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
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/
#include "drilldownview.h"

#include <kdebug.h>

class DrillDownItemDelegate: public QItemDelegate {
public:
    DrillDownItemDelegate(DrillDownView *parent)
        :QItemDelegate(parent), m_parent(parent)
    {
    }


    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QModelIndex &index) const
    {
        QItemDelegate::paint(painter, option, index);

        if (m_parent->model()->hasChildren(index))
        {
            QStyleOptionViewItem opt(option);
            opt.rect.setLeft(opt.rect.width() - 16);
            m_parent->style()->drawPrimitive(QStyle::PE_IndicatorArrowRight, &opt, painter);
        }
    }

private:
    DrillDownView *m_parent;

};



DrillDownView::DrillDownView(QWidget *parent)
    :QListView(parent)
{
    connect(&animation, SIGNAL(frameChanged(int)), this, SLOT(slide(int)));
    connect(&animation, SIGNAL(finished()), this, SLOT(update()));
    animation.setDuration(200);

    setItemDelegate(new DrillDownItemDelegate(this));
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

void DrillDownView::keyPressEvent(QKeyEvent *event)
{
    QModelIndex current = currentIndex();
    if (current.isValid())
    {
        Qt::Key moveDirection = Qt::Key_Any;
        if ((event->key() == Qt::Key_Right) && (model()->hasChildren(current)))
            moveDirection = Qt::Key_Right;
        else if (event->key() == Qt::Key_Left)
            moveDirection = Qt::Key_Left;

        if (moveDirection != Qt::Key_Any)
        {
            //eat event if animation is running
            if (animation.state() == QTimeLine::Running)
                return;

            bool animate = true;
            setUpdatesEnabled(false);
            if (moveDirection == Qt::Key_Right)
            {
                setRootIndex(current);
                if (model()->canFetchMore(current))
                {
                    animate = false;
                    model()->fetchMore(current);
                }
            }
            else
            {
                QModelIndex root = rootIndex();
                if (root.isValid())
                    setRootIndex(root.parent());
            }
            if (animate)
            {
                executeDelayedItemsLayout();
                // Force the hiding/showing of scrollbars
                setVerticalScrollBarPolicy(verticalScrollBarPolicy());
                newView = QPixmap::grabWidget(viewport());
                setUpdatesEnabled(true);
                int length = qMax(oldView.width(), newView.width());
                lastPosition = moveDirection == Qt::Key_Left ? length : 0;
                animation.setFrameRange(0, length);
                animation.stop();
                animation.setDirection(moveDirection == Qt::Key_Right ?
                    QTimeLine::Forward : QTimeLine::Backward);
                animation.start();
            }
            setUpdatesEnabled(true);
        }
    }
    if (event->key() == Qt::Key_Return)
        emit returnPressed(currentIndex());
    QListView::keyPressEvent(event);
}

#include "drilldownview.moc"
