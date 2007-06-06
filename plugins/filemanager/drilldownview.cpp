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
        painter->save();
        QItemDelegate::paint(painter, option, index);

        if (m_parent->model()->hasChildren(index))
        {
            QStyleOptionViewItem opt(option);
            opt.rect = m_parent->normalizeDrillIconRect(m_parent->visualRect(index));
            m_parent->style()->drawPrimitive(QStyle::PE_IndicatorArrowRight, &opt, painter);
        }
        painter->restore();
    }

private:
    DrillDownView *m_parent;

};



DrillDownView::DrillDownView(QWidget *parent)
    :QListView(parent), needSlideLeftAnimation(false)
{
    setFrameShape(QFrame::NoFrame);

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
    if (current.isValid() || event->key() == Qt::Key_Left)
    {
        Qt::Key moveDirection = Qt::Key_Any;
        if ((event->key() == Qt::Key_Right) && (model()->hasChildren(current)))
            moveDirection = Qt::Key_Right;
        else if (event->key() == Qt::Key_Left)
            moveDirection = Qt::Key_Left;

        if (moveDirection != Qt::Key_Any)
        {
            //eat event if animation is running
            if (isBusy())
                return;

            if (moveDirection == Qt::Key_Right)
                slideRight();
            else
                slideLeft();
        }
    }
    if (event->key() == Qt::Key_Return)
        emit returnPressed(currentIndex());
    QListView::keyPressEvent(event);
}

void DrillDownView::slideRight()
{
    QModelIndex current = currentIndex();
    bool animate = true;
    setUpdatesEnabled(false);
    if (current != rootIndex())
    {
        if (model()->canFetchMore(current))
        {
            animate = false;
            model()->fetchMore(current);
            newUrlIndex = current;
        }
        else
        {
            setRootIndex(current);
            if (current.child(0, 0).isValid())
               setCurrentIndex(current.child(0, 0));
        }
    }
    else
        animate = false;
    if (animate)
        animateSlide(Qt::Key_Right);
    setUpdatesEnabled(true);
}

void DrillDownView::slideLeft()
{
    QModelIndex current = currentIndex();
    bool animate = true;
    setUpdatesEnabled(false);
    QModelIndex root = rootIndex();

    if (root.isValid())
    {
        setRootIndex(root.parent());
        setCurrentIndex(root);
    }
    else
    {
        //root can be invalid if dir lister has the directory other than "/" opened
        emit tryToSlideLeft();
        needSlideLeftAnimation = true;
        animate = false;
    }
    if (animate)
        animateSlide(Qt::Key_Left);
    setUpdatesEnabled(true);
}

void DrillDownView::animateSlide(int moveDirection)
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

void DrillDownView::verticalScrollbarValueChanged(int /*value*/)
{
    //do nothing and don't let the QAbstractItemView implementation
    //to try fetching more items, KDirModel doesn't like that
}

void DrillDownView::horizontalScrollbarValueChanged(int /*value*/)
{
    //do nothing and don't let the QAbstractItemView implementation
    //to try fetching more items, KDirModel doesn't like that
}

void DrillDownView::setDirty(const QRect& rect)
{
    setDirtyRegion(rect);
}

void DrillDownView::animateNewUrl()
{
    if (!newUrlIndex.isValid() && !needSlideLeftAnimation)
        return;

    int direction = Qt::Key_Right;
    if (newUrlIndex.isValid())
    {
        setRootIndex(newUrlIndex);
        if (newUrlIndex.child(0, 0).isValid())
            setCurrentIndex(newUrlIndex.child(0, 0));
    }
    else if (needSlideLeftAnimation)
    {
        if (model()->index(0, 0).isValid())
            setCurrentIndex(model()->index(0, 0));
        direction = Qt::Key_Left;
    }
    needSlideLeftAnimation = false;

    if (model()->rowCount(newUrlIndex) > 0)
    {
        setUpdatesEnabled(false);
        animateSlide(direction);
        setUpdatesEnabled(true);
    }

    newUrlIndex = QModelIndex();
}

bool DrillDownView::isBusy()
{
    return animation.state() == QTimeLine::Running;
}

void DrillDownView::setRootIndex(const QModelIndex &index)
{
    QListView::setRootIndex(index);
    emit rootIndexChanged(index);
}

void DrillDownView::scrollContentsBy(int dx, int dy)
{
    setDirty(normalizeDrillIconRect(rect()));
    QListView::scrollContentsBy(dx, dy);
}

QRect DrillDownView::normalizeDrillIconRect(const QRect &rect)
{
    QRect r(rect);
    r.setLeft(viewport()->width() - 16);
    r.setWidth(16);
    return r;
}

#include "drilldownview.moc"
