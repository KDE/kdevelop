// **************************************************************************
//    begin                : Sun Aug 8 1999
//    copyright            : (C) 1999 by John Birch
//    email                : jbb@kdevelop.org
//	
//                          Adapted for ruby debugging
//                          --------------------------
//    begin                : Mon Nov 1 2004
//    copyright            : (C) 2004 by Richard Dale
//    email                : Richard_Dale@tipitina.demon.co.uk
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
#include "rdbparser.h"
#include "rdbcommand.h"

#include <kdebug.h>
#include <kpopupmenu.h>
#include <klineedit.h>
#include <kdeversion.h>

#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qhbox.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qcursor.h>
#include <klocale.h>

#include <qpoint.h>
#include <qclipboard.h>
#include <kapplication.h>

// **************************************************************************
// **************************************************************************
// **************************************************************************

namespace RDBDebugger
{

VariableWidget::VariableWidget(QWidget *parent, const char *name)
    : QWidget(parent, name)
{
    varTree_ = new VariableTree(this);
    QLabel *label = new QLabel(i18n("E&xpression to watch:"), this);

    QHBox *watchEntry = new QHBox( this );
    watchVarEditor_ = new KHistoryCombo( watchEntry, "var-to-watch editor");
    label->setBuddy(watchVarEditor_);

    QPushButton *addButton = new QPushButton(i18n("&Add"), watchEntry );
    addButton->adjustSize();
    addButton->setFixedWidth(addButton->width());

    QBoxLayout * vbox = new QVBoxLayout();
    vbox->addWidget( label );
    vbox->addWidget( watchEntry );

    QVBoxLayout *topLayout = new QVBoxLayout(this, 2);
    topLayout->addWidget(varTree_, 10);
    topLayout->addLayout( vbox );

    connect( addButton, SIGNAL(clicked()), SLOT(slotAddWatchExpression()) );
    connect( watchVarEditor_, SIGNAL(returnPressed()), SLOT(slotAddWatchExpression()) );
}


// **************************************************************************

void VariableWidget::setEnabled(bool bEnabled)
{
    QWidget::setEnabled(bEnabled);
    if (bEnabled && parentWidget() != 0) {
        varTree_->setColumnWidth(0, parentWidget()->width()/2);
	}
}
// **************************************************************************

void VariableWidget::slotAddWatchExpression()
{
    QString watchVar(watchVarEditor_->currentText());
    if (!watchVar.isEmpty()) {
        slotAddWatchExpression(watchVar);
    }
}

// **************************************************************************

void VariableWidget::slotAddWatchExpression(const QString &ident)
{
    if (!ident.isEmpty()) {
        watchVarEditor_->addToHistory(ident);
        varTree_->slotAddWatchExpression(ident);
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
      activationId_(0),
      currentThread_(-1),
	  selectedFrame_(0),
	  watchRoot_(0),
	  globalRoot_(0)
{
    setRootIsDecorated(true);
    setAllColumnsShowFocus(true);
    setColumnWidthMode(0, Manual);
    setSorting(VAR_NAME_COLUMN);
    QListView::setSelectionMode(QListView::Single);

    addColumn(i18n("Variable"), 100 );
    addColumn(i18n("Value"), 100 );
    addColumn(i18n("Type"), 100 );

    connect( this, SIGNAL(contextMenu(KListView*, QListViewItem*, const QPoint&)),
             SLOT(slotContextMenu(KListView*, QListViewItem*)) );
    
	connect( this, SIGNAL(pressed(QListViewItem*)),
             this, SLOT(slotPressed(QListViewItem*)) );

	watchRoot_ = new WatchRoot(this);
}

// **************************************************************************

VariableTree::~VariableTree()
{
}

// **************************************************************************

void VariableTree::clear()
{
    QListViewItem *sibling = firstChild();
    while (sibling != 0) {
		QListViewItem * current = sibling;
		sibling = sibling->nextSibling();
		if (current->rtti() != RTTI_WATCH_ROOT) {
			delete current;
		}
    }
	
	globalRoot_ = 0;
	selectedFrame_ = 0;
	return;
}

// **************************************************************************

void VariableTree::slotContextMenu(KListView *, QListViewItem *item)
{
    if (item == 0)
        return;

    setSelected(item, true);    // Need to select this item.

    if (item->parent() != 0) {
        KPopupMenu popup(this);
        popup.insertTitle(item->text(VAR_NAME_COLUMN));
        int idRemoveWatch = -2;
        if (item->rtti() == RTTI_WATCH_VAR_ITEM) {
            idRemoveWatch = popup.insertItem( i18n("Remove Watch Expression") );
		}

        int	idCopyToClipboard = popup.insertItem( i18n("Copy to Clipboard") );
        int res = popup.exec(QCursor::pos());

        if (res == idRemoveWatch) {
			emit removeWatchExpression(((WatchVarItem*)item)->displayId());
            delete item;
        } else if (res == idCopyToClipboard) {
            QClipboard *qb = KApplication::clipboard();
            QString text = "{ \"" + item->text( VAR_NAME_COLUMN ) + "\", " +
                            "\"" + item->text( VALUE_COLUMN ) + "\", " +
                            "\"" + item->text( VAR_TYPE_COLUMN ) + "\" }";

#if KDE_VERSION > 305
            qb->setText( text, QClipboard::Clipboard );
#else
            qb->setText( text );
#endif
        }
    }
}

/***************************************************************************/

void VariableTree::setSelected(QListViewItem * item, bool selected)
{
	// Save the last selected VarFrameRoot for slotPressed() to restore
	if (item->rtti() == RTTI_VAR_FRAME_ROOT && selected) {
		selectedFrame_ = (VarFrameRoot *) item;
	}
	
	QListView::setSelected(item, selected);
}

/***************************************************************************/

// Makes sure that only VarFrameRoot items can be selected
void VariableTree::slotPressed(QListViewItem * item)
{
	if (item == 0) {
		return;
	}
	
	while (item->rtti() == RTTI_VAR_ITEM) {
		item = item->parent();
	}
	
	if (	item->rtti() == RTTI_GLOBAL_ROOT
			|| item->rtti() == RTTI_WATCH_ROOT
			|| item->rtti() == RTTI_WATCH_VAR_ITEM )
	{
		if (selectedFrame_ != 0) {
			setSelected(selectedFrame_, true);
		}
		return;
	}
	
	if (item->rtti() == RTTI_VAR_FRAME_ROOT) {
		VarFrameRoot * frame = (VarFrameRoot*) item;
        emit selectFrame(frame->frameNo(), frame->threadNo());
	}
	
	return;
}

// **************************************************************************

void VariableTree::prune()
{
	QListViewItem *child = firstChild();

    while (child != 0) {
        QListViewItem *nextChild = child->nextSibling();

        // Only prune var frames, not the watch or global root
        if (child->rtti() == RTTI_VAR_FRAME_ROOT) {
			if (((VarFrameRoot*) child)->isActive()) {
				if (child->isOpen()) {
					((VarFrameRoot*) child)->prune();
				}
			} else {
				delete child;
			}
		}
		
        child = nextChild;
    }
}

// **************************************************************************

// The debugger has moved onto the next program pause, so invalidate 
// everything in the Variable Tree
void VariableTree::nextActivationId() 
{ 
	activationId_++; 
	globalRoot()->setActivationId();
	watchRoot()->setActivationId();
	// ..but that's only the Watch and Global roots
}
    
// **************************************************************************

// VarFrameRoot frames in the Variable Tree from the previous program pause, 
// are set active here. Notified by the Frame Stack widget when it parses the 
// backtrace from the 'where' command after a pause.
//
// After that, any frames which aren't marked as active must have gone
// out of scope and will end up pruned.
void VariableTree::slotFrameActive(int frameNo, int threadNo, const QString& frameName)
{
	VarFrameRoot * frame = findFrame(frameNo, threadNo);
	if (frameNo == 1) {
		// If the current frame 1 doesn't exist, create it
		if (frame == 0) {
			frame = new VarFrameRoot(this, frameNo, threadNo);
		}
		
		frame->setFrameName(frameName);
	}
	
	if (frame != 0 && frame->text(VAR_NAME_COLUMN) == frameName) {
		frame->setActivationId();
	}
}

// **************************************************************************

bool VariableTree::schedule()
{
    QListViewItem *	child = firstChild();
	VarFrameRoot * frame = 0;

    while (child != 0) {
        if (child->rtti() == RTTI_VAR_FRAME_ROOT) {
			frame = (VarFrameRoot *) child;
			Q_ASSERT( !frame->isWaitingForData() );
			
			if (frame->needsVariables()) {
				if (QApplication::overrideCursor() == 0) {
					QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
				}
				
				// Tell the controller to fetch the variable values
				emit selectFrame(frame->frameNo(), frame->threadNo());
				return true;
			}
		}
		
        child = child->nextSibling();
    }
	
	frame = findFrame(1, currentThread_);
	Q_ASSERT( frame != 0 );
	Q_ASSERT( !frame->needsVariables() );
		
	// All over, nothing left to fetch. 
	// Return to frame 1, and prune the inactive items
	// from the variable tree..
	QApplication::restoreOverrideCursor();
	emit selectFrame(1, currentThread_);
	prune();
	
	return false;
}

// **************************************************************************

void VariableTree::slotAddWatchExpression(const QString &watchVar)
{
    new WatchVarItem(watchRoot(), watchVar, UNKNOWN_TYPE);
	emit addWatchExpression(watchVar, true);
}


// **************************************************************************

void VariableTree::setFetchGlobals(bool fetch)
{
	emit fetchGlobals(fetch);
}

// **************************************************************************

VarFrameRoot *VariableTree::findFrame(int frameNo, int threadNo) const
{
    // frames only exist on the top level so we only need to
    // check the siblings
    QListViewItem *sibling = firstChild();
    while (sibling != 0) {
		if (	sibling->rtti() == RTTI_VAR_FRAME_ROOT
				&& ((VarFrameRoot*) sibling)->frameNo() == frameNo 
				&& ((VarFrameRoot*) sibling)->threadNo() == threadNo ) 
		{
			return (VarFrameRoot*) sibling;
		}
		
        sibling = sibling->nextSibling();
    }

    return 0;
}

// **************************************************************************

WatchRoot *VariableTree::watchRoot()
{
    return watchRoot_;
}

// **************************************************************************

GlobalRoot *VariableTree::globalRoot()
{
	if (globalRoot_ == 0) {
		globalRoot_ = new GlobalRoot(this);
	}

    return globalRoot_;
}

// **************************************************************************

// Watch variables can be added before the start of a debugging session,
// so tell the controller about any already in the tree at start.
void VariableTree::resetWatchVars()
{
	for (QListViewItem *child = watchRoot()->firstChild(); child != 0; child = child->nextSibling()) {
		((WatchVarItem*) child)->setDisplayId(-1);
        emit addWatchExpression(child->text(VAR_NAME_COLUMN), false);
	}
}

// **************************************************************************

void VariableTree::maybeTip(const QPoint &p)
{
    VarItem * item = dynamic_cast<VarItem*>( itemAt(p) );
    if (item != 0) {
        QRect r = itemRect(item);
        if (r.isValid()) {
            tip(r, item->tipText());
		}
    }
}


// **************************************************************************
// **************************************************************************
// **************************************************************************

LazyFetchItem::LazyFetchItem(VariableTree *parent)
    : KListViewItem(parent),
      activationId_(0),
	  waitingForData_(false)
{
    setActivationId();
}

// **************************************************************************

LazyFetchItem::LazyFetchItem(LazyFetchItem *parent)
    : KListViewItem(parent),
      activationId_(0),
      waitingForData_(false)
{
    setActivationId();
}

// **************************************************************************

LazyFetchItem::~LazyFetchItem()
{
}

// **************************************************************************

void LazyFetchItem::paintCell(QPainter *p, const QColorGroup &cg,
                              int column, int width, int align)
{
    if (p == 0) {
        return;
	}
	
    // make toplevel item (watch and frame items) names bold
    if (column == VAR_NAME_COLUMN && parent() == 0) {
        QFont f = p->font();
        f.setBold(true);
        p->setFont(f);
    }
	
    QListViewItem::paintCell( p, cg, column, width, align );
}

// **************************************************************************

VarItem *LazyFetchItem::findItem(const QString &name) const
{
	QListViewItem *child = firstChild();

    // Check the siblings on this branch
    while (child != 0) {
		if (child->text(VAR_NAME_COLUMN) == name) {
			return (VarItem*) child;
		}

        child = child->nextSibling();
    }

    return 0;
}

// **************************************************************************

void LazyFetchItem::prune()
{
    QListViewItem *child = firstChild();

    while (child != 0) {
        LazyFetchItem *item = (LazyFetchItem*) child;
        child = child->nextSibling();
		// Never prune a branch if we are waiting on data to arrive.
		if (!waitingForData_) {
			if (item->isActive()) {
 				item->prune();
			} else {
				delete item;
			}
        }
    }
}


// **************************************************************************
// **************************************************************************
// **************************************************************************

VarItem::VarItem(LazyFetchItem *parent, const QString &varName, DataType dataType)
    : LazyFetchItem (parent),
      cache_(QCString()),
      dataType_(dataType),
      highlight_(false)
{
    setText(VAR_NAME_COLUMN, varName);
	setSelectable(false);
	
	// Order the VarItems so that globals are first, then
	// constants, class variables, instance variables and 
	// finally local variables
	QRegExp arrayelement_re("\\[(\\d+)\\]");
	key_ = varName;
	
	if (arrayelement_re.search(varName) != -1) {
		key_.sprintf("%.6d", arrayelement_re.cap(1).toInt());
	} else if (key_.startsWith("$")) {
		key_.prepend("1001");		// Global variable
	} else if (QRegExp("^[A-Z]").search(varName) != -1) {
		key_.prepend("1002");		// Constant
	} else if (key_.startsWith("@@")) {
		key_.prepend("1003");		// Class variable
	} else if (key_.startsWith("@")) {
		key_.prepend("1004");		// Instance variable
	} else {
		key_.prepend("1005");		// Local variable or parameter
	}

//    kdDebug(9012) << " ### VarItem::VarItem *CONSTR* " << varName << endl;	
}

// **************************************************************************

VarItem::~VarItem()
{
}

QString VarItem::key(int /*column*/, bool /*ascending*/) const 
{
	return key_;
}

// **************************************************************************

// Returns the path of a ruby item. If it is an instance variable, assume
// that there is an attr_accessor method for it.
// For example, @foobar within instance obj is accessed as obj.foobar.
// But don't strip off the @ for an instance variable with no path,
// and leave a plain '@foobar' as it is.
QString VarItem::fullName() const
{
	QString itemName = text(VAR_NAME_COLUMN);
    QString vPath("");
    const VarItem *item = this;

	if (item->parent()->rtti() != RTTI_VAR_ITEM) {
		return itemName;
	}

    // This stops at the root item (FrameRoot or GlobalRoot)
	while (item->rtti() == RTTI_VAR_ITEM) {
		QString itemName = item->text(VAR_NAME_COLUMN);
		
		if (vPath.startsWith("[")) {
			// If it's a Hash or an Array, then just insert the value. As
			// in adding '[0]' to foo.bar to give foo.bar[0]
 			vPath.prepend(itemName);
		} else {
			if (vPath.isEmpty()) {
				vPath = itemName;
			} else {
 				vPath.prepend(itemName + ".");
			}
        }
		item = (VarItem*) item->parent();
    }

	// Change 'self.@foobar' to '@foobar'
	vPath.replace(QRegExp("^self\\.@"), "@");
	
	// Strip out any '@'s in the middle of a path	
	vPath.replace(QRegExp("\\.@+"), ".");		

    return vPath;
}

// **************************************************************************

void VarItem::setText(int column, const QString &data)
{
	setActivationId();
	
    if (column == VALUE_COLUMN) {
		QListViewItem::setText(VAR_TYPE_COLUMN, typeFromValue(data));
		highlight_ = (!text(VALUE_COLUMN).isEmpty() && text(VALUE_COLUMN) != data);
    }

    QListViewItem::setText(column, data);
    repaint();
}

// **************************************************************************

void VarItem::expandValue(char *buf)
{
    LazyFetchItem::stopWaitingForData();
    RDBParser::parseExpandedVariable(this, buf);
}

// **************************************************************************

QString VarItem::typeFromValue(const QString& value)
{
	QRegExp ref_re("^#<(([^:]|::)+):");
	
	if (ref_re.search(value) != -1) {
		return QString(ref_re.cap(1));
	} else if (QRegExp("^(/|%r)").search(value) != -1) {
		return QString("Regexp");
	} else if (QRegExp("^[\"'%<]").search(value) != -1) {
		return QString("String");
	} else if (QRegExp("^(\\[)|(Array \\(\\d+ element\\(s\\)\\))").search(value) != -1) {
		return QString("Array");
	} else if (QRegExp("^(\\{)|(Hash \\(\\d+ element\\(s\\)\\))").search(value) != -1) {
		return QString("Hash");
	} else if (QRegExp("^:").search(value) != -1) {
		return QString("Symbol");
	} else if (QRegExp("\\.\\.").search(value) != -1) {
		return QString("Range");
	} else if (value == "true" || value == "false") {
		return QString("Boolean");
	} else if (  QRegExp("^[-+]?[0-9_]+").exactMatch(value)
				|| QRegExp("^[-+]?(0x|0|0b|\\?)").search(value) != -1 ) 
	{
		return QString("Integer");
	} else if (QRegExp("[0-9._]+(e[-+0-9]+)?").exactMatch(value)) {
		return QString("Float");
	} else {
    	return QString("");
	}
}

// **************************************************************************

void VarItem::setOpen(bool open)
{
    QListViewItem::setOpen(open);
	
	Q_ASSERT(	dataType_ == REFERENCE_TYPE 
				|| dataType_ == ARRAY_TYPE 
				|| dataType_ == HASH_TYPE
				|| dataType_ == STRUCT_TYPE );
					
	update();
	return;
}

// **************************************************************************

void VarItem::update()
{
	if (isOpen()) {
		startWaitingForData();
		emit ((VariableTree*)listView())->expandItem(this, fullName().latin1());
	}

	return;
}

// **************************************************************************

DataType VarItem::dataType() const
{
    return dataType_;
}

// **************************************************************************

void VarItem::setDataType(DataType dataType)
{
    dataType_ = dataType;
}

// **************************************************************************

// Overridden to highlight the changed items
void VarItem::paintCell(QPainter *p, const QColorGroup &cg,
                        int column, int width, int align)
{
    if (p == 0) {
        return;
	}

    if (column == VALUE_COLUMN && highlight_) {
        QColorGroup hl_cg( cg.foreground(), cg.background(), cg.light(),
                           cg.dark(), cg.mid(), red, cg.base());
        QListViewItem::paintCell( p, hl_cg, column, width, align );
    } else {
        QListViewItem::paintCell( p, cg, column, width, align );
	}
	
	return;
}

// **************************************************************************

QString VarItem::tipText() const
{
    const unsigned int MAX_TOOLTIP_SIZE = 70;
    QString tip = text(VALUE_COLUMN);

    if (tip.length() < MAX_TOOLTIP_SIZE) {
	    return tip;
    } else {
	    return tip.mid(0, MAX_TOOLTIP_SIZE - 1) + " [...]";
	}
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

VarFrameRoot::VarFrameRoot(VariableTree *parent, int frameNo, int threadNo)
    : LazyFetchItem(parent),
      needsVariables_(true),
      frameNo_(frameNo),
      threadNo_(threadNo),
      cache_("")
{
    setExpandable(true);
}

// **************************************************************************

VarFrameRoot::~VarFrameRoot()
{
}

// **************************************************************************

void VarFrameRoot::addLocals(char *variables)
{
    cache_.append(variables);
}

// **************************************************************************

void VarFrameRoot::setLocals()
{
    RDBParser::parseVariables(this, cache_.data());
	cache_ = "";
    needsVariables_ = false;
	stopWaitingForData();
	prune();
	
	return;
}

// **************************************************************************

// Override setOpen so that we can decide what to do when we do change
// state.
void VarFrameRoot::setOpen(bool open)
{
	bool localsViewChanged = (isOpen() != open);
    QListViewItem::setOpen(open);
	
    if (localsViewChanged) {
		((VariableTree*)listView())->selectFrame(frameNo_, threadNo_);
	}

	return;
}

void VarFrameRoot::setFrameName(const QString &frameName)
{
	setText(VAR_NAME_COLUMN, frameName); 
	setText(VALUE_COLUMN, "");
	
	return; 
}

void VarFrameRoot::setActivationId() 
{ 
	LazyFetchItem::setActivationId(); 
	stopWaitingForData();
	needsVariables_ = true;
	cache_ = "";
} 
	
bool VarFrameRoot::needsVariables() const 
{ 
	return (	text(VAR_NAME_COLUMN).contains("try_initialize") == 0 
				&& isOpen() 
				&& !isWaitingForData() 
				&& needsVariables_ ); 
}

// **************************************************************************
// **************************************************************************
// **************************************************************************
// **************************************************************************


GlobalRoot::GlobalRoot(VariableTree *parent)
    : LazyFetchItem(parent)
{
    setText(0, i18n("Global"));
    setExpandable(true);
    setOpen(false);
	setSelectable(false);
}

// **************************************************************************

GlobalRoot::~GlobalRoot()
{
}

// **************************************************************************

void GlobalRoot::setGlobals(char * globals)
{
    setActivationId();
    RDBParser::parseVariables(this, globals);
	
	return;
}

// **************************************************************************

void GlobalRoot::setOpen(bool open)
{
	bool globalsViewChanged = (isOpen() != open);
	QListViewItem::setOpen(open);
	
	if (globalsViewChanged) {
    	((VariableTree*)listView())->setFetchGlobals(isOpen());
	}
	
	return;
}

// **************************************************************************
// **************************************************************************
// **************************************************************************
// **************************************************************************

WatchVarItem::WatchVarItem( LazyFetchItem *parent, const QString &varName, DataType dataType, int displayId )
    : VarItem(parent, varName, dataType),
	displayId_(displayId)
{
}

// **************************************************************************

WatchVarItem::~WatchVarItem()
{
}

// **************************************************************************

void WatchVarItem::setDisplayId(int id)
{
	displayId_ = id;
}

// **************************************************************************

int WatchVarItem::displayId()
{
	return displayId_;
}

// **************************************************************************
// **************************************************************************
// **************************************************************************
// **************************************************************************

WatchRoot::WatchRoot(VariableTree *parent)
    : LazyFetchItem(parent)
{
    setText(VAR_NAME_COLUMN, i18n("Watch"));
    setOpen(true);
	setSelectable(false);
}

// **************************************************************************

WatchRoot::~WatchRoot()
{
}

// **************************************************************************

// Sets the initial value of a new Watch item, along with the
// display id
void WatchRoot::setWatchExpression(char * buf, char * expression)
{
	QString expr(expression);
	QRegExp display_re("^(\\d+):\\s([^\n]+)\n");
	
    for (QListViewItem *child = firstChild(); child != 0; child = child->nextSibling()) {
        WatchVarItem *varItem = (WatchVarItem*) child;
		if (	varItem->text(VAR_NAME_COLUMN) == expr 
				&& varItem->displayId() == -1
				&& display_re.search(buf) >= 0 ) 
		{
			varItem->setDisplayId(display_re.cap(1).toInt());
			// Skip over the 'thing = ' part of expr to get the value
			varItem->setText(	VALUE_COLUMN, 
								display_re.cap(2).mid(varItem->text(VAR_NAME_COLUMN).length() + strlen(" = ")) );
			return;
		}
	}
}

// After a program pause, this updates the new value of a Watch item
// expr is the thing = value part of "1: a = 1", id is the display number
void WatchRoot::updateWatchExpression(int id, const QString& expr)
{
    for (QListViewItem *child = firstChild(); child != 0; child = child->nextSibling()) {
        WatchVarItem *varItem = (WatchVarItem*) child;
		if (varItem->displayId() == id) {
			Q_ASSERT( expr.startsWith(varItem->text(VAR_NAME_COLUMN)) );
			// Skip over the 'thing = ' part of expr to get the value
			varItem->setText(	VALUE_COLUMN, 
								expr.mid(varItem->text(VAR_NAME_COLUMN).length() + strlen(" = ")) );
			return;
		}
	}
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

}


#include "variablewidget.moc"

