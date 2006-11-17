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
#include "gdbparser.h"
#include "gdbcommand.h"
#include "gdbbreakpointwidget.h"

#include <kdebug.h>
#include <kpopupmenu.h>
#include <klineedit.h>
#include <kdeversion.h>
#include <kiconloader.h>

#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qcursor.h>
#include <qwhatsthis.h>
#include <klocale.h>

#include <qpoint.h>
#include <qclipboard.h>
#include <kapplication.h>
#include <kmessagebox.h>

#include <set>
#include <typeinfo>

/** The variables widget is passive, and is invoked by the rest of the
    code via two main slots:
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

namespace GDBDebugger
{

VariableWidget::VariableWidget(GDBController*  controller, 
                               GDBBreakpointWidget* breakpointWidget,
                               QWidget *parent, const char *name)
: QWidget(parent, name)
{
    setIcon(SmallIcon("math_brace"));
    setCaption(i18n("Variable Tree"));

    varTree_ = new VariableTree(this, controller, breakpointWidget);
    
    watchVarEditor_ = new KHistoryCombo( this, 
                                         "var-to-watch editor");

    QHBoxLayout* buttons = new QHBoxLayout();

    buttons->addStretch();

    QPushButton *evalButton = new QPushButton(i18n("&Evaluate"), this );
    buttons->addWidget(evalButton);   

    QPushButton *addButton = new QPushButton(i18n("&Watch"), this );
    buttons->addWidget(addButton);

    QVBoxLayout *topLayout = new QVBoxLayout(this, 2);
    topLayout->addWidget(varTree_, 10);
    topLayout->addWidget(watchVarEditor_);
    topLayout->addItem(buttons);
    

    connect( addButton, SIGNAL(clicked()), SLOT(slotAddWatchVariable()) );
    connect( evalButton, SIGNAL(clicked()), SLOT(slotEvaluateExpression()) );

    connect( watchVarEditor_, SIGNAL(returnPressed()), 
             SLOT(slotEvaluateExpression()) );

    connect(controller, SIGNAL(event(GDBController::event_t)),
            varTree_,       SLOT(slotEvent(GDBController::event_t)));


    // Setup help items.

    QWhatsThis::add(this, i18n(
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

    QWhatsThis::add(watchVarEditor_, 
                    i18n("<b>Expression entry</b>"
                         "<p>Type in expression to evaluate."));

    QWhatsThis::add(evalButton, 
                    i18n("Evaluate the expression."));

    QWhatsThis::add(addButton, 
                    i18n("Evaluate the expression and "
                         "auto-update the value when stepping."));
}

void VariableWidget::slotAddWatchVariable()
{
//    QString watchVar(watchVarEntry_->text());
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
        varTree_->slotAddWatchVariable(ident);
        watchVarEditor_->clearEdit();
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
        varTree_->slotEvaluateExpression(ident);
        watchVarEditor_->clearEdit();
    }    
}

// **************************************************************************

void VariableWidget::focusInEvent(QFocusEvent */*e*/)
{
    varTree_->setFocus();
}




// **************************************************************************
// **************************************************************************
// **************************************************************************

VariableTree::VariableTree(VariableWidget *parent, 
                           GDBController*  controller,
                           GDBBreakpointWidget* breakpointWidget,
                           const char *name)
    : KListView(parent, name),
      QToolTip( viewport() ),
      controller_(controller),
      breakpointWidget_(breakpointWidget),
      activeFlag_(0),
      recentExpressions_(0),
      currentFrameItem(0),
      activePopup_(0)
{
    setRootIsDecorated(true);
    setAllColumnsShowFocus(true);
    setSorting(-1);
    QListView::setSelectionMode(QListView::Single);    

    // Note: it might be reasonable to set width of value
    // column to 10 characters ('0x12345678'), and rely on
    // tooltips for showing larger values. Currently, both
    // columns will get roughly equal width.
    addColumn(i18n("Variable"));
    addColumn(i18n("Value"));
    setResizeMode(AllColumns);

    connect( this, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
             SLOT(slotContextMenu(KListView*, QListViewItem*)) );
    connect( this, SIGNAL(itemRenamed( QListViewItem*, int, const QString&)),
             this, SLOT(slotItemRenamed( QListViewItem*, int, const QString&)));
}

// **************************************************************************

VariableTree::~VariableTree()
{
}

// **************************************************************************

void VariableTree::slotContextMenu(KListView *, QListViewItem *item)
{
    if (!item)
        return;

    setSelected(item, true);    // Need to select this item.

    if (item->parent())
    {
        KPopupMenu popup(this);
        KPopupMenu format(this);

        int idRemember = -2;
        int idRemove = -2;
        int idReevaluate = -2;        
        int idWatch = -2;
        
        int idNatural = -2;
        int idHex = -2;
        int idDecimal = -2;
        int idCharacter = -2;
        int idBinary = -2;

        VarItem* var;
        if ((var = dynamic_cast<VarItem*>(item))) 
        {
            popup.insertTitle(var->gdbExpression());
            

            format.setCheckable(true);
            idNatural = format.insertItem(i18n("Natural"), 
                                          (int)VarItem::natural);
            format.setAccel(Qt::Key_N, idNatural);
            idHex = format.insertItem(i18n("Hexadecimal"), 
                                      (int)VarItem::hexadecimal);
            format.setAccel(Qt::Key_X, idHex);
            idDecimal = format.insertItem(i18n("Decimal"), 
                                          (int)VarItem::decimal);
            format.setAccel(Qt::Key_D, idDecimal);
            idCharacter = format.insertItem(i18n("Character"),
                                          (int)VarItem::character);
            format.setAccel(Qt::Key_C, idCharacter);
            idBinary = format.insertItem(i18n("Binary"), 
                                         (int)VarItem::binary);
            format.setAccel(Qt::Key_T, idBinary);


            format.setItemChecked((int)(var->format()), true);

            popup.insertItem(i18n("Format"), &format);
        }
        

        QListViewItem* root = findRoot(item);

        if (root != recentExpressions_)
        {
            idRemember = popup.insertItem(
                SmallIcon("pencil"), i18n("Remember Value"));
        }

        if (dynamic_cast<WatchRoot*>(root)) {
            idRemove = popup.insertItem( 
                SmallIcon("editdelete"), i18n("Remove Watch Variable") );
            popup.setAccel(Qt::Key_Delete, idRemove);
        } else if (root != recentExpressions_) {
            idWatch = popup.insertItem(
                i18n("Watch Variable"));
        }
        if (root == recentExpressions_) {
            idReevaluate = popup.insertItem( 
                SmallIcon("reload"), i18n("Reevaluate Expression") );
            idRemove = popup.insertItem( 
                SmallIcon("editdelete"), i18n("Remove Expression") );
            popup.setAccel(Qt::Key_Delete, idRemove);
        }

        if (var)
        {
            popup.insertItem( i18n("Data write breakpoint"), idToggleWatch );
            popup.setItemEnabled(idToggleWatch, false);
        }

        int	idCopyToClipboard = popup.insertItem( 
            SmallIcon("editcopy"), i18n("Copy Value") );
        popup.setAccel(Qt::CTRL + Qt::Key_C, idCopyToClipboard);

        activePopup_ = &popup;
        if (var)
            controller_->addCommand(
                new GDBCommand(
                    QString("-data-evaluate-expression &%1")
                    .arg(var->gdbExpression()),
                    this,
                    &VariableTree::handleAddressComputed,
                    true /*handles error*/));
        

        int res = popup.exec(QCursor::pos());

        activePopup_ = 0;

        
        if (res == idNatural || res == idHex || res == idDecimal
            || res == idCharacter || res == idBinary)
        {
            // Change format.
            VarItem* var_item = static_cast<VarItem*>(item);
            var_item->setFormat(static_cast<VarItem::format_t>(res));
        }
        else if (res == idRemember)
        {
            if (VarItem *item = dynamic_cast<VarItem*>(currentItem()))
            {
                ((VariableWidget*)parent())->
                    slotEvaluateExpression(item->gdbExpression());
            }
        } 
        else if (res == idWatch)
        {
            if (VarItem *item = dynamic_cast<VarItem*>(currentItem()))
            {
                ((VariableWidget*)parent())->
                    slotAddWatchVariable(item->gdbExpression());
            }
        } 
        else if (res == idRemove)
            delete item;
        else if (res == idCopyToClipboard)
        {
            copyToClipboard(item);
        }
        else if (res == idToggleWatch)
        {
            if (VarItem *item = dynamic_cast<VarItem*>(currentItem()))
                emit toggleWatchpoint(item->gdbExpression());
        }
        else if (res == idReevaluate)
        {
            if (VarItem* item = dynamic_cast<VarItem*>(currentItem()))
            {
                item->recreate();
            }
        }
    }
    else if (item == recentExpressions_)
    {
        KPopupMenu popup(this);
        popup.insertTitle(i18n("Recent Expressions"));
        int idRemove = popup.insertItem(
            SmallIcon("editdelete"), i18n("Remove All"));
        int idReevaluate = popup.insertItem(
            SmallIcon("reload"), i18n("Reevaluate All"));
        int res = popup.exec(QCursor::pos());
        
        if (res == idRemove)
        {
            delete recentExpressions_;
            recentExpressions_ = 0;
        }
        else if (res == idReevaluate)
        {
            for(QListViewItem* child = recentExpressions_->firstChild();
                child; child = child->nextSibling())
            {
                static_cast<VarItem*>(child)->recreate();
            }
        }
    }
}

void VariableTree::slotEvent(GDBController::event_t event)
{
    switch(event)
    {
        case GDBController::program_exited:
        case GDBController::debugger_exited:
        {
            // Remove all locals.
            QListViewItem *child = firstChild();

            while (child) {
                QListViewItem *nextChild = child->nextSibling();

                // don't remove the watch root, or 'recent expressions' root.
                if (!(dynamic_cast<WatchRoot*> (child)) 
                    && child != recentExpressions_) 
                {
                    delete child;
                }
                child = nextChild;
            }
            currentFrameItem = 0;

            if (recentExpressions_)
            {
                for(QListViewItem* child = recentExpressions_->firstChild();
                    child; child = child->nextSibling())
                {
                    static_cast<VarItem*>(child)->unhookFromGdb();
                }
            }

            if (WatchRoot* w = findWatch())
            {
                for(QListViewItem* child = w->firstChild();
                    child; child = child->nextSibling())
                {
                    static_cast<VarItem*>(child)->unhookFromGdb();
                }                
            }

            break;
        }

        case GDBController::program_state_changed:

            // Fall-through intended.

        case GDBController::thread_or_frame_changed:
            {
                VarFrameRoot *frame = demand_frame_root(
                    controller_->currentFrame(), controller_->currentThread());

                if (frame->isOpen())
                {
                    updateCurrentFrame();
                }
                else
                {
                    frame->setDirty();
                }                
            }
            break;

        default:
            break;
    }
}

void VariableTree::updateCurrentFrame()
{
    // In GDB 6.4, the -stack-list-locals command is broken.
    // If there's any local reference variable which is not
    // initialized yet, for example because it's in the middle
    // of function, gdb will still print it and try to dereference
    // it. If the address in not accessible, the MI command will
    // exit with an error, and we won't be able to see *any*
    // locals. A patch is submitted:
    //    http://sourceware.org/ml/gdb-patches/2006-04/msg00069.html
    // but we need to work with 6.4, not with some future version. So,
    // we just -stack-list-locals to get just names of the locals,
    // but not their values.
    // We'll fetch values separately:

    controller_->addCommand(
        new GDBCommand(QString("-stack-list-arguments 0 %1 %2")
                       .arg(controller_->currentFrame())
                       .arg(controller_->currentFrame())
                       .ascii(),
                       this,
                       &VariableTree::argumentsReady));


    controller_->addCommand(
        new GDBCommand("-stack-list-locals 0",
                       this,
                       &VariableTree::localsReady));

}


// **************************************************************************

void VariableTree::slotAddWatchVariable(const QString &watchVar)
{
    VarItem *varItem = 0;
    varItem = new VarItem(findWatch(), watchVar);
}

void VariableTree::slotEvaluateExpression(const QString &expression)
{
    if (recentExpressions_ == 0)
    {
        recentExpressions_ = new TrimmableItem(this);
        recentExpressions_->setText(0, "Recent");
        recentExpressions_->setOpen(true);
    }

    VarItem *varItem = new VarItem(recentExpressions_, 
                                   expression,
                                   true /* freeze */);
    varItem->setRenameEnabled(0, 1);            
}

// **************************************************************************

QListViewItem *VariableTree::findRoot(QListViewItem *item) const
{
    while (item->parent())
        item = item->parent();

    return item;
}

// **************************************************************************

VarFrameRoot *VariableTree::findFrame(int frameNo, int threadNo) const
{
    QListViewItem *sibling = firstChild();

    // frames only exist on th top level so we only need to
    // check the siblings
    while (sibling) {
        VarFrameRoot *frame = dynamic_cast<VarFrameRoot*> (sibling);
        if (frame && frame->matchDetails(frameNo, threadNo))
            return frame;

        sibling = sibling->nextSibling();
    }

    return 0;
}

// **************************************************************************

WatchRoot *VariableTree::findWatch()
{
    QListViewItem *sibling = firstChild();

    while (sibling) {
        if (WatchRoot *watch = dynamic_cast<WatchRoot*> (sibling))
            return watch;

        sibling = sibling->nextSibling();
    }

    return new WatchRoot(this);
}

// **************************************************************************

QListViewItem *VariableTree::lastChild() const
{
    QListViewItem *child = firstChild();
    if (child)
        while (QListViewItem *nextChild = child->nextSibling())
            child = nextChild;

    return child;
}

// **************************************************************************

void VariableTree::maybeTip(const QPoint &p)
{
    VarItem * item = dynamic_cast<VarItem*>( itemAt( p ) );
    if ( item )
    {
        QRect r = itemRect( item );
        if ( r.isValid() )
            tip( r, item->tipText() );
    }
}

class ValueSpecialRepresentationCommand : public QObject, public CliCommand
{
public:
    ValueSpecialRepresentationCommand(VarItem* item, const QString& command)
    : CliCommand(command.ascii(),
                 this,
                 &ValueSpecialRepresentationCommand::handleReply),
      item_(item)
    {}

private:

    VarItem* item_;

    void handleReply(const QValueVector<QString>& lines)
    {
        QString s;
        for(unsigned i = 1; i < lines.count(); ++i)
            s += lines[i];
        item_->updateSpecialRepresentation(s.local8Bit());
    }
};

void VariableTree::slotVarobjNameChanged(
    const QString& from, const QString& to)
{
    if (!from.isEmpty())
        varobj2varitem.erase(from);

    if (!to.isEmpty())
        varobj2varitem[to] = 
            const_cast<VarItem*>(
                static_cast<const VarItem*>(sender()));
}



VarFrameRoot* VariableTree::demand_frame_root(int frameNo, int threadNo)
{
    VarFrameRoot *frame = findFrame(frameNo, threadNo); 
    if (!frame)
    {
        frame = new VarFrameRoot(this, frameNo, threadNo);
        frame->setFrameName(i18n("Locals"));
        // Make sure "Locals" item is always the top item, before
        // "watch" and "recent experessions" items.
        this->takeItem(frame);
        this->insertItem(frame);
        frame->setOpen(true);
    }
    return frame;
}

void VariableTree::argumentsReady(const GDBMI::ResultRecord& r)
{
    const GDBMI::Value& args = r["stack-args"][0]["args"];

    fetch_time.start();

    locals_and_arguments.clear();
    for(unsigned i = 0; i < args.size(); ++i)
    {
        locals_and_arguments.push_back(args[i].literal());
    }    
}

void VariableTree::localsReady(const GDBMI::ResultRecord& r)
{    
    const GDBMI::Value& locals = r["locals"];

    for(unsigned i = 0; i < locals.size(); ++i)
    {
        QString val = locals[i].literal();

        // Check ada internal variables like <R45b>, <L23R> ...
        bool is_ada_variable = (val[0] == '<' && val[val.length() - 1] == '>');

        if (!is_ada_variable)
        {
            locals_and_arguments.push_back(val);
        }
    }

    controller_->addCommand(new CliCommand("info frame",
                                           this,
                                           &VariableTree::frameIdReady));
}

void VariableTree::frameIdReady(const QValueVector<QString>& lines)
{
    //kdDebug(9012) << "localAddresses: " << lines[1] << "\n";

    QString frame_info;
    for(unsigned i = 1; i < lines.size(); ++i)
        frame_info += lines[i];

    kdDebug(9012) << "frame info: " << frame_info << "\n";
    frame_info.replace('\n', "");

    static QRegExp frame_base_rx("frame at 0x([0-9a-fA-F]*)");
    static QRegExp frame_code_rx("saved [a-zA-Z0-9]* 0x([0-9a-fA-F]*)");

    int i = frame_base_rx.search(frame_info);
    int i2 = frame_code_rx.search(frame_info);

    bool frameIdChanged = false;

    VarFrameRoot *frame = demand_frame_root(
        controller_->currentFrame(), controller_->currentThread());

    if (frame != currentFrameItem)
    {
        if (currentFrameItem)
        {
            currentFrameItem->setVisible(false);
        }
    }
    currentFrameItem = frame;
    currentFrameItem->setVisible(true);


    if (i != -1 && i2 != -1)
    {
        unsigned long long new_frame_base = 
            frame_base_rx.cap(1).toULongLong(0, 16);
        unsigned long long new_code_address = 
            frame_code_rx.cap(1).toULongLong(0, 16);
        kdDebug(9012) << "Frame base = " << QString::number(new_frame_base, 16)
                      << " code = " << QString::number(new_code_address, 16)
                      << "\n";
        kdDebug(9012) << "Previous frame " << 
            QString::number(frame->currentFrameBase, 16)
                      << " code = " << QString::number(
                          frame->currentFrameCodeAddress, 16)
                      << "\n";

        frameIdChanged = (new_frame_base != frame->currentFrameBase ||
                          new_code_address != frame->currentFrameCodeAddress);

        frame->currentFrameBase = new_frame_base;
        frame->currentFrameCodeAddress = new_code_address;
    }
    else
    {
        KMessageBox::error(
            0, 
            "<b>Can't get frame id</b>"
            "Could not found frame id from output of 'info frame'. "
            "Further debugging can be unreliable. ",
            i18n("Internal error"));
    }

    if (frameIdChanged)
    {
        // Remove all variables.
        // FIXME: probably, need to do this in all frames.
        QListViewItem* child = frame->firstChild();
        QListViewItem* next;
        for(; child; child = next)
        {
            next = child->nextSibling();
            delete child;
        }
    }

    setUpdatesEnabled(false);

    std::set<QListViewItem*> alive;

    for(unsigned i = 0; i < locals_and_arguments.size(); ++i)
    {
        QString name = locals_and_arguments[i];

        // See if we've got VarItem for this one already.
        VarItem* var = 0;
        for(QListViewItem *child = frame->firstChild(); 
            child; 
            child = child->nextSibling())
        {
            if (child->text(VarNameCol) == name)
            {
                var = dynamic_cast<VarItem*>(child);
                break;
            }
        }
        if (!var)
        {
            var = new VarItem(frame, name);
        }
        alive.insert(var);

        var->clearHighlight();
    }

    // Remove VarItems that don't correspond to any local
    // variables any longer. Perform type/address updates
    // for others.
    for(QListViewItem* child = frame->firstChild(); child;)
    {
        QListViewItem* current = child;
        child = current->nextSibling();
        if (!alive.count(current))
            delete current;
        else
            static_cast<VarItem*>(current)->recreateLocallyMaybe();
    }

    for(QListViewItem* child = findWatch()->firstChild();
        child; child = child->nextSibling())
    {
        VarItem* var = static_cast<VarItem*>(child);
        var->clearHighlight();
        // For watched expressions, we don't have an easy way
        // to check if their meaning is still the same, so
        // unconditionally recreate them.
        var->recreate();
    }
    
    // Note: can't use --all-values in this command, because gdb will
    // die if there's any uninitialized variable. Ouch!
    controller_->addCommand(new GDBCommand(
                                "-var-update *",
                                this,
                                &VariableTree::handleVarUpdate));

    controller_->addCommand(new SentinelCommand(
                                this,
                                &VariableTree::variablesFetchDone));
}

void VariableTree::handleVarUpdate(const GDBMI::ResultRecord& r)
{
    const GDBMI::Value& changed = r["changelist"];

    std::set<QString> names_to_update;

    for(unsigned i = 0; i < changed.size(); ++i)
    {
        const GDBMI::Value& c = changed[i];

        QString name = c["name"].literal();
        if (c.hasField("in_scope") && c["in_scope"].literal() == "false")
            continue;

        names_to_update.insert(name);
    }

    QMap<QString, VarItem*>::iterator i, e;
    for (i = varobj2varitem.begin(), e = varobj2varitem.end(); i != e; ++i)
    {
        if (names_to_update.count(i.key()) 
            || i.data()->updateUnconditionally())
        {
            i.data()->updateValue();
        }
    }
}

void VarItem::handleCliPrint(const QValueVector<QString>& lines)
{
    static QRegExp r("(\\$[0-9]+)");
    if (lines.size() >= 2)
    {
        int i = r.search(lines[1]);
        if (i == 0)
        {
            controller_->addCommand(
                new GDBCommand(QString("-var-create %1 * \"%2\"")
                               .arg(varobjName_)
                               .arg(r.cap(1)),
                               this,
                               &VarItem::varobjCreated,
                               // On initial create, errors get reported
                               // by generic code. After then, errors
                               // are swallowed by varobjCreated.
                               initialCreation_ ? false : true));            
        }
        else
        {
            // FIXME: merge all output lines together.
            // FIXME: add 'debuggerError' to debuggerpart.
            KMessageBox::error(
                0,
                i18n("<b>Debugger error</b><br>") + lines[1],
                i18n("Debugger error"));
        }
    }
}


void VariableTree::variablesFetchDone()
{
    // During parsing of fetched variable values, we might have issued
    // extra command to handle 'special values', like QString.
    // We don't want to enable updates just yet, because this will cause
    // flicker, so add a sentinel command just to enable updates.
    //
    // We need this intermediate hook because commands for special
    // representation are issues when responses to orginary fetch
    // values commands are received, so we can add sentinel command after
    // special representation fetch only when commands for ordinary
    // fetch are all executed.
    controller_->addCommand(new SentinelCommand(
                                this,
                                &VariableTree::fetchSpecialValuesDone));

}

void VariableTree::fetchSpecialValuesDone()
{
    // FIXME: can currentFrame_ or currentThread_ change between
    // start of var fetch and call of 'variablesFetchDone'?
    VarFrameRoot *frame = demand_frame_root(
        controller_->currentFrame(), controller_->currentThread());

//    frame->trim();

    frame->needLocals_ = false;

    setUpdatesEnabled(true);
    triggerUpdate();

    kdDebug(9012) << "Time to fetch variables: " << fetch_time.elapsed() << 
        "ms\n";
}

void 
VariableTree::slotItemRenamed(QListViewItem* item, int col, const QString& text)
{
    if (col == ValueCol)
    {
        VarItem* v = dynamic_cast<VarItem*>(item);
        Q_ASSERT(v);
        if (v)
        {
            v->setValue(text);
        }
    }
}


void VariableTree::keyPressEvent(QKeyEvent* e)
{
    if (VarItem* item = dynamic_cast<VarItem*>(currentItem()))
    {
        QString text = e->text();

        if (text == "n" || text == "x" || text == "d" || text == "c" 
            || text == "t")
        {
            item->setFormat(
                item->formatFromGdbModifier(text[0].latin1()));
        }

        if (e->key() == Qt::Key_Delete)
        {
            QListViewItem* root = findRoot(item);

            if (dynamic_cast<WatchRoot*>(root) || root == recentExpressions_) 
            {
                delete item;
            }
        }

        if (e->key() == Qt::Key_C && e->state() == Qt::ControlButton)
        {
            copyToClipboard(item);
        }
    }        
}


void VariableTree::copyToClipboard(QListViewItem* item)
{
    QClipboard *qb = KApplication::clipboard();
    QString text = item->text( 1 );

#if KDE_VERSION > 305
    qb->setText( text, QClipboard::Clipboard );
#else
    qb->setText( text );
#endif
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
        activePopup_->setItemEnabled(idToggleWatch, true);

        unsigned long long address = r["value"].literal().toULongLong(0, 16);
        if (breakpointWidget_->hasWatchpointForAddress(address))
        {
            activePopup_->setItemChecked(idToggleWatch, true);
        }
    }
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

TrimmableItem::TrimmableItem(VariableTree *parent)
    : KListViewItem (parent, parent->lastChild())
{
}

// **************************************************************************

TrimmableItem::TrimmableItem(TrimmableItem *parent)
    : KListViewItem (parent, parent->lastChild())
{
}

// **************************************************************************

TrimmableItem::~TrimmableItem()
{
}

// **************************************************************************

void TrimmableItem::paintCell(QPainter *p, const QColorGroup &cg,
                              int column, int width, int align)
{
    if ( !p )
        return;
    // make toplevel item (watch and frame items) names bold
    if (column == 0 && !parent())
    {
        QFont f = p->font();
        f.setBold(true);
        p->setFont(f);
    }
    QListViewItem::paintCell( p, cg, column, width, align );
}

QListViewItem *TrimmableItem::lastChild() const
{
    QListViewItem *child = firstChild();
    if (child)
        while (QListViewItem *nextChild = child->nextSibling())
            child = nextChild;

    return child;
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

int VarItem::varobjIndex = 0;

VarItem::VarItem(TrimmableItem *parent, 
                 const QString& expression,
                 bool frozen)
    : TrimmableItem (parent),
      expression_(expression),
      highlight_(false),
      oldSpecialRepresentationSet_(false),
      format_(natural),
      numChildren_(0),
      childrenFetched_(false),
      updateUnconditionally_(false),
      frozen_(frozen),
      initialCreation_(true)
{
    connect(this, SIGNAL(varobjNameChange(const QString&, const QString&)),
            varTree(), 
            SLOT(slotVarobjNameChanged(const QString&, const QString&)));


    // User might have entered format together with expression: like
    //   /x i1+i2
    // If we do nothing, it will be impossible to watch the variable in
    // different format, as we'll just add extra format specifier.
    // So:
    //   - detect initial value of format_
    //   - remove the format specifier from the string.

    static QRegExp explicit_format("^\\s*/(.)\\s*(.*)");
    if (explicit_format.search(expression_) == 0)
    {
        format_ = formatFromGdbModifier(explicit_format.cap(1)[0].latin1());
        expression_ = explicit_format.cap(2);
    }

    setText(VarNameCol, expression_);
    // Allow to change variable name by editing.
    setRenameEnabled(ValueCol, true);

    // Need to store this locally, since varTree() is 0 in
    // destructor.
    controller_ = varTree()->controller();

    createVarobj();
}

VarItem::VarItem(TrimmableItem *parent, const GDBMI::Value& varobj,
                 format_t format)
: TrimmableItem (parent),
  highlight_(false),
  oldSpecialRepresentationSet_(false),
  format_(format),
  numChildren_(0),
  childrenFetched_(false),
  updateUnconditionally_(false),
  frozen_(false),
  initialCreation_(false)
{ 
    connect(this, SIGNAL(varobjNameChange(const QString&, const QString&)),
            varTree(), 
            SLOT(slotVarobjNameChanged(const QString&, const QString&)));

    expression_ = varobj["exp"].literal();
    varobjName_ = varobj["name"].literal();

    varobjNameChange("", varobjName_);
  
    setText(VarNameCol, displayName());

    // Allow to change variable name by editing.
    setRenameEnabled(ValueCol, true);

    controller_ = varTree()->controller();

    // Set type and children.
    originalValueType_ = varobj["type"].literal();
    numChildren_ = varobj["numchild"].literal().toInt();
    setExpandable(numChildren_ != 0);


    // Get the initial value.
    updateValue();
}

void VarItem::createVarobj()
{
    QString old = varobjName_;
    varobjName_ = QString("KDEV%1").arg(varobjIndex++);
    emit varobjNameChange(old, varobjName_);

    if (frozen_)
    {
        // MI has no way to freeze a variable object. So, we
        // issue print command that returns $NN convenience
        // variable and we create variable object from that.
        controller_->addCommand(
            new CliCommand(
                QString("print %1").arg(expression_),
                this,
                &VarItem::handleCliPrint));        
    }
    else
    {
        controller_->addCommand(
            new CliCommand(
                QString("print /x &%1").arg(expression_),
                this,
                &VarItem::handleCurrentAddress));
        
        controller_->addCommand(
            // Need to quote expression, otherwise gdb won't like 
            // spaces inside it.
            new GDBCommand(QString("-var-create %1 * \"%2\"")
                           .arg(varobjName_)
                           .arg(expression_),
                           this,
                           &VarItem::varobjCreated,
                           initialCreation_ ? false : true));
    }
}

void VarItem::varobjCreated(const GDBMI::ResultRecord& r)
{
    // If we've tried to recreate varobj (for example for watched expression)
    // after step, and it's no longer valid, it's fine.
    if (r.reason == "error")
    {
        varobjName_ = "";
        return;
    }
    setEnabledRecursively(true);

    QString oldType = originalValueType_;
    originalValueType_ = r["type"].literal();
    if (!oldType.isEmpty() && oldType != originalValueType_)
    {
        // Type changed, the children might be no longer valid,
        // so delete them.
        for(QListViewItem* child = firstChild(); child; )
        {
            QListViewItem* cur = child;
            child = child->nextSibling();
            delete cur;
        }
    }
        
    if (r.hasField("exp"))
        expression_ = r["exp"].literal();
    numChildren_ = r["numchild"].literal().toInt();
    setExpandable(numChildren_ != 0);
    currentAddress_ = lastObtainedAddress_;

    setVarobjName(varobjName_);
}

void VarItem::setVarobjName(const QString& name)
{
    if (varobjName_ != name)
        emit varobjNameChange(varobjName_, name);

    varobjName_ = name;
 
    if (format_ != natural)
    {
        controller_->addCommand(
            new GDBCommand(QString("-var-set-format \"%1\" %2")
                           .arg(varobjName_).arg(varobjFormatName()),
                           NOTRUNCMD, NOTINFOCMD));
    }

    // Get the initial value.
    updateValue();

    if (isOpen())
    {
        // This regets children list.
        setOpen(true);
    }    
}

void VarItem::valueDone(const GDBMI::ResultRecord& r)
{
    if (r.reason == "done")
    {
        QString s = GDBParser::getGDBParser()->undecorateValue(
            r["value"].literal());

        if (format_ == character)
        {
            QString encoded = s;
            bool ok;
            int value = s.toInt(&ok);
            if (ok)
            {
                char c = (char)value;
                encoded += " '";
                if (isprint(c))
                    encoded += c;
                else {
                    // Try common escape characters.
                    static char *backslashed[] = {"a", "b", "f", "n", 
                                                  "r", "t", "v", "0"};
                    static char represented[] = "\a\b\f\n\r\t\v";
                    
                    const char* ix = strchr (represented, c);
                    if (ix) {
                        encoded += "\\";
                        encoded += backslashed[ix - represented];
                    }
                    else
                        encoded += "\\" + s;
                }
                encoded += "'";
                s = encoded;
            }
        }

        if (format_ == binary)
        {
            // For binary format, split the value at 4-bit boundaries
            static QRegExp r("^[01]+$");
            int i = r.search(s);
            if (i == 0)
            {
                QString split;                               
                for(unsigned i = 0; i < s.length(); ++i)
                {
                    // For string 11111, we should split it as
                    // 1 1111, not as 1111 1.

                    // 0 is past the end character
                    int distance = i - s.length(); 

                    if (distance % 4 == 0 && !split.isEmpty())
                        split.append(' ');
                    split.append(s[i]);
                }
                s = split;
            }
        }

        setText(ValueCol, s);
    }
    else
    {
        QString s = r["msg"].literal();
        // Error response.
        if (s.startsWith("Cannot access memory"))
        {
            s = "(inaccessible)";
            setExpandable(false);
        }
        else
        {
            setExpandable(numChildren_ != 0);
        }
        setText(ValueCol, s);
    }
}

void VarItem::childrenDone(const GDBMI::ResultRecord& r)
{
    const GDBMI::Value& children = r["children"];

    for (unsigned i = 0; i < children.size(); ++i)
    {
        QString exp = children[i]["exp"].literal();
        // For artificial accessibility nodes,
        // fetch their children.
        if (exp == "public" || exp == "protected" || exp == "private")
        {
            QString name = children[i]["name"].literal();
            controller_->addCommand(new GDBCommand(
                                        "-var-list-children \"" + 
                                        name + "\"",
                                        this,
                                        &VarItem::childrenDone));            
        }
        else
        {
            VarItem* existing = 0;
            for(QListViewItem* child = firstChild();
                child; child = child->nextSibling())
            {
                VarItem* v = static_cast<VarItem*>(child);
                kdDebug(9012) << "Child exp : " << v->expression_ << 
                    " new exp " << exp << "\n";

                if (v->expression_ == exp)
                {
                    existing = v;
                }
            }
            if (existing)
            {
                existing->setVarobjName(children[i]["name"].literal());
            }
            else
            {
                // Propagate format from parent.
                VarItem* v = 0;
                v = new VarItem(this, children[i], format_);
            }
        }
    }

    childrenFetched_ = true;
}

void VarItem::handleCurrentAddress(const QValueVector<QString>& lines)
{
    if (lines.count() > 1)
    {
        static QRegExp r("\\$\\d+ = ([^\n]*)");
        int i = r.search(lines[1]);
        if (i == 0)
        {
            lastObtainedAddress_ = r.cap(1);
            kdDebug(9012) << "new address " << lastObtainedAddress_ << "\n";
        }
    }
}

void VarItem::handleType(const QValueVector<QString>& lines)
{
    bool recreate = false;

    if (lastObtainedAddress_ != currentAddress_)
    {
        kdDebug(9012) << "Address changed from " << currentAddress_
                      << " to " << lastObtainedAddress_ << "\n";
        recreate = true;
    }
    else
    {    
        // FIXME: add error diagnostic.
        if (lines.count() > 1)
        {
            static QRegExp r("type = ([^\n]*)");
            int i = r.search(lines[1]);
            if (i == 0)
            {
                kdDebug(9012) << "found type: " << r.cap(1) << "\n";
                kdDebug(9012) << "original Type: " << originalValueType_ << "\n";
            
                if (r.cap(1) != originalValueType_)
                {
                    recreate = true;
                }
            }
        }
    }
    if (recreate)
    {
        this->recreate();
    }
}

QString VarItem::displayName() const
{
    if (expression_[0] != '*')
        return expression_;

    if (const VarItem* parent = 
        dynamic_cast<const VarItem*>(TrimmableItem::parent()))
    {
        return "*" + parent->displayName();
    }
    else
    {
        return expression_;
    }
}

void VarItem::setEnabledRecursively(bool enable)
{
    setEnabled(enable);
    for(QListViewItem* child = firstChild();
        child; child = child->nextSibling())
    {
        static_cast<VarItem*>(child)->setEnabledRecursively(enable);
    }
}


VarItem::~VarItem()
{
    unhookFromGdb();
}

QString VarItem::gdbExpression() const
{
    // The expression for this item can be either:
    //  - number, for array element
    //  - identifier, for member,
    //  - ***intentifier, for derefenreced pointer.
    const VarItem* parent = dynamic_cast<const VarItem*>(TrimmableItem::parent());
    
    bool ok = false;
    expression_.toInt(&ok);
    if (ok)
    {
        // Array, parent always exists.
        return parent->gdbExpression() + "[" + expression_ + "]";
    } 
    else if (expression_[0] == '*')
    {
        if (parent)
        {
            // For MI, expression_ can be "*0" (meaing
            // references 0-th element of some array).
            // So, we really need to get to the parent to computed the right
            // gdb expression.
            return "*" + parent->gdbExpression();            
        }
        else
        {
            // Parent can be null for watched expressions. In that case,
            // expression_ should be a valid C++ expression.
            return expression_;
        }         
    }
    else
    {        
        if (parent)
            return parent->gdbExpression() + "." + expression_;
        else
            return expression_;                
    }
}

// **************************************************************************


// FIXME: we have two method to set VarItem: this one
// and updateValue below. That's bad, must have just one.
void VarItem::setText(int column, const QString &data)
{
    QString strData=data;

    if (column == ValueCol) {
        QString oldValue(text(column));
        if (!oldValue.isEmpty()) // Don't highlight new items
        {
            highlight_ = (oldValue != QString(data));
        }
    }

    QListViewItem::setText(column, strData);
}

void VarItem::clearHighlight()
{
    highlight_ = false;

    for(QListViewItem* child = firstChild(); 
        child; child = child->nextSibling())
    {
        static_cast<VarItem*>(child)->clearHighlight();
    }
}

// **************************************************************************

void VarItem::updateValue()
{
    if (handleSpecialTypes())
    {
        // 1. Gdb never includes structures in output from -var-update
        // 2. Even if it did, the internal state of object can be
        //    arbitrary complex and gdb can't detect if pretty-printed
        //    value remains the same.
        // So, we need to reload value on each step.
        updateUnconditionally_ = true;
        return;
    }
    updateUnconditionally_ = false;

    controller_->addCommand(
        new GDBCommand(
            "-var-evaluate-expression \"" + varobjName_ + "\"",
            this,
            &VarItem::valueDone,
            true /* handle error */));
}

void VarItem::setValue(const QString& new_value)
{
    controller_->addCommand(
        new GDBCommand(QString("-var-assign \"%1\" %2").arg(varobjName_)
                       .arg(new_value),
                       NOTRUNCMD, NOTINFOCMD));

    // And immediately reload it from gdb, 
    // so that it's display format is the one gdb uses,
    // not the one user has typed. Otherwise, on the next
    // step, the visible value might change and be highlighted
    // as changed, which is bogus.
    updateValue();
}

void VarItem::updateSpecialRepresentation(const QString& xs)
{
    QString s(xs);
    if (s[0] == '$') 
    {
        int i = s.find('=');
        if (i != -1)
            s = s.mid(i+2);
    }
    
    // A hack to nicely display QStrings. The content of QString is unicode
    // for for ASCII only strings we get ascii character mixed with \000.
    // Remove those \000 now.
    
    // This is not very nice, becuse we're doing this unconditionally
    // and this method can be called twice: first with data that gdb sends
    // for a variable, and second after we request the string data. In theory
    // the data sent by gdb might contain \000 that should not be translated.
    //
    // What's even worse, ideally we should convert the string data from
    // gdb into a QString again, handling all other escapes and composing
    // one QChar from two characters from gdb. But to do that, we *should*
    // now if the data if generic gdb value, and result of request for string
    // data. Fixing is is for later.
    s.replace( QRegExp("\\\\000|\\\\0"), "" );

    // FIXME: for now, assume that all special representations are 
    // just strings.

    s = GDBParser::getGDBParser()->undecorateValue(s);

    setText(ValueCol, s);
    // On the first stop, when VarItem was just created,
    // don't show it in red.
    if (oldSpecialRepresentationSet_)
        highlight_ = (oldSpecialRepresentation_ != s);
    else
        highlight_ = false;

    oldSpecialRepresentationSet_ = true;
    oldSpecialRepresentation_ = s;
}

void VarItem::recreateLocallyMaybe()
{
    controller_->addCommand(
        new CliCommand(
            QString("print /x &%1").arg(expression_),
            this,
            &VarItem::handleCurrentAddress));

    controller_->addCommand(
        new CliCommand(
            QString("whatis %1").arg(expression_),
            this,
            &VarItem::handleType));            
}

void VarItem::recreate()
{
    unhookFromGdb();

    initialCreation_ = false;
    createVarobj();    
}


// **************************************************************************

void VarItem::setOpen(bool open)
{
    QListViewItem::setOpen(open);

    if (open && !childrenFetched_)
    {
        controller_->addCommand(new GDBCommand(
                                    "-var-list-children \"" + varobjName_ + "\"",
                                    this,
                                    &VarItem::childrenDone));
    }
}

bool VarItem::handleSpecialTypes()
{
    kdDebug(9012) << "handleSpecialTypes: " << originalValueType_ << "\n";
    if (originalValueType_.isEmpty())
        return false;

    static QRegExp qstring("^(const)?[ ]*QString[ ]*&?$");

    if (qstring.exactMatch(originalValueType_)) {

        VariableTree* varTree = static_cast<VariableTree*>(listView());

        QString command;
        if (varTree->controller()->qtVersion() >= 4)
            command = "print (($len=($data=%1.d).size)>0?*((char*)&$data.data[0])@($len>100?200:$len*2):\"\")";
        else
            command = "print (($len=($data=%1.d).len)>0?*((char*)&$data.unicode[0])@($len>100?200:$len*2):\"\")";

        varTree->controller()->addCommand(
            new ValueSpecialRepresentationCommand(
                this, command.arg(gdbExpression())));

        return true;
    }

    return false;
}

// **************************************************************************

VarItem::format_t VarItem::format() const
{
    return format_;
}

void VarItem::setFormat(format_t f)
{
    if (f == format_)
        return;

    format_ = f;

    if (numChildren_)
    {
        // If variable has children, change format for children.
        // - for structures, that's clearly right
        // - for arrays, that's clearly right
        // - for pointers, this can be confusing, but nobody ever wants to
        //   see the pointer in decimal!
        for(QListViewItem* child = firstChild();
            child; child = child->nextSibling())
        {
            static_cast<VarItem*>(child)->setFormat(f);
        }
    }
    else
    {
         controller_->addCommand(
            new GDBCommand(QString("-var-set-format \"%1\" %2")
                           .arg(varobjName_).arg(varobjFormatName()),
                           NOTRUNCMD, NOTINFOCMD));
        
        updateValue();
    }
}

VarItem::format_t VarItem::formatFromGdbModifier(char c) const
{
    format_t nf;
    switch(c)
    {
    case 'n': // Not quite gdb modifier, but used in our UI.
        nf = natural; break;
    case 'x':
        nf = hexadecimal; break;
    case 'd':
        nf = decimal; break;
    case 'c':
        nf = character; break;
    case 't':
        nf = binary; break;
    default:
        nf = natural; break;
    }
    return nf;
}

QString VarItem::varobjFormatName() const
{
    switch(format_)
    {
    case natural:
        return "natural";
        break;

    case hexadecimal:
        return "hexadecimal";
        break;

    case decimal:
        return "decimal";
        break;

        // Note: gdb does not support 'character' natively,
        // so we'll generate appropriate representation
        // ourselfs.
    case character:
        return "decimal";
        break;
        
    case binary:
        return "binary";
        break;
    }
    return "<undefined>";    
}


// **************************************************************************

// Overridden to highlight the changed items
void VarItem::paintCell(QPainter *p, const QColorGroup &cg,
                        int column, int width, int align)
{
    if ( !p )
        return;

    // Draw values in fixed font. For example, when there are several
    // pointer variables, it's nicer if they are aligned -- it allows
    // to easy see the diferrence between the pointers.
    if (column == ValueCol)
    {
        p->setFont(KGlobalSettings::fixedFont());
    }

    if (column == ValueCol && highlight_) {
        QColorGroup hl_cg( cg.foreground(), cg.background(), cg.light(),
                           cg.dark(), cg.mid(), red, cg.base());
        QListViewItem::paintCell( p, hl_cg, column, width, align );
    } else
        QListViewItem::paintCell( p, cg, column, width, align );
}


VariableTree* VarItem::varTree() const
{
    return static_cast<VariableTree*>(listView());
}

void VarItem::unhookFromGdb()
{
    // Unhook children first, so that child varitems are deleted
    // before parent. Strictly speaking, we can avoid calling
    // -var-delete on child varitems, but that's a bit cheesy,
    for(QListViewItem* child = firstChild();
        child; child = child->nextSibling())
    {
        static_cast<VarItem*>(child)->unhookFromGdb();
    }

    setEnabled(false);
    childrenFetched_ = false;

    emit varobjNameChange(varobjName_, "");

    if (!controller_->stateIsOn(s_dbgNotStarted) && !varobjName_.isEmpty())
    {
        controller_->addCommand(
            new GDBCommand(
                QString("-var-delete \"%1\"").arg(varobjName_),
                NOTRUNCMD, NOTINFOCMD));
    }

    varobjName_ = "";    
}

// **************************************************************************

QString VarItem::tipText() const
{
    const unsigned int maxTooltipSize = 70;
    QString tip = text( ValueCol );

    if (tip.length() > maxTooltipSize)
        tip = tip.mid(0, maxTooltipSize - 1 ) + " [...]";

    if (!tip.isEmpty())
        tip += "\n" + originalValueType_;

    return tip;
}

bool VarItem::updateUnconditionally() const
{
    return updateUnconditionally_;
}


// **************************************************************************
// **************************************************************************
// **************************************************************************

VarFrameRoot::VarFrameRoot(VariableTree *parent, int frameNo, int threadNo)
    : TrimmableItem (parent),
      needLocals_(false),
      frameNo_(frameNo),
      threadNo_(threadNo),
      currentFrameBase(-1),
      currentFrameCodeAddress(-1)
{
    setExpandable(true);
}

// **************************************************************************

VarFrameRoot::~VarFrameRoot()
{
}

void VarFrameRoot::setOpen(bool open)
{
    bool frameOpened = ( isOpen()==false && open==true );
    QListViewItem::setOpen(open);

    if (frameOpened && needLocals_)
    {
        needLocals_ = false;
        VariableTree* parent = static_cast<VariableTree*>(listView());
        parent->updateCurrentFrame();        
    }    
}

// **************************************************************************

bool VarFrameRoot::matchDetails(int frameNo, int threadNo)
{
    return frameNo == frameNo_ && threadNo == threadNo_;
}

void VarFrameRoot::setDirty()
{
    needLocals_ = true;
}

// **************************************************************************
// **************************************************************************
// **************************************************************************
// **************************************************************************

WatchRoot::WatchRoot(VariableTree *parent)
    : TrimmableItem(parent)
{
    setText(0, i18n("Watch"));
    setOpen(true);
}

// **************************************************************************

WatchRoot::~WatchRoot()
{
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

}


#include "variablewidget.moc"

