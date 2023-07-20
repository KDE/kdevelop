/*
    SPDX-FileCopyrightText: 2009 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_FOCUSEDTREEVIEW_H
#define KDEVPLATFORM_FOCUSEDTREEVIEW_H

#include <QTreeView>
#include "utilexport.h"

namespace KDevelop {
class FocusedTreeViewPrivate;

/**
 * Specialized version of QTreeView, that allows efficiently managing an extremely
 * long list of items, by focusing the size of the horizontal scroll-bars only on the currently
 * visible items.
 *
 * In addition, this class provides optional automatic scrolling when rows are inserted at the end.
 *
 * @warning Either the scroll-mode ScrollPerItem must be enabled, or the uniformRowHeight flag, for this to work efficiently.
 * @warning This currently only works with flat list models(todo).
 */
class KDEVPLATFORMUTIL_EXPORT FocusedTreeView : public QTreeView
{
    Q_OBJECT

public:
    explicit FocusedTreeView(QWidget* parent);
    ~FocusedTreeView() override;

    /**
     * When enabled, automatically scroll to bottom when new rows are inserted at the end
     * and the end was previously visible. (Default: false)
     */
    void setAutoScrollAtEnd(bool enable);

    void setModel(QAbstractItemModel* model) override;
    int sizeHintForColumn(int column) const override;

    /**
     * Resize columns to contents.
     *
     * Call this function after modifying the @a wordWrap property to rewrap lines
     * immediately rather than only after subsequent vertical scrolling.
     */
    void fitColumns();

protected:
    void resizeEvent(QResizeEvent* event) override;

private Q_SLOTS:
    void rowsAboutToBeInserted(const QModelIndex& parent, int first, int last);
    void rowsRemoved(const QModelIndex& parent, int first, int last);
    void delayedAutoScrollAndResize();

private:
    const QScopedPointer<class FocusedTreeViewPrivate> d_ptr;
    Q_DECLARE_PRIVATE(FocusedTreeView)
};

}

#endif // KDEVPLATFORM_FOCUSEDLISTVIEW_H
