
#include "tooltipwidget.h"

#include "variablecollection.h"
#include "breakpointcontroller.h"
#include "gdbcontroller.h"
#include "gdbcommand.h"
#include "mi/gdbmi.h"

#include "util/treeview.h"
#include "util/treemodel.h"
#include "util/activetooltip.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QTreeView>
#include <QApplication>
#include <QEvent>
#include <QStandardItemModel>
#include <QMouseEvent>
#include <QHeaderView>
#include <QLabel>
#include <KTextEditor/View>

using namespace GDBDebugger;

VariableToolTip::VariableToolTip(QWidget* parent, QPoint position, 
                                 GDBController* controller,
                                 const QString& identifier)
: ActiveToolTip(parent, position), controller_(controller)
{
    model_ = new TreeModel(QVector<QString>() << "Name" << "Type",
                           this);
    TooltipRoot* tr = new TooltipRoot(model_);    
    model_->setRootItem(tr);
    tr->init(controller, identifier);
    var_ = tr->var;

    controller->addCommand(
        new GDBCommand(
            GDBMI::VarCreate, 
            QString("var%1 @ %2").arg(Variable::nextId_++).arg(identifier),
            this, &VariableToolTip::handleCreated, true));
    
    QVBoxLayout* l = new QVBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    QTreeView* view = new GDBDebugger::AsyncTreeView(model_, this);
    view->header()->resizeSection(0, 200);
    view->header()->resizeSection(1, 90);
    view->header()->hide();
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setSelectionMode(QAbstractItemView::SingleSelection);    
    l->addWidget(view);

    selection_ = view->selectionModel();
    selection_->select(model_->indexForItem(var_, 0), 
                       QItemSelectionModel::Rows
                       | QItemSelectionModel::ClearAndSelect);

    QHBoxLayout* inner = new QHBoxLayout();
    inner->setContentsMargins(11, 0, 11, 6);
    l->addLayout(inner);
    QLabel* label = new QLabel("<a href=add_watch>Watch this</a>", this);
    inner->addWidget(label);
    QLabel* label2 = new QLabel("<a href=watchpoint>Stop on change</a>", 
                                this);
    inner->addWidget(label2);
    connect(label, SIGNAL(linkActivated(const QString&)),
            this, SLOT(slotLinkActivated(const QString&)));
    connect(label2, SIGNAL(linkActivated(const QString&)),
            this, SLOT(slotLinkActivated(const QString&)));
        
    move(position);
    resize(300, 100);
}

void VariableToolTip::handleCreated(const GDBMI::ResultRecord& r)
{
    if (r.reason == "done" && r.hasField("value")
        && !r["value"].literal().isEmpty())
    {
        var_->handleCreation(r);
        show();
    }
    else
    {
        close();
    }
}

void VariableToolTip::addWatch(const GDBMI::ResultRecord& r)
{
    // FIXME: handle error.
    if (r.reason == "done")
    {
        controller_->variables()->watches()
            ->add(r["path_expr"].literal());
    }
    close();
}

void VariableToolTip::addWatchpoint(const GDBMI::ResultRecord& r)
{
    // FIXME: handle error.
    if (r.reason == "done")
    {
        controller_->breakpoints()->breakpointsItem()
            ->addWatchpoint(r["path_expr"].literal());
    }
    close();
}

void VariableToolTip::slotLinkActivated(const QString& link)
{
    Variable* v = var_;
    QItemSelection s = selection_->selection();
    if (!s.empty())
    {
        QModelIndex index = s.front().topLeft();
        TreeItem *item = model_->itemForIndex(index);
        if (item)
        {
            Variable* v2 = dynamic_cast<Variable*>(item);
            if (v2)
                v = v2;
        }            
    }

    if (link == "add_watch")
    {
        controller_->addCommand(
            new GDBCommand(GDBMI::VarInfoPathExpression,
                           v->varobj(),
                           this,
                           &VariableToolTip::addWatch));
    }
    else if (link == "watchpoint")
    {
        controller_->addCommand(
            new GDBCommand(GDBMI::VarInfoPathExpression,
                           v->varobj(),
                           this,
                           &VariableToolTip::addWatchpoint));
    }
    hide();
}

#include "tooltipwidget.moc"
