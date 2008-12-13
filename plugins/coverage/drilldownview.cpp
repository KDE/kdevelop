/***************************************************************************
 *   Copyright 2005 Benjamin Meyer <ben@meyerhome.net>                     *
 *   Copyright 2006 Alexander Dymo <adymo@kdevelop.org>                    *
 *   Copyright 2008 Manuel Breugelmans <mbr.nxi@gmail.com>                 *
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

#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QAbstractScrollArea>

using Veritas::DrillDownView;

DrillDownView::DrillDownView(QWidget *parent)
        : QTableView(parent)
{
    setFrameShape((QFrame::Shape)(QFrame::StyledPanel | QFrame::Raised));
    setGridStyle(Qt::SolidLine);
    setShowGrid(true);

    connect(&animation, SIGNAL(frameChanged(int)), this, SLOT(slide(int)));
    connect(&animation, SIGNAL(finished()), this, SLOT(update()));
    animation.setDuration(125);
    setAlternatingRowColors(true); // hmz
    verticalHeader()->hide();
    setSortingEnabled(true);
    horizontalHeader()->resizeSection(0, width());

    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    resizeDirStateColumns();

    setSelectionBehavior(QAbstractItemView::SelectRows);
}

DrillDownView::~DrillDownView()
{}

void DrillDownView::paintEvent(QPaintEvent *event)
{
    if (animation.state() != QTimeLine::Running) {
        QTableView::paintEvent(event);
        return;
    }

    QPainter painter(viewport());
    if (animation.direction() == QTimeLine::Backward) {
        painter.drawPixmap(-animation.currentFrame(), 0, newView);
        painter.drawPixmap(-animation.currentFrame() + animation.endFrame(), 0, oldView);
    } else {
        painter.drawPixmap(-animation.currentFrame(), 0, oldView);
        painter.drawPixmap(-animation.currentFrame() + animation.endFrame(), 0, newView);
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
    if (isBusy()) {
        return; //eat event if animation is running
    }
    if (!(current.isValid() || event->key() == Qt::Key_Left)) {
        QTableView::keyPressEvent(event);
        return;
    }
    QSortFilterProxyModel* p = static_cast<QSortFilterProxyModel*>(model());
    QAbstractItemModel* m = p->sourceModel();
    QModelIndex srcIndex;
    switch(event->key()) {
    case Qt::Key_Right:
        current = current.sibling(current.row(), 0);
        srcIndex = p->mapToSource(current);
        if (m->hasChildren(srcIndex)) slideRight(current);
        return;
        break;
    case Qt::Key_Left:
        slideLeft();
        return;
        break;
    case Qt::Key_Return:
        emit returnPressed(current);
        break;
    default: break;
    }
    QTableView::keyPressEvent(event);
}

void DrillDownView::resizeFileStateColumns()
{
    QHeaderView* header = horizontalHeader();
    header->resizeSection(1, 75);
    header->resizeSection(2, 75);
    header->resizeSection(3, 75);
    header->resizeSection(0, (maxWidth()-225 > 75) ? maxWidth()-225 : 75);
}

int DrillDownView::maxWidth() const
{
    QScrollBar* vertbar = verticalScrollBar();
    Q_ASSERT(vertbar);
    int width = maximumViewportSize().width();
    if (vertbar->isVisible()) {
        width = width - vertbar->width();
    }
    return width - 5;
}

void DrillDownView::resizeDirStateColumns()
{
    horizontalHeader()->resizeSection(1, 20);
    horizontalHeader()->resizeSection(0, maxWidth() - 20);
}

void DrillDownView::slideRight(const QModelIndex& current)
{
    if (current == rootIndex()) return;
    if (model()->canFetchMore(current)) {
        model()->fetchMore(current);
        return;
    }
    setUpdatesEnabled(false);

    leftSelection = selectionModel()->selection();
    selectionModel()->clear();

    setRootIndex(current);

    animateSlide(Qt::Key_Right);
    resizeFileStateColumns();
    setUpdatesEnabled(true);
    emit completedSlideRight();
 }

void DrillDownView::slideLeft()
{
    QModelIndex current = currentIndex();
    QModelIndex root = rootIndex();
    if (!root.isValid()) return;
    setUpdatesEnabled(false);

    setRootIndex(root.parent());

    selectionModel()->clear();
    selectionModel()->select(leftSelection, QItemSelectionModel::Select);
    selectionModel()->setCurrentIndex(root, QItemSelectionModel::NoUpdate);

    animateSlide(Qt::Key_Left);
    resizeDirStateColumns();
    setUpdatesEnabled(true);
    emit completedSlideLeft();
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

bool DrillDownView::isBusy()
{
    return animation.state() == QTimeLine::Running;
}

void DrillDownView::setRootIndex(const QModelIndex &index)
{
    QTableView::setRootIndex(index);
    emit rootIndexChanged(index);
}

#include "drilldownview.moc"
