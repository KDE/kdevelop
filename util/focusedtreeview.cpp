/*
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "focusedtreeview.h"

#include <QScrollBar>
#include <QTimer>

namespace KDevelop {

struct FocusedTreeView::Private {
    bool autoScrollAtEnd = false;
    QTimer timer;
    bool wasAtEnd = false;
    int insertedBegin = -1;
    int insertedEnd = -1;
};

FocusedTreeView::FocusedTreeView(QWidget* parent)
    : QTreeView(parent)
    , d(new Private)
{
    setVerticalScrollMode(ScrollPerItem);

    d->timer.setInterval(200);
    d->timer.setSingleShot(true);
    connect(&d->timer, &QTimer::timeout, this, &FocusedTreeView::delayedAutoScrollAndResize);

    connect(verticalScrollBar(), &QScrollBar::valueChanged, &d->timer, static_cast<void (QTimer::*)()>(&QTimer::start));
}

FocusedTreeView::~FocusedTreeView()
{
}

void FocusedTreeView::setModel(QAbstractItemModel* newModel)
{
    if (QAbstractItemModel* oldModel = model()) {
        disconnect(oldModel, 0, this, 0);
    }

    QTreeView::setModel(newModel);

    if (newModel) {
        connect(newModel, &QAbstractItemModel::rowsAboutToBeInserted, this, &FocusedTreeView::rowsAboutToBeInserted);
        connect(newModel, &QAbstractItemModel::rowsRemoved, this, &FocusedTreeView::rowsRemoved);
    }
}

void FocusedTreeView::setAutoScrollAtEnd(bool enable)
{
    d->autoScrollAtEnd = enable;
}

int FocusedTreeView::sizeHintForColumn(int column) const {
    QModelIndex i = indexAt(QPoint(0, 0));
    if(i.isValid()) {
        QSize hint = sizeHintForIndex(i);
        int maxWidth = hint.width();
        if(hint.height()) {
            //Also consider one item above, because else we can get problems with
            //the vertical scroll-bar
            for(int a = -1; a < (height() / hint.height())+1; ++a) {
                QModelIndex current = i.sibling(i.row()+a, column);
                QSize tempHint = sizeHintForIndex(current);
                if(tempHint.width() > maxWidth)
                    maxWidth = tempHint.width();
            }
        }
        return maxWidth;
    }
    return columnWidth(column);
}

void FocusedTreeView::delayedAutoScrollAndResize()
{
    if (!model()) {
        // might happen on shutdown
        return;
    }

    if (d->autoScrollAtEnd && d->insertedBegin != -1 && d->wasAtEnd && d->insertedEnd == model()->rowCount()) {
        scrollToBottom();
    }

    for(int a = 0; a < model()->columnCount(); ++a)
        resizeColumnToContents(a);

    d->insertedBegin = -1;

    // Timer is single-shot, but the code above may have recursively restarted the timer
    // (e.g. via the connection from the scroll-bar signal), so explicitly prevent a redundant
    // call here.
    d->timer.stop();
}

void FocusedTreeView::rowsAboutToBeInserted(const QModelIndex&, int first, int last)
{
    if (d->insertedBegin == -1) {
        d->insertedBegin = d->insertedEnd = first;

        d->wasAtEnd = true;
        QModelIndex last = model()->index(model()->rowCount() - 1, 0);
        if (last.isValid()) {
            auto rect = visualRect(last);
            d->wasAtEnd = rect.isValid() && viewport()->rect().intersects(rect);
        }
    }
    if (first == d->insertedEnd) {
        d->insertedEnd = last + 1;
    }

    if (!d->timer.isActive())
        d->timer.start();
}

// Removing rows can make longer rows move into the visible region, so we also must trigger a
// column resize
void FocusedTreeView::rowsRemoved(const QModelIndex& parent, int first, int last)
{
    QTreeView::rowsRemoved(parent, first, last);

    if (!d->timer.isActive())
        d->timer.start();
}


}
