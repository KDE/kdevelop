
#ifndef TREEVIEW_H
#define TREEVIEW_H

#include <QTreeView>

namespace GDBDebugger
{
    class TreeModel;

    class AsyncTreeView : public QTreeView
    {
        Q_OBJECT
        public:
        AsyncTreeView(TreeModel* model, QWidget *parent);

    private slots:
        void slotExpanded(const QModelIndex &index);
        void slotCollapsed(const QModelIndex &index);
        void slotClicked(const QModelIndex &index);
    };

}



#endif
