/*
    SPDX-FileCopyrightText: 2008 Vladimir Prus <ghost@cs.msu.su>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_TREEVIEW_H
#define KDEVPLATFORM_TREEVIEW_H

#include <QTreeView>

#include <debugger/debuggerexport.h>

namespace KDevelop
{
class TreeModel;

    class KDEVPLATFORMDEBUGGER_EXPORT AsyncTreeView : public QTreeView
    {
        Q_OBJECT
    public:
        explicit AsyncTreeView(TreeModel& treeModel, QWidget* parent = nullptr);

        QSize sizeHint() const override;
        void resizeColumns();

        // Well, I really, really, need this.
        using QTreeView::indexRowSizeHint;

    protected:
        TreeModel& treeModel();

    private Q_SLOTS:
        void slotExpanded(const QModelIndex &index);
        void slotCollapsed(const QModelIndex &index);
        void slotClicked(const QModelIndex &index);
        void slotExpandedDataReady();

    private:
        virtual QModelIndex mapViewIndexToTreeModelIndex(const QModelIndex& viewIndex) const;

        TreeModel& m_treeModel;
    };

}



#endif
