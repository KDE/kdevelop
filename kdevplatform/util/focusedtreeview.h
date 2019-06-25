/* This file is part of KDevelop
 *
 * Copyright 2009 David Nolden <david.nolden.kdevelop@art-master.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
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
