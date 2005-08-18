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

VariableWidget::VariableWidget(QWidget *parent, const char *name)
    : QWidget(parent, name), firstShow_(true)
{
    setIcon(SmallIcon("math_brace"));
    setCaption(i18n("Variable Tree"));

    varTree_ = new VariableTree(this);
    
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

// **************************************************************************

void VariableWidget::clear()
{
  // Use 'trim' so that logic about which top-level items are 
  // always retained is contained only in that function.
  varTree_->setActiveFlag();
  varTree_->trim();
}

// **************************************************************************

// When the variables view is shown the first time, 
// set the width of 'variable name' column to half the total
// width.
// Ideally, KMDI should emit 'initial size set' signal, but
// it does not, so we rely on the fact that size is already
// set when the widget is first shown.
void VariableWidget::showEvent(QShowEvent *)
{
    if (firstShow_)
    {
        firstShow_ = false;
        varTree_->setColumnWidth(0, width()/2);
    }
}

// **************************************************************************

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

VariableTree::VariableTree(VariableWidget *parent, const char *name)
    : KListView(parent, name),
      QToolTip( viewport() ),
      activeFlag_(0),
      currentThread_(-1),
      justPaused_(false),
      recentExpressions_(0)
{
    setRootIsDecorated(true);
    setAllColumnsShowFocus(true);
    setColumnWidthMode(0, Manual);
    setSorting(-1);
    QListView::setSelectionMode(QListView::Single);

    addColumn(i18n("Variable"), 100 );
    addColumn(i18n("Value"), 100 );
    addColumn(i18n("Type"), 100 );

    connect( this, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
             SLOT(slotContextMenu(KListView*, QListViewItem*)) );
    connect( this, SIGNAL(toggleRadix(QListViewItem*)), SLOT(slotToggleRadix(QListViewItem*)) );
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
        popup.insertTitle(item->text(VarNameCol));
        int idRemove = -2;
        int idReevaluate = -2;
        QListViewItem* root = findRoot(item);
        if (dynamic_cast<WatchRoot*>(root)) {
            idRemove = popup.insertItem( 
                SmallIcon("editdelete"), i18n("Remove Watch Variable") );
        }
        if (root == recentExpressions_) {
            idReevaluate = popup.insertItem( 
                SmallIcon("reload"), i18n("Reevaluate Expression") );
            idRemove = popup.insertItem( 
                SmallIcon("editdelete"), i18n("Remove Expression") );
        }

        int idToggleWatch = popup.insertItem( i18n("Toggle Watchpoint") );
        int idToggleRadix = popup.insertItem( i18n("Toggle Hex/Decimal") );
        int	idCopyToClipboard = popup.insertItem( 
            SmallIcon("editcopy"), i18n("Copy to Clipboard") );
        int res = popup.exec(QCursor::pos());

        if (res == idRemove)
            delete item;
        if (res == idToggleRadix)
            emit toggleRadix(item);
        else if (res == idCopyToClipboard)
        {
            QClipboard *qb = KApplication::clipboard();
            QString text = "{ \"" + item->text( 0 ) + "\", " + // name
                            "\"" + item->text( 2 ) + "\", " + // type
                            "\"" + item->text( 1 ) + "\" }";  // value

#if KDE_VERSION > 305
            qb->setText( text, QClipboard::Clipboard );
#else
            qb->setText( text );
#endif
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
                emit expandItem(item);
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
            for(QListViewItemIterator it(item); *it; ++it)
            {
                VarItem* var = dynamic_cast<VarItem*>(*it);                    
                Q_ASSERT(var && 
                         "only VarItem allowed under 'Recent expressions'");
                emit expandItem(var);
            }
        }
    }
}

// **************************************************************************

void VariableTree::slotAddWatchVariable(const QString &watchVar)
{
    kdDebug(9012) << "Add watch variable: " << watchVar << endl;
    VarItem *varItem = new VarItem(findWatch(), watchVar, typeUnknown);
    emit expandItem(varItem);
}

void VariableTree::slotEvaluateExpression(const QString &expression)
{
    if (recentExpressions_ == 0)
    {
        recentExpressions_ = new TrimmableItem(this);
        recentExpressions_->setText(0, "Recent");
        recentExpressions_->setOpen(true);
    }

    VarItem *varItem = new VarItem(recentExpressions_, expression, typeUnknown);
    varItem->setRenameEnabled(0, 1);
    emit expandItem(varItem);
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

void VariableTree::trim()
{
    QListViewItem *child = firstChild();

    while (child) {
        QListViewItem *nextChild = child->nextSibling();

        // don't trim the watch root, or 'recent expressions' root.
        if (!(dynamic_cast<WatchRoot*> (child)) 
            && child != recentExpressions_) {
            if (TrimmableItem *item = dynamic_cast<TrimmableItem*> (child)) {
                if (item->isActive())
                    item->trim();
                else
                    delete item;
            }
        }
        child = nextChild;
    }
}

// **************************************************************************

void VariableTree::trimExcessFrames()
{
    viewport()->setUpdatesEnabled(false);
    QListViewItem *child = firstChild();

    while (child) {
        QListViewItem *nextChild = child->nextSibling();
        if (VarFrameRoot *frame = dynamic_cast<VarFrameRoot*> (child)) {
            if (!frame->matchDetails(0, currentThread_))
                delete frame;
        }
        child = nextChild;
    }
    viewport()->setUpdatesEnabled(true);
    repaint();
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
    kdDebug(9012) << "ToolTip::maybeTip()" << endl;

    VarItem * item = dynamic_cast<VarItem*>( itemAt( p ) );
    if ( item )
    {
        QRect r = itemRect( item );
        if ( r.isValid() )
            tip( r, item->tipText() );
    }
}

/* rgruber:
 * this it the slot which is connected to the toggleRadix() signal
 * it removes the given watch variable an replaces it by another
 * watch that includes a format modifier
 */
void VariableTree::slotToggleRadix(QListViewItem * item)
{
  if (item==NULL)  //no item->nothing to do
    return;

  VarItem *pOldItem = dynamic_cast<VarItem*>(item);
  VarItem *pNewItem = NULL;

  QString strName = pOldItem->text(VarNameCol);

  QString strTmp = strName.left(3).lower();
  if (iOutRadix == 10) {
      if (strTmp == "/d ")   //is there a wrong format modifier...
          strName = "/x "+strName.right(strName.length()-3);  //...replace the modifier
      else if (strTmp == "/x ")
          strName = strName.right(strName.length()-3);  //stripe the modifier
      else
          strName = QString("/x ")+strName;  //add the hex-formater
  } else
  if (iOutRadix == 16) {
      if (strTmp == "/x ")   //is there a wrong format modifier...
          strName = "/d "+strName.right(strName.length()-3);  //...replace the modifier
      else if (strTmp == "/d ")   //is there a format modifier?
          strName = strName.right(strName.length()-3);  //stripe the modifier
      else
          strName = QString("/d ")+strName;  //add the dec-formater
  }

  pNewItem = new VarItem((TrimmableItem *) item->parent(), strName, typeUnknown);
  emit expandItem(pNewItem);

  pNewItem->moveItem(pOldItem);  //move the new item up right under the old one

  delete item;  //remove the old one so that is seam as if it was replaced by the new item
  pOldItem=NULL;
}

void VariableTree::slotDbgStatus(const QString&, int statusFlag)
{
    if (statusFlag & s_appNotStarted)
    {
        // The application no longer exists. Remove all locals.
        setActiveFlag();

        // Now wipe the tree out
        viewport()->setUpdatesEnabled(false);
        trim();
        setUpdatesEnabled(true);
        repaint();
    }
    else
    {
        // Application still exists.
        if (!(statusFlag & s_appBusy))
        {
            // But is not busy. This means application has just stopped for
            // some reason. Need to refresh locals when
            // slotChangedFrame is called. Cannot do it here, because
            // we don't know which thread we're in.
            justPaused_ = true;
        }
    }
}

VarFrameRoot* VariableTree::demand_frame_root(int frameNo, int threadNo)
{
    VarFrameRoot *frame = findFrame(frameNo, threadNo); 
    if (!frame)
    {
        frame = new VarFrameRoot(this, frameNo, threadNo);
        frame->setFrameName("Locals");
        // Make sure "Locals" item is always the top item, before
        // "watch" and "recent experessions" items.
       this->takeItem(frame);
       this->insertItem(frame);
    }
    return frame;
}

void VariableTree::slotParametersReady(const char* data)
{
    // The locals are always attached to the currentFrame
    VarFrameRoot *frame = demand_frame_root(currentFrame_, currentThread_);
    frame->setParams(data);
}

void VariableTree::slotLocalsReady(const char* data)
{
    setUpdatesEnabled(false);

    VarFrameRoot *frame = demand_frame_root(currentFrame_, currentThread_);
    frame->setLocals(data);
    frame->setOpen(true);
    
    // If we're regetting locals for the frame 0, it surely means
    // the application was just paused. Otherwise, 
    // (say after selecting frame 1 and then frame 0) we'd have locals
    // for frame 0 already. If app was just paused, then other frames
    // are out-of-date, and we trim them. If user later selects frame 1,
    // we get locals for that frame.
    // TODO: should we reset data for other threads?
    if (currentFrame_ == 0 || currentThread_ == -1)
        trim();
    else 
       frame->trim();

    setUpdatesEnabled(true);
    triggerUpdate();
}

void VariableTree::slotCurrentFrame(int frameNo, int threadNo)
{
    // It's quite likely that frameNo == currentFrame_ and
    // threadNo == currentThread_. For example, this happens
    // when the 'step' command is executed.
    if (frameNo != currentFrame_ || threadNo != currentThread_)
    {
        // Hide the current frame vars root.
        demand_frame_root(currentFrame_, currentThread_)->setVisible(false);
        
        currentFrame_ = frameNo;
        currentThread_ = threadNo;
    }

    // Show the current frame.
    VarFrameRoot* frame = demand_frame_root(currentFrame_, currentThread_);
    frame->setVisible(true);
        
    // If no locals for frame were obtained, reget the local.
    // Also reget the locals if the program was just paused. In that
    // case we're always on frame 0, and the setLocals function
    // we eventually remove frames 1, N, if they are present. They
    // will be repopulated if needed.
    if (frame->needLocals() || justPaused_) 
    {
        setActiveFlag();
        // This will eventually call back to slotParametersReady and 
        // slotLocalsReady
        emit produceVariablesInfo();

        if (justPaused_)
        {
            findWatch()->requestWatchVars();
        }
        justPaused_ = false;
    }
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
            // Set the value
            emit setValue(v->gdbExpression(), text);
            // And immediately reload it from gdb, 
            // so that it's display format is the one gdb uses,
            // not the one user has typed. Otherwise, on the next
            // step, the visible value might change and be highlighted
            // as changed, which is bogus.
            emit expandItem(v);
        }
    }
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

TrimmableItem::TrimmableItem(VariableTree *parent)
    : KListViewItem (parent, parent->lastChild()),
      activeFlag_(0)
{
    setActive();
}

// **************************************************************************

TrimmableItem::TrimmableItem(TrimmableItem *parent)
    : KListViewItem (parent, parent->lastChild()),
      activeFlag_(0),
      waitingForData_(false)
{
    setActive();
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

// **************************************************************************

int TrimmableItem::rootActiveFlag() const
{
    return ((VariableTree*)listView())->activeFlag();
}

// **************************************************************************

bool TrimmableItem::isTrimmable() const
{
    return !waitingForData_;
}

// **************************************************************************

QListViewItem *TrimmableItem::lastChild() const
{
    QListViewItem *child = firstChild();
    if (child)
        while (QListViewItem *nextChild = child->nextSibling())
            child = nextChild;

    return child;
}

// **************************************************************************

TrimmableItem *TrimmableItem::findMatch(const QString &match, DataType type) const
{
    QListViewItem *child = firstChild();
    bool bRenew=false;  //this indicates if the current item needs to be replaced by a new one.
   			//the problem is, that the debugger always replaces already
			//format-modified local item with non-mofified ones. So with every
			//run we need to newly modify the outcome of the debugger

    int iOutRad = ((VariableTree*)listView())->iOutRadix; //local copy of the output radix

    // Check the siblings on this branch
    while (child) {
        QString strMatch = child->text(VarNameCol);
        bRenew=false;
	if (strMatch.left(3) == "/x " || strMatch.left(3) == "/d ") {  //is the current item format modified?
	    strMatch = strMatch.right(strMatch.length()-3);
	    bRenew=true;
	}
	if (strMatch == match) {
            if (TrimmableItem *item = dynamic_cast<TrimmableItem*> (child))
                if ( item->getDataType() == type ||
		     ( iOutRad==16 && item->getDataType() == typeValue ) ||
		     ( iOutRad==10 && item->getDataType() == typePointer ) ) {
		    if (bRenew && dynamic_cast<VarItem*>(item)) { //do we need to replace?
			VarItem* pNewItem = new VarItem((TrimmableItem *) item->parent(),
				 child->text(VarNameCol), typeUnknown);
			emit ((VariableTree*)pNewItem->listView())->expandItem(pNewItem);
			pNewItem->moveItem(item);
			delete item;
			item=NULL;
			item=pNewItem;
		    }
		    return item;
		}
        }

        child = child->nextSibling();
    }

    return 0;
}

// **************************************************************************

void TrimmableItem::trim()
{
    QListViewItem *child = firstChild();

    while (child) {
        QListViewItem *nextChild = child->nextSibling();
        if (TrimmableItem *item = dynamic_cast<TrimmableItem*>(child)) {
            // Never trim a branch if we are waiting on data to arrive.
            if (isTrimmable()) {
                if (item->isActive())
                    item->trim();      // recurse
                else
                    delete item;
            }
        }
        child = nextChild;
    }
}

// **************************************************************************

DataType TrimmableItem::getDataType() const
{
    return typeUnknown;
}

// **************************************************************************

void TrimmableItem::setCache(const QCString&)
{
    Q_ASSERT(false);
}

// **************************************************************************

QCString TrimmableItem::getCache()
{
    Q_ASSERT(false);
    return QCString();
}

// **************************************************************************

void TrimmableItem::updateValue(char* /* buf */)
{
    waitingForData_ = false;
}

// **************************************************************************

QString TrimmableItem::key (int, bool) const
{
    return QString::null;
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

VarItem::VarItem(TrimmableItem *parent, const QString &varName, DataType dataType)
    : TrimmableItem (parent),
      name_(varName),
      cache_(QCString()),
      dataType_(dataType),
      highlight_(false)
{
    setText(VarNameCol, varName);
    // Allow to change variable name by editing.
    setRenameEnabled(ValueCol, true);

    kdDebug(9012) << " ### VarItem::VarItem *CONSTR*" << endl;
    emit ((VariableTree*)listView())->varItemConstructed(this);
}

// **************************************************************************

VarItem::~VarItem()
{
}

// **************************************************************************

QString VarItem::gdbExpression() const
{
    QString vPath("");
    for(const VarItem* item = this; 
        item; 
        item = dynamic_cast<const VarItem*>(item->parent()))
    {
        // Children of array item have array names in them,
        // e.g. "p[0]", so when visiting parent we don't need to
        // add parent name. However, when 'gdbExpression' is called
        // on array itself, we do need the name.
        if (item->getDataType() == typeArray && item != this)
            continue;

        // VP, 2005/07/19: I don't know the reason for this
        // check. But retaining to avoid breaking anything.
        if ((item->text(VarNameCol))[0] != '<') {
            QString itemName = item->name_;
            if (vPath.isEmpty())
                vPath = itemName.replace(QRegExp("^static "), "");
            else
                vPath = itemName.replace(QRegExp("^static "), "") 
                    + "." + vPath;
        }
    }

    if (isOpen() && dataType_ == typePointer)
        // We're currently showing pointed-to value        
        return "*" + vPath;
    else
        return vPath;
}

// **************************************************************************

void VarItem::setText(int column, const QString &data)
{
    QString strData=data;

    if (!isActive() && isOpen() && dataType_ == typePointer) {
        waitingForData();
        ((VariableTree*)listView())->expandItem(this);
    }

    setActive();
    if (column == ValueCol) {
        QString oldValue(text(column));
        if (!oldValue.isEmpty())                   // Don't highlight new items
            highlight_ = (oldValue != QString(data));
    }

    QListViewItem::setText(column, strData);
    repaint();
}

// **************************************************************************

void VarItem::updateValue(char *buf)
{
    TrimmableItem::updateValue(buf);

    // Hack due to my bad QString implementation - this just tidies up the display
    if ((strncmp(buf, "There is no member named len.", 29) == 0) ||
        (strncmp(buf, "There is no member or method named len.", 39) == 0))
        return;

    if (strncmp(buf, "Cannot access memory at address", 31) == 0 &&
        dataType_ == typePointer &&  //only if it is a pointer...
        ((VariableTree*)listView())->iOutRadix == 16) { //...and only do if outputradix is set to hex
	dataType_ = typeValue;
	((VariableTree*)listView())->expandItem(this);
	return;
    }

    if (*buf == '$') {
        if (char *end = strchr(buf, '='))
            buf = end+2;
    }

    if (dataType_ == typeUnknown) {
        dataType_ = GDBParser::getGDBParser()->determineType(buf);

        // Try fixing a format string here by overriding the dataType calculated
        // from this data
        QString varName = getName();
        if (dataType_ == typePointer && varName[0] == '/')
            dataType_ = typeValue;
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
    QCString r(buf);
    r.replace( QRegExp("\\\\000|\\\\0"), "" );

    GDBParser::getGDBParser()->parseValue(this, r.data());
    setActive();
}

// **************************************************************************

void VarItem::updateType(char *buf)
{
    kdDebug(9012) << " ### VarItem::updateType " << buf << endl;

    QString str(buf);
    int eq = str.find('=');
    if (eq < 0)
        return;
    str.replace(QRegExp("[\n\r]"),"");
    str = str.mid(eq + 1, 0xffff).stripWhiteSpace();

    originalValueType_ = str.latin1();

    setText(VarTypeCol, str);
    handleSpecialTypes();    
}

// **************************************************************************

void VarItem::setCache(const QCString &value)
{
    cache_ = value;
    setExpandable(true);
    handleSpecialTypes();
    if (isOpen())
        setOpen(true);
    repaint();
    setActive();
}

// **************************************************************************

void VarItem::setOpen(bool open)
{
    QListViewItem::setOpen(open);

    if (open) {
        // Opening an item can be potentially expensive, so cache value
        // received from gdb. If we have value already, just parse it,
        // don't issue another request.
        if (cache_) {
            QCString value = cache_;
            cache_ = QCString();
            GDBParser::getGDBParser()->parseCompositeValue(this, value.data());
            handleSpecialTypes();
            trim();
        } else {
            if (dataType_ == typePointer || dataType_ == typeReference) {
                waitingForData();
                emit ((VariableTree*)listView())->expandItem(this);
            }
        }
    } else {
        // Closing item. For pointer/references, it means we switch from
        // display the pointer-to value to displaying the pointer itself.
        if (dataType_ == typePointer || dataType_ == typeReference) {
            waitingForData();
            emit ((VariableTree*)listView())->expandItem(this);
        }
    }
}

// **************************************************************************

QCString VarItem::getCache()
{
    return cache_;
}

// **************************************************************************

/* This function is called in two cases -- when the type of variable first
   becomes known.in two cases. First is when the type of variable
   first becomes known. Second is when value has changed.

   The method looks at the type, and if necessary, issues an additional gdb
   requests. 
*/
void VarItem::handleSpecialTypes()
{
    if (originalValueType_.isEmpty())
        return;

    QString type = originalValueType_;
    if (dataType_ == typeReference)
    {
        // Disable the special processing for non-opened
        // references. Not sure it's the best thing, but
        // previous code worked like this.
        if (!isOpen())
            return;

        static QRegExp strip_reference("(.*)[ ]*&");
        if (strip_reference.exactMatch(type))
        {
            type = strip_reference.cap(1);
        }
    }
    if (dataType_ == typePointer)
    {
        if (!isOpen())
            return;

        static QRegExp strip_pointer("(.*)[ ]*\\*");
        if (strip_pointer.exactMatch(type))
        {
            type = strip_pointer.cap(1);
        }
    }

    static QRegExp qstring("^(const)?[ ]*QString[ ]*$");

    if (qstring.exactMatch(type)) {
        waitingForData();
        emit ((VariableTree*)listView())->expandUserItem(
            this,
            QCString().sprintf("(($len=($data=%s.d).len)?*((char*)&$data.unicode[0])@($len>100?200:$len*2):\"\")",
                               gdbExpression().latin1()));
    }
}

// **************************************************************************

DataType VarItem::getDataType() const
{
    return dataType_;
}

// **************************************************************************

// Overridden to highlight the changed items
void VarItem::paintCell(QPainter *p, const QColorGroup &cg,
                        int column, int width, int align)
{
    if ( !p )
        return;

    if (column == ValueCol && highlight_) {
        QColorGroup hl_cg( cg.foreground(), cg.background(), cg.light(),
                           cg.dark(), cg.mid(), red, cg.base());
        QListViewItem::paintCell( p, hl_cg, column, width, align );
    } else
        QListViewItem::paintCell( p, cg, column, width, align );
}

// **************************************************************************

QString VarItem::tipText() const
{
    const unsigned int maxTooltipSize = 70;
    QString tip = text( ValueCol );

    if (tip.length() < maxTooltipSize )
	    return tip;
    else
	    return tip.mid( 0, maxTooltipSize - 1 ) + " [...]";
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

VarFrameRoot::VarFrameRoot(VariableTree *parent, int frameNo, int threadNo)
    : TrimmableItem (parent),
      needLocals_(true),
      frameNo_(frameNo),
      threadNo_(threadNo),
      params_(QCString()),
      locals_(QCString())
{
    setExpandable(true);
}

// **************************************************************************

VarFrameRoot::~VarFrameRoot()
{
}

// **************************************************************************

void VarFrameRoot::setParams(const char *params)
{
    setActive();
    params_ = params;
}

// **************************************************************************

void VarFrameRoot::setLocals(const char *locals)
{
    setActive();

    // "No symbol table info available" or "No locals."
    bool noLocals = (locals &&  (strncmp(locals, "No ", 3) == 0));
    setExpandable(!params_.isEmpty() || !noLocals);

    if (noLocals) {
        locals_ = "";
        if (locals)
            if (char *end = strchr(locals, '\n'))
                *end = 0;      // clobber the new line
    } else
        locals_ = locals;

    if (!isExpandable() && noLocals)
        setText( ValueCol, locals );

    needLocals_ = false;
    if (isOpen())
        setOpen(true);
}

// **************************************************************************

// Override setOpen so that we can decide what to do when we do change
// state. This
void VarFrameRoot::setOpen(bool open)
{
    QListViewItem::setOpen(open);

    if (!open)
        return;

    if (!params_.isNull())
        GDBParser::getGDBParser()->parseCompositeValue(this, params_.data());
    if (!locals_.isNull())
        GDBParser::getGDBParser()->parseCompositeValue(this, locals_.data());

    locals_ = QCString();
    params_ = QCString();
}

// **************************************************************************

bool VarFrameRoot::matchDetails(int frameNo, int threadNo)
{
    return frameNo == frameNo_ && threadNo == threadNo_;
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

void WatchRoot::requestWatchVars()
{
    for (QListViewItem *child = firstChild(); child; child = child->nextSibling())
        if (VarItem *varItem = dynamic_cast<VarItem*>(child))
            emit ((VariableTree*)listView())->expandItem(varItem);
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

}


#include "variablewidget.moc"

