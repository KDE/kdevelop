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

namespace KDevelop {
    
FocusedTreeView::FocusedTreeView(QWidget* parent) : QTreeView(parent) {
    setVerticalScrollMode(ScrollPerItem);
    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, &FocusedTreeView::resizeColumnsToContents);
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

void FocusedTreeView::resizeColumnsToContents() {
    if (!model()) {
        // might happen on shutdown
        return;
    }
    for(int a = 0; a < model()->columnCount(); ++a)
        resizeColumnToContents(a);
}

void FocusedTreeView::rowsInserted(const QModelIndex& parent, int start, int end) {
    QTreeView::rowsInserted(parent, start, end);
    resizeColumnsToContents();
}

}

