// **************************************************************************
//    begin                : Sun Aug 8 1999
//    copyright            : (C) 1999 by John Birch
//    email                : jbb@kdevelop.org
// **************************************************************************
// * Copyright 2006 Vladimir Prus <ghost@cs.msu.su>
// **************************************************************************
// *                                                                        *
// *   This program is free software; you can redistribute it and/or modify *
// *   it under the terms of the GNU General Public License as published by *
// *   the Free Software Foundation; either version 2 of the License, or    *
// *   (at your option) any later version.                                  *
// *                                                                        *
// **************************************************************************

#include "variablewidget.h"

#include <QLabel>
#include <QLayout>
#include <QPainter>
#include <QPushButton>
#include <QRegExp>
#include <QCursor>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QVBoxLayout>
#include <QPoint>
#include <QClipboard>

#include <kapplication.h>
#include <kmessagebox.h>
#include <khistorycombobox.h>
#include <kdebug.h>
#include <kmenu.h>
#include <klineedit.h>
#include <kdeversion.h>
#include <kiconloader.h>
#include <klocale.h>

#include "../../interfaces/icore.h"
#include "../../shell/debugcontroller.h"
#include "../interfaces/ivariablecontroller.h"
#include "variablecollection.h"

/** The variables widget is passive, and is invoked by the rest of the
    code via two main Q_SLOTS:
    - slotDbgStatus
    - slotCurrentFrame

    The first is received the program status changes and the second is
    received after current frame in the debugger can possibly changes.

    The widget has a list item for each frame/thread combination, with
    variables as children. However, at each moment only one item is shown.
    When handling the slotCurrentFrame, we check if variables for the
    current frame are available. If yes, we simply show the corresponding item.
    Otherwise, we fetch the new data from debugger.

    Fetching the data is done by emitting the produceVariablesInfo signal.
    In response, we get slotParametersReady and slotLocalsReady signal,
    in that order.

    The data is parsed and changed variables are highlighted. After that,
    we 'trim' variable items that were not reported by gdb -- that is, gone
    out of scope.
*/

// **************************************************************************
// **************************************************************************
// **************************************************************************

namespace KDevelop
{

VariableCollection *variableCollection()
{
    return ICore::self()->debugController()->variableCollection();
}


VariableWidget::VariableWidget(DebugController* controller, QWidget *parent)
: QWidget(parent), variablesRoot_(controller->variableCollection()->root())
{
  //setWindowIcon(KIcon("math_brace"));
    setWindowIcon(KIcon("debugger"));
    setWindowTitle(i18n("Debugger Variables"));

    varTree_ = new VariableTree(controller, this);
    setFocusProxy(varTree_);

    watchVarEditor_ = new KHistoryComboBox( this );

    QVBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->addWidget(varTree_, 10);
    topLayout->addWidget(watchVarEditor_);
    topLayout->setMargin(0);

    connect(watchVarEditor_, SIGNAL(returnPressed(const QString &)),
            this, SLOT(slotAddWatch(const QString&)));

    //TODO
    //connect(plugin, SIGNAL(raiseVariableViews()), this, SIGNAL(requestRaise()));

    // Setup help items.

    setWhatsThis( i18n(
        "<b>Variable tree</b><p>"
        "The variable tree allows you to see the values of local "
        "variables and arbitrary expressions.</p>"
        "<p>Local variables are displayed automatically and are updated "
        "as you step through your program. "
        "For each expression you enter, you can either evaluate it once, "
        "or \"watch\" it (make it auto-updated). Expressions that are not "
        "auto-updated can be updated manually from the context menu. "
        "Expressions can be renamed to more descriptive names by clicking "
        "on the name column.</p>"
        "<p>To change the value of a variable or an expression, "
        "click on the value.</p>"));

    watchVarEditor_->setWhatsThis(
                    i18n("<b>Expression entry</b>"
                         "<p>Type in expression to watch.</p>"));

}

void VariableWidget::slotAddWatch(const QString &expression)
{
    if (!expression.isEmpty())
    {
        watchVarEditor_->addToHistory(expression);
        kDebug(9012) << "Trying to add watch\n";
        Variable* v = variablesRoot_->watches()->add(expression);
        if (v) {
            QModelIndex index = variableCollection()->indexForItem(v, 0);
            /* For watches on structures, we really do want them to be shown
            expanded.  Except maybe for structure with custom pretty printing,
            but will handle that later.
            FIXME: it does not actually works now.
            */
            //varTree_->setExpanded(index, true);
        }
        watchVarEditor_->clearEditText();
    }
}

void VariableWidget::hideEvent(QHideEvent* e)
{
    QWidget::hideEvent(e);
    variableCollection()->variableWidgetHidden();
}

void VariableWidget::showEvent(QShowEvent* e)
{
    QWidget::showEvent(e);
    variableCollection()->variableWidgetShown();
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

VariableTree::VariableTree(DebugController *controller,
                           VariableWidget *parent)
: AsyncTreeView(controller->variableCollection(), parent)
#if 0
,
      activePopup_(0),
      toggleWatch_(0)
#endif
{
    setRootIsDecorated(true);
    setAllColumnsShowFocus(true);

    QModelIndex index = controller->variableCollection()->indexForItem(
        controller->variableCollection()->watches(), 0);
    setExpanded(index, true);
    
    m_signalMapper = new QSignalMapper(this);
    setupActions();
}


VariableCollection* VariableTree::collection() const
{
    Q_ASSERT(qobject_cast<VariableCollection*>(model()));
    return static_cast<VariableCollection*>(model());
}


VariableTree::~VariableTree()
{
    delete m_watchDelete;
    delete m_contextMenuTitle;
    delete m_formatMenu;
    delete m_signalMapper;
}

void VariableTree::setupActions()
{
    // TODO decorate this properly to make nice menu title
    m_contextMenuTitle = new QAction(this);
    m_contextMenuTitle->setEnabled(false);
    
    // make Format menu action group
    m_formatMenu = new QMenu(i18n("&Format"), this);
    QActionGroup *ag= new QActionGroup(m_formatMenu);
    
    QAction* act;
    
    act = new QAction(i18n("&Natural"), ag);
    act->setData(Variable::Natural);
    act->setShortcut(Qt::Key_N);
    m_formatMenu->addAction(act);

    act = new QAction(i18n("&Binary"), ag);
    act->setData(Variable::Binary);
    act->setShortcut(Qt::Key_B);
    m_formatMenu->addAction(act);
       
    act = new QAction(i18n("&Octal"), ag);
    act->setData(Variable::Octal);
    act->setShortcut(Qt::Key_O);
    m_formatMenu->addAction(act);
    
    act = new QAction(i18n("&Decimal"), ag);
    act->setData(Variable::Decimal);
    act->setShortcut(Qt::Key_D);
    m_formatMenu->addAction(act);
    
    act = new QAction(i18n("&Hexadecimal"), ag);
    act->setData(Variable::Hexadecimal);
    act->setShortcut(Qt::Key_H);
    m_formatMenu->addAction(act);
    
    foreach(QAction* act, m_formatMenu->actions())
    {
        act->setShortcutContext(Qt::WidgetWithChildrenShortcut);
        m_signalMapper->setMapping(act, act->data().toInt());
        connect(act, SIGNAL(triggered()), m_signalMapper, SLOT(map()));
        addAction(act);
    }
    
    m_watchDelete = new QAction(
        KIcon("edit-delete"), i18n( "Remove Watch Variable" ), this);
        
    m_watchDelete->setShortcut(Qt::Key_Delete);
    m_watchDelete->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    addAction(m_watchDelete);
}

void VariableTree::contextMenuEvent(QContextMenuEvent* event)
{
    QModelIndex index = indexAt(event->pos());
    if (!index.isValid())
        return;
    TreeItem* item = collection()->itemForIndex(index);
    Variable* var = dynamic_cast<Variable*>(item);
    if (!var) return;

    // set up menu
    QMenu contextMenu(this->parentWidget());
    m_contextMenuTitle->setText(var->expression());
    contextMenu.addAction(m_contextMenuTitle);
    
    if(var->canSetFormat())
        contextMenu.addMenu(m_formatMenu);

    // connect all format change actions to currently selected variable
    // the slotSetFormat slot will know the action by the action object name
    foreach(QAction* act, m_formatMenu->actions())
    {
        act->setCheckable(true);
        if(act->data().toInt()==var->getFormat())
            act->setChecked(true);
    }
        
    // connect is done when selecting the variable
    //connect(m_signalMapper, SIGNAL(mapped(int)), var, SLOT(slotSetFormat(int)));
  
    if (dynamic_cast<Watches*>(var->parent()))
    {
        contextMenu.addAction(m_watchDelete);
        //connect(m_watchDelete, SIGNAL(triggered()), var, SLOT(die()));
    }
    
    contextMenu.addSeparator();
    QAction* act=contextMenu.addAction(i18n("&Copy Value"), var, SLOT(slotCopyValueToClipboard()));
    act->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    act->setShortcut(QKeySequence::Copy);
        
    contextMenu.exec(event->globalPos());
    
    // remove signal connections since the action is reused for all variables
    // done when item gets deselected
    //disconnect(m_signalMapper, 0, var, 0);
    //disconnect(m_watchDelete, 0, var, 0);
}

void VariableTree::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Variable *var=NULL;

    if(selected.count()==1)
    {
        QModelIndex index = selected.indexes().first();
        if (index.isValid())
        {
            TreeItem* item = collection()->itemForIndex(index);
            var = dynamic_cast<Variable*>(item);
            if(var) // we have new variable selected
            {
                connect(m_signalMapper, SIGNAL(mapped(int)), var, SLOT(slotSetFormat(int)));
                if (dynamic_cast<Watches*>(var->parent()))
                    connect(m_watchDelete, SIGNAL(triggered()), var, SLOT(die()));
            }
        }
    }
    
    if(deselected.count()==1)
    {
        QModelIndex index = deselected.indexes().first();
        if (index.isValid())
        {
            TreeItem* item = collection()->itemForIndex(index);
            var = dynamic_cast<Variable*>(item);
            if(var) // old selection deselected
            {
                disconnect(m_signalMapper, 0, var, 0);
                if (dynamic_cast<Watches*>(var->parent()))
                    disconnect(m_watchDelete, 0, var, 0);
            }
        }
    }
    
    // var_old is deselected element (if any), var_new is selected element
    
    QTreeView::selectionChanged(selected, deselected);
}


#if 0
void VariableTree::contextMenuEvent(QContextMenuEvent* event)
{
    QModelIndex index = indexAt(event->pos());
    if (!index.isValid())
        return;

    AbstractVariableItem* item = collection()->itemForIndex(index);

    if (RecentItem* recent = qobject_cast<RecentItem*>(item))
    {
        KMenu popup(this);
        popup.addTitle(i18n("Recent Expressions"));
        QAction* remove = popup.addAction(KIcon("editdelete"), i18n("Remove All"));
        QAction* reevaluate = popup.addAction(KIcon("reload"), i18n("Re-evaluate All"));

        if (controller()->stateIsOn(s_dbgNotStarted))
            reevaluate->setEnabled(false);

        QAction* res = popup.exec(QCursor::pos());

        if (res == remove)
        {
            collection()->deleteItem(recent);
        }
        else if (res == reevaluate)
        {
            foreach (AbstractVariableItem* item, recent->children())
            {
                if (VariableItem* variable = qobject_cast<VariableItem*>(item))
                    variable->updateValue();
            }
        }
    }
    else
    {
        activePopup_ = new KMenu(this);
        KMenu format(this);

        QAction* remember = 0;
        QAction* remove = 0;
        QAction* reevaluate = 0;
        QAction* watch = 0;

        QAction* natural = 0;
        QAction* hex = 0;
        QAction* decimal = 0;
        QAction* character = 0;
        QAction* binary = 0;

#define MAYBE_DISABLE(action) if (!var->isAlive()) action->setEnabled(false)

        VariableItem* var = qobject_cast<VariableItem*>(item);
        if (var) {
            activePopup_->addTitle(var->gdbExpression());

            format.setTitle(i18n("Format"));

            QActionGroup* ag = new QActionGroup(&format);

            natural = format.addAction(i18n("Natural"));
            natural->setData(VariableItem::natural);
            natural->setShortcut(Qt::Key_N);

            hex = format.addAction(i18n("Hexadecimal"));
            hex->setData(VariableItem::hexadecimal);
            hex->setShortcut(Qt::Key_X);

            decimal = format.addAction(i18n("Decimal"));
            decimal->setData(VariableItem::decimal);
            decimal->setShortcut(Qt::Key_D);

            character = format.addAction(i18n("Character"));
            character->setData(VariableItem::character);
            character->setShortcut(Qt::Key_C);

            binary = format.addAction(i18n("Binary"));
            binary->setData(VariableItem::binary);
            binary->setShortcut(Qt::Key_T);

            foreach (QAction* action, ag->actions()) {
              action->setCheckable(true);
              if (action->data().toInt() == var->format())
                action->setChecked(true);
            }

            QAction* formatActions = activePopup_->addMenu(&format);
            MAYBE_DISABLE(formatActions);
        }


        AbstractVariableItem* root = item->abstractRoot();

        RecentItem* recentRoot = qobject_cast<RecentItem*>(root);

        if (!recentRoot)
        {
            remember = activePopup_->addAction(KIcon("draw-freehand"), i18n("Remember Value"));
            MAYBE_DISABLE(remember);
        }

        if (qobject_cast<WatchItem*>(root)) {
            remove = activePopup_->addAction(KIcon("editdelete"), i18n("Remove Watch Variable"));
            remove->setShortcut(Qt::Key_Delete);

        } else if (!recentRoot) {
            watch = activePopup_->addAction(i18n("Watch Variable"));
            MAYBE_DISABLE(watch);
        }

        if (recentRoot) {
            reevaluate = activePopup_->addAction(KIcon("reload"), i18n("Reevaluate Expression"));
            MAYBE_DISABLE(reevaluate);
            remove = activePopup_->addAction(KIcon("editdelete"), i18n("Remove Expression"));
            remove->setShortcut(Qt::Key_Delete);
        }

        if (var)
        {
            toggleWatch_ = activePopup_->addAction( i18n("Data write breakpoint") );
            toggleWatch_->setCheckable(true);
            toggleWatch_->setEnabled(false);
        }

        QAction* copyToClipboard = activePopup_->addAction(
            KIcon("editcopy"), i18n("Copy Value") );
        copyToClipboard->setShortcut(Qt::CTRL + Qt::Key_C);

        /* This code can be executed when debugger is stopped,
           and we invoke popup menu on a var under "recent expressions"
           just to delete it. */
        if (var && var->isAlive() && !controller()->stateIsOn(s_dbgNotStarted)) {
            GDBCommand* cmd =  new GDBCommand(DataEvaluateExpression,
                                                QString("&%1")
                                                .arg(var->gdbExpression()));
            cmd->setHandler(this, &VariableTree::handleAddressComputed, true /*handles error*/);
            cmd->setThread(var->thread());
            cmd->setFrame(var->frame());
            controller_->addCommand(cmd);
        }


        QAction* res = activePopup_->exec(event->globalPos());
        delete activePopup_;
        activePopup_ = 0;

        if (res == natural || res == hex || res == decimal
            || res == character || res == binary)
        {
            // Change format.
            VariableItem* var_item = static_cast<VariableItem*>(item);
            var_item->setFormat(static_cast<VariableItem::FormatTypes>(res->data().toInt()));
        }
        else if (res == remember)
        {
            if (var)
            {
                ((VariableWidget*)parent())->
                    slotEvaluateExpression(var->gdbExpression());
            }
        }
        else if (res == watch)
        {
            if (var)
            {
                ((VariableWidget*)parent())->
                    slotAddWatchVariable(var->gdbExpression());
            }
        }
        else if (res == remove)
        {
            delete item;
        }
        else if (res == copyToClipboard)
        {
            VariableTree::copyToClipboard(item);
        }
        else if (res == toggleWatch_)
        {
            if (var)
                emit toggleWatchpoint(var->gdbExpression());
        }
        else if (res == reevaluate)
        {
            if (var)
            {
                var->updateValue();
            }
        }

        event->accept();
    }
}


void VariableTree::updateCurrentFrame()
{
}

// **************************************************************************

void VariableTree::keyPressEvent(QKeyEvent* e)
{
    if (VariableItem* item = qobject_cast<VariableItem*>(collection()->itemForIndex(currentIndex())))
    {
        QString text = e->text();

        if (text == "n" || text == "x" || text == "d" || text == "c"
            || text == "t")
        {
            item->setFormat(
                item->formatFromGdbModifier(text[0].toLatin1()));
        }

        if (e->key() == Qt::Key_Delete)
        {
            AbstractVariableItem* root = item->abstractRoot();

            if (qobject_cast<WatchItem*>(root) || qobject_cast<RecentItem*>(root))
            {
                AbstractVariableItem* parent = item->abstractParent();
                Q_ASSERT(parent);
                if (parent)
                    parent->deleteChild(item);
            }
        }

        if (e->key() == Qt::Key_C && e->modifiers() == Qt::ControlModifier)
        {
            copyToClipboard(item);
        }
    }
}


void VariableTree::copyToClipboard(AbstractVariableItem* item)
{
    QClipboard *qb = KApplication::clipboard();
    QString text = item->data( 1, Qt::DisplayRole ).toString();

    qb->setText( text, QClipboard::Clipboard );
}

void VariableTree::handleAddressComputed(const GDBMI::ResultRecord& r)
{
    if (r.reason == "error")
    {
        // Not lvalue, leave item disabled.
        return;
    }

    if (activePopup_)
    {
        toggleWatch_->setEnabled(true);

        //quint64 address = r["value"].literal().toULongLong(0, 16);
        /*if (breakpointWidget_->hasWatchpointForAddress(address))
        {
            toggleWatch_->setChecked(true);
        }*/
    }
}

VariableCollection * VariableTree::collection() const
{
    return controller_->variables();
}

GDBController * VariableTree::controller() const
{
    return controller_;
}

void VariableTree::showEvent(QShowEvent * event)
{
    Q_UNUSED(event)

    for (int i = 0; i < model()->columnCount(); ++i)
        resizeColumnToContents(i);
}
#endif

// **************************************************************************
// **************************************************************************
// **************************************************************************

}

#include "variablewidget.moc"
