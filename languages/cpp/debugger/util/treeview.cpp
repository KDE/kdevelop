
#include "treeview.h"
#include "treemodel.h"

using namespace GDBDebugger;

AsyncTreeView::AsyncTreeView(TreeModel* model, QWidget *parent = 0) 
: QTreeView(parent)
{
    setModel(model);
    connect (this, SIGNAL(expanded(const QModelIndex &)),
             this, SLOT(slotExpanded(const QModelIndex &)));
    connect (this, SIGNAL(collapsed(const QModelIndex &)),
             this, SLOT(slotCollapsed(const QModelIndex &)));
    connect (this, SIGNAL(clicked(const QModelIndex &)),
             this, SLOT(slotClicked(const QModelIndex &)));
}


void AsyncTreeView::slotExpanded(const QModelIndex &index)
{
    static_cast<TreeModel*>(model())->expanded(index);
}

void AsyncTreeView::slotCollapsed(const QModelIndex &index)
{
    static_cast<TreeModel*>(model())->collapsed(index);
}

void AsyncTreeView::slotClicked(const QModelIndex &index)
{
    static_cast<TreeModel*>(model())->clicked(index);
}


