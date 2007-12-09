// **************************************************************************
//    begin                : Sun Aug 8 1999
//    copyright            : (C) 1999 by John Birch
//    email                : jbb@kdevelop.org
// **************************************************************************

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

#include "gdbparser.h"
#include "gdbcommand.h"
#include "gdbbreakpointwidget.h"
#include "gdbglobal.h"
#include "debuggerplugin.h"
#include "variableitem.h"
#include "frameitem.h"
#include "variablecollection.h"
#include "watchitem.h"

/** The variables widget is passive, and is invoked by the rest of the
    code via two main Q_SLOTS:
    - slotDbgStatus
    - slotCurrentFrame

    The first is received the program status changes and the second is
    recieved after current frame in the debugger can possibly changes.

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

using namespace GDBMI;

namespace GDBDebugger
{

VariableWidget::VariableWidget(CppDebuggerPlugin* plugin, GDBController*  controller,
                               QWidget *parent)
: QWidget(parent)
{
    setWindowIcon(KIcon("math_brace"));
    setWindowTitle(i18n("Debugger Variables"));

    varTree_ = new VariableTree(this, controller);
    setFocusProxy(varTree_);

    watchVarEditor_ = new KHistoryComboBox( this );

    QHBoxLayout* buttons = new QHBoxLayout();

    buttons->addStretch();

    QPushButton *evalButton = new QPushButton(i18n("&Evaluate"), this );
    buttons->addWidget(evalButton);

    QPushButton *addButton = new QPushButton(i18n("&Watch"), this );
    buttons->addWidget(addButton);

    QVBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->addWidget(varTree_, 10);
    topLayout->addWidget(watchVarEditor_);
    topLayout->addItem(buttons);


    connect( addButton, SIGNAL(clicked()), SLOT(slotAddWatchVariable()) );
    connect( evalButton, SIGNAL(clicked()), SLOT(slotEvaluateExpression()) );

    connect( watchVarEditor_, SIGNAL(returnPressed()),
             SLOT(slotEvaluateExpression()) );

    connect(plugin, SIGNAL(raiseVariableViews()), this, SIGNAL(requestRaise()));

    connect(this, SIGNAL(addWatchVariable(const QString&)), controller->variables(), SLOT(slotAddWatchVariable(const QString&)));
    connect(this, SIGNAL(evaluateExpression(const QString&)), controller->variables(), SLOT(slotEvaluateExpression(const QString&)));

    // Setup help items.

    setWhatsThis( i18n(
        "<b>Variable tree</b><p>"
        "The variable tree allows you to see the values of local "
        "variables and arbitrary expressions."
        "<p>Local variables are displayed automatically and are updated "
        "as you step through your program. "
        "For each expression you enter, you can either evaluate it once, "
        "or \"watch\" it (make it auto-updated). Expressions that are not "
        "auto-updated can be updated manually from the context menu. "
        "Expressions can be renamed to more descriptive names by clicking "
        "on the name column."
        "<p>To change the value of a variable or an expression, "
        "click on the value."));

    watchVarEditor_->setWhatsThis(
                    i18n("<b>Expression entry</b>"
                         "<p>Type in expression to evaluate."));

    evalButton->setWhatsThis(
                    i18n("Evaluate the expression."));

    addButton->setWhatsThis(
                    i18n("Evaluate the expression and "
                         "auto-update the value when stepping."));
}

void VariableWidget::slotAddWatchVariable()
{
    QString watchVar(watchVarEditor_->currentText());
    if (!watchVar.isEmpty())
    {
        slotAddWatchVariable(watchVar);
    }
}

// **************************************************************************

void VariableWidget::slotAddWatchVariable(const QString &ident)
{
    if (!ident.isEmpty())
    {
        watchVarEditor_->addToHistory(ident);
        emit addWatchVariable(ident);
        watchVarEditor_->clearEditText();
    }
}

void VariableWidget::slotEvaluateExpression()
{
    QString exp(watchVarEditor_->currentText());
    if (!exp.isEmpty())
    {
        slotEvaluateExpression(exp);
    }
}

void VariableWidget::slotEvaluateExpression(const QString &ident)
{
    if (!ident.isEmpty())
    {
        watchVarEditor_->addToHistory(ident);
        emit evaluateExpression(ident);
        watchVarEditor_->clearEditText();
    }
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

VariableTree::VariableTree(VariableWidget *parent,
                           GDBController*  controller)
    : QTreeView(parent),
      controller_(controller),
      activePopup_(0),
      toggleWatch_(0)
{
    setRootIsDecorated(true);
    setAllColumnsShowFocus(true);

    setModel(controller->variables());
}

// **************************************************************************

VariableTree::~VariableTree()
{
}

// **************************************************************************

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
                    variable->recreate();
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
        if (var && var->isAlive() && !controller()->stateIsOn(s_dbgNotStarted))
            controller_->addCommand(
                new GDBCommand(DataEvaluateExpression,
                    QString("&%1")
                    .arg(var->gdbExpression()),
                    this,
                    &VariableTree::handleAddressComputed,
                    true /*handles error*/));


        QAction* res = activePopup_->exec(QCursor::pos());
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
                var->recreate();
            }
        }

        event->accept();
    }
}


void VariableTree::updateCurrentFrame()
{
}


// **************************************************************************

class ValueSpecialRepresentationCommand : public QObject, public CliCommand
{
public:
    ValueSpecialRepresentationCommand(GDBMI::CommandType type, VariableItem* item, const QString& command)
    : CliCommand(type, command,
                 this,
                 &ValueSpecialRepresentationCommand::handleReply,
                 true),
      item_(item)
    {}

private:

    VariableItem* item_;

    void handleReply(const QStringList& lines)
    {
        QString s;
        for(int i = 1; i < lines.count(); ++i)
            s += lines[i];
        item_->updateSpecialRepresentation(s.toLocal8Bit());
    }
};

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

// **************************************************************************
// **************************************************************************
// **************************************************************************

}

#include "variablewidget.moc"

