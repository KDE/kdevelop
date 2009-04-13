
#ifndef TREEVIEW_H
#define TREEVIEW_H

#include <QTreeView>

#include "../debuggerexport.h"


namespace KDevelop
{
class TreeModel;

    class KDEVPLATFORMDEBUGGER_EXPORT AsyncTreeView : public QTreeView
    {
        Q_OBJECT
    public:
        AsyncTreeView(TreeModel* model, QWidget *parent);

        // Well, I really, really, need this.
        using QTreeView::indexRowSizeHint;

    private slots:
        void slotExpanded(const QModelIndex &index);
        void slotCollapsed(const QModelIndex &index);
        void slotClicked(const QModelIndex &index);
    };

}



#endif
