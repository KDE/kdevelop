
#ifndef TOOLTIP_H_f2dc78af9cdf452ce712f673b2bbcd00
#define TOOLTIP_H_f2dc78af9cdf452ce712f673b2bbcd00

#include "variablecollection.h"
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
#include <KTextEditor/View>

// FIXME:
using namespace GDBDebugger;

class VariableToolTip : public ActiveToolTip
{
    Q_OBJECT
public:
    VariableToolTip(QWidget* parent, QPoint position, 
                    GDBController* controller,
                    const QString& identifier)
    : ActiveToolTip(parent, position)
    {
        TreeModel* model = new TreeModel(QVector<QString>() << "Name" << "Type",
                                         this);
        TooltipRoot* tr = new TooltipRoot(model);    
        model->setRootItem(tr);
        tr->init(controller, identifier);
        var_ = tr->var;

        controller->addCommand(
            new GDBCommand(
                GDBMI::VarCreate, 
                QString("var%1 @ %2").arg(Variable::nextId_++).arg(identifier),
                this, &VariableToolTip::handleCreated, true));
    
        QHBoxLayout* l = new QHBoxLayout(this);
        l->setContentsMargins(0, 0, 0, 0);
        QTreeView* view = new GDBDebugger::AsyncTreeView(model, this);
        view->header()->resizeSection(0, 200);
        view->header()->resizeSection(1, 90);
        view->header()->hide();
        l->addWidget(view);

        move(position);
        resize(300, 100);
    }

    void handleCreated(const GDBMI::ResultRecord& r)
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

private:
    Variable* var_;
};


#endif
