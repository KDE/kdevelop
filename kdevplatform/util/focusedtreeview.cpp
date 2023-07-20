/*
    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "focusedtreeview.h"

#include <QResizeEvent>
#include <QScrollBar>
#include <QTimer>

namespace KDevelop {

class FocusedTreeViewPrivate
{
public:
    bool autoScrollAtEnd = false;
    QTimer timer;
    bool wasAtEnd = false;
    int insertedBegin = -1;
    int insertedEnd = -1;
};

FocusedTreeView::FocusedTreeView(QWidget* parent)
    : QTreeView(parent)
    , d_ptr(new FocusedTreeViewPrivate)
{
    Q_D(FocusedTreeView);

    setVerticalScrollMode(ScrollPerItem);

    d->timer.setInterval(200);
    d->timer.setSingleShot(true);
    connect(&d->timer, &QTimer::timeout, this, &FocusedTreeView::delayedAutoScrollAndResize);

    connect(verticalScrollBar(), &QScrollBar::valueChanged,
            &d->timer, QOverload<>::of(&QTimer::start));
}

FocusedTreeView::~FocusedTreeView()
{
}

void FocusedTreeView::setModel(QAbstractItemModel* newModel)
{
    if (QAbstractItemModel* oldModel = model()) {
        disconnect(oldModel, nullptr, this, nullptr);
    }

    QTreeView::setModel(newModel);

    if (newModel) {
        connect(newModel, &QAbstractItemModel::rowsAboutToBeInserted, this, &FocusedTreeView::rowsAboutToBeInserted);
        connect(newModel, &QAbstractItemModel::rowsRemoved, this, &FocusedTreeView::rowsRemoved);
    }
}

void FocusedTreeView::setAutoScrollAtEnd(bool enable)
{
    Q_D(FocusedTreeView);

    d->autoScrollAtEnd = enable;
}

int FocusedTreeView::sizeHintForColumn(int column) const
{
    QModelIndex i = indexAt(QPoint(0, 0));
    if (i.isValid()) {
        QSize hint = sizeHintForIndex(i);
        int maxWidth = hint.width();
        if (hint.height()) {
            //Also consider one item above, because else we can get problems with
            //the vertical scroll-bar
            for (int a = -1; a < (height() / hint.height()) + 1; ++a) {
                QModelIndex current = i.sibling(i.row() + a, column);
                QSize tempHint = sizeHintForIndex(current);
                if (tempHint.width() > maxWidth)
                    maxWidth = tempHint.width();
            }
        }
        return maxWidth;
    }
    return columnWidth(column);
}

void FocusedTreeView::fitColumns()
{
    if (!model()) {
        return;
    }

    for (int c = 0, columnCount = model()->columnCount(); c < columnCount; ++c) {
        resizeColumnToContents(c);
    }
}

void FocusedTreeView::resizeEvent(QResizeEvent* event)
{
    QTreeView::resizeEvent(event);

    // Rewrap lines when the width changes.
    if (wordWrap() && event->size().width() != event->oldSize().width()) {
        // fitColumns() calls resizeColumnToContents(), which executes items layout
        // scheduled here (QTreeView::setWordWrap() also schedules items layout).
        // Redoing the items layout is necessary for correct rewrapping, but causes a roughly
        // 5-second-long UI freeze inside QItemDelegate::sizeHint() when the user selects
        // a 100'000th line of output. The freeze happens because non-uniform row heights (necessary
        // for word-wrapping) force QTreeView to calculate heights of all items above the selected one.
        // The number of consecutive resize events does not affect the duration of the resulting UI freeze.
        scheduleDelayedItemsLayout();
        // Resizing columns to contents here rewraps lines immediately rather than only after subsequent vertical
        // scrolling. This is relatively fast and does not noticeably contribute to the UI freeze duration.
        fitColumns();
    }
}

void FocusedTreeView::delayedAutoScrollAndResize()
{
    Q_D(FocusedTreeView);

    if (!model()) {
        // might happen on shutdown
        return;
    }

    if (d->autoScrollAtEnd && d->insertedBegin != -1 && d->wasAtEnd && d->insertedEnd == model()->rowCount()) {
        scrollToBottom();
    }

    for (int a = 0; a < model()->columnCount(); ++a)
        resizeColumnToContents(a);

    d->insertedBegin = -1;

    // Timer is single-shot, but the code above may have recursively restarted the timer
    // (e.g. via the connection from the scroll-bar signal), so explicitly prevent a redundant
    // call here.
    d->timer.stop();
}

void FocusedTreeView::rowsAboutToBeInserted(const QModelIndex&, int first, int last)
{
    Q_D(FocusedTreeView);

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
    Q_D(FocusedTreeView);

    QTreeView::rowsRemoved(parent, first, last);

    if (!d->timer.isActive())
        d->timer.start();
}

}

#include "moc_focusedtreeview.cpp"
