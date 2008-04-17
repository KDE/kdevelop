
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



#if 0
        QStandardItemModel *m = new QStandardItemModel(this);
        QStandardItem *parentItem = m->invisibleRootItem();
        for (int i = 0; i < 4; ++i) {
            QStandardItem *item = new QStandardItem(QString("item %0").arg(i));
            parentItem->appendRow(item);
            parentItem = item;
        }
        view->setModel(m);
#endif
        
//        view->setPalette(p);

        move(position);
        resize(300, 100);
#if 0
        rect = geometry();
        rect.moveTo(parent->mapFromGlobal(rect.topLeft()));
        rect.adjust(-10, -10, 0, 0);
        setMouseTracking(true);
        mouseOut = 0;
#endif
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

#if 0
    bool eventFilter(QObject *o, QEvent *e)
    {
        switch (e->type()) {

            // Don't need this in our case.
#if 0
        case QEvent::Leave:
            close();
            break;
#endif

        case QEvent::WindowActivate:
        case QEvent::WindowDeactivate:
            close();
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        {
            /* If the click is within tooltip, it's fine.
               Clicks outside close it.  */
            bool ok = false;
            while (o)
            {
                if (o == this)
                {
                    ok = true;
                    break;
                }
                o = o->parent();
            }
            if (!ok)
                close();            
        }
        // FIXME: revisit this code later.
#if 0
        case QEvent::FocusIn:
        case QEvent::FocusOut:
        case QEvent::Wheel:
            close();
            break;
#endif
        case QEvent::MouseMove:
            if (o == parent() 
                && !rect.isNull() 
                && !rect.contains(static_cast<QMouseEvent*>(e)->pos()))
                // On X, when the cursor leaves the tooltip and enters
                // the parent, we sotimes get some wrong Y coordinate.
                // Don't know why, so wait for two out-of-range mouse
                // positions before closing.
                ++mouseOut;
            else               
                mouseOut = 0;
            if (mouseOut == 2)
                close();
        default:
            break;
        }
        return false;
    }



private slots:
    void viewFocusOut()
    {
        close();
    }
#endif

#if 0
private:
    QRect rect;
    int mouseOut;
#endif
};


#endif
