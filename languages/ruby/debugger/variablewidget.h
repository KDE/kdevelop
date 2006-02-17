/***************************************************************************
    begin                : Sun Aug 8 1999
    copyright            : (C) 1999 by John Birch
    email                : jbb@kdevelop.org
	
                          Adapted for ruby debugging
                          --------------------------
    begin                : Mon Nov 1 2004
    copyright            : (C) 2004 by Richard Dale
    email                : Richard_Dale@tipitina.demon.co.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _VARIABLEWIDGET_H_
#define _VARIABLEWIDGET_H_

#include "rdbcontroller.h"

#include <klistview.h>
#include <kcombobox.h>
#include <qwidget.h>
#include <qtooltip.h>
#include <kdebug.h>

class KLineEdit;

namespace RDBDebugger
{

class LazyFetchItem;
class VarFrameRoot;
class GlobalRoot;
class WatchRoot;
class VarItem;
class VariableTree;
class DbgController;
class Breakpoint;

enum { 
	VAR_NAME_COLUMN	= 0, 
	VALUE_COLUMN	= 1
};

enum DataType { 
	UNKNOWN_TYPE, 
	VALUE_TYPE, 
	REFERENCE_TYPE,
	ARRAY_TYPE, 
	HASH_TYPE,
	STRUCT_TYPE,
	COLOR_TYPE,
	STRING_TYPE
};		   

class VariableWidget : public QWidget
{
    Q_OBJECT

public:
    VariableWidget( QWidget *parent=0, const char *name=0 );

    VariableTree *varTree() const
    { return varTree_; }

    virtual void setEnabled(bool b);
	
	void restorePartialProjectSession(const QDomElement* el);
	void savePartialProjectSession(QDomElement* el);

protected:
    virtual void focusInEvent(QFocusEvent *e);

public slots:
    void slotAddWatchExpression();
    void slotAddWatchExpression(const QString &expr);

private:
    VariableTree *varTree_;
    KHistoryCombo *watchVarEditor_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class VariableTree : public KListView, public QToolTip
{
    Q_OBJECT
//we need this to be able to emit expandItem() from within LazyFetchItem
friend class LazyFetchItem;

public:
	VariableTree( VariableWidget *parent, const char *name=0 );
    virtual ~VariableTree();
	
	// Clear everything but the Watch frame
    void clear();

    int activationId() const                { return activationId_; }
    void nextActivationId(); 

    VarFrameRoot *findFrame(int frameNo, int threadNo) const;
	
    GlobalRoot *globalRoot();	
    WatchRoot *watchRoot();
	
    void resetWatchVars();
    void setCurrentThread(int currentThread) { currentThread_ = currentThread; }

    // Remove items that are not active
    void prune();
	
	// Look for a frame where 'needsVariables()' is true. 
	// If found, send commands to the debugger to fetch
	// the variable values.
	// Return true if a fetch has been scheduled, otherwise
	// false.
    bool schedule();
	
	// Tell the controller whether or not to fetch the
	// values of the global variables
    void setFetchGlobals(bool fetch);

	// (from QToolTip) Display a tooltip when the cursor is over an item
	virtual void maybeTip(const QPoint &);
	
	virtual void setSelected(QListViewItem * item, bool selected);

signals:
    void toggleWatchpoint(const QString &varName);
    void selectFrame(int frame, int thread);
    void expandItem(VarItem *item, const QCString &request);
    void fetchGlobals(bool fetch);
    void addWatchExpression(const QString& expr, bool execute);
    void removeWatchExpression(int displayId);

public slots:
    void slotAddWatchExpression(const QString& watchVar);
    void slotFrameActive(int frameNo, int threadNo, const QString& frameName);
    void slotPressed(QListViewItem * item);	

private slots:
    void slotContextMenu(KListView *, QListViewItem *item);

private:
    int activationId_;
    int currentThread_;
	VarFrameRoot * selectedFrame_;
	
	WatchRoot *		watchRoot_;
	GlobalRoot *	globalRoot_;

    friend class VarFrameRoot;
    friend class VarItem;
    friend class WatchRoot;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class LazyFetchItem : public KListViewItem
{
public:
    LazyFetchItem(VariableTree *parent);
    LazyFetchItem(LazyFetchItem *parent);

    virtual ~LazyFetchItem();

	virtual int rtti() const { return RTTI_LAZY_FETCH_ITEM; }
	
    virtual void prune();
    virtual VarItem *findItem(const QString& name) const;
    
	int  currentActivationId() const        { return ((VariableTree*) listView())->activationId(); }
	virtual void setActivationId()          { activationId_ = currentActivationId(); }
    bool isActive() const                   { return activationId_ == currentActivationId(); }
    
	void startWaitingForData()              { waitingForData_ = true; }
    void stopWaitingForData()               { waitingForData_ = false; }
    bool isWaitingForData() const           { return waitingForData_; }

protected:
    void paintCell( QPainter *p, const QColorGroup &cg,
                    int column, int width, int align );

private:
    int activationId_;
    bool waitingForData_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class VarItem : public LazyFetchItem
{
public:
    VarItem( LazyFetchItem *parent, const QString &varName, DataType dataType );

    virtual ~VarItem();
	
	virtual int rtti() const { return RTTI_VAR_ITEM; }
	virtual QString key(int column, bool ascending) const;

    QString fullName() const;
	
    DataType dataType() const;
	void setDataType(DataType dataType);

    void setOpen(bool open);
    void setText (int column, const QString& text);

    // Returns the text to be displayed as tooltip (the value)
    QString tipText() const;
	
	// If the item is open, fetch details via a pp command
    void update();
	
	// The details from the pp command have arrived, parse them
	// and update the UI
    void expandValue(char *data);

private:
    void paintCell( QPainter *p, const QColorGroup &cg,
                    int column, int width, int align );

private:
	QString   key_;
    QCString  cache_;
    DataType  dataType_;
    bool      highlight_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class WatchVarItem : public VarItem
{
public:
    WatchVarItem( LazyFetchItem *parent, const QString &varName, DataType dataType, int displayId = -1);

    virtual ~WatchVarItem();
	
	virtual int rtti() const { return RTTI_WATCH_VAR_ITEM; }
	
	// The id returned by rdb after a display expression is added
	void setDisplayId(int id);
	int displayId();
	
private:
    int  displayId_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class VarFrameRoot : public LazyFetchItem
{
public:
    VarFrameRoot(VariableTree *parent, int frame, int thread);
    virtual ~VarFrameRoot();
	
	virtual int rtti() const { return RTTI_VAR_FRAME_ROOT; }
	
	virtual QString key(int column, bool /*ascending*/) const {
		return QString("%1%2").arg(RTTI_VAR_FRAME_ROOT).arg(text(column));
	}

    void addLocals(char *variables);
    void setLocals();
    void setOpen(bool open);

    void setFrameName(const QString &frameName);

	virtual void setActivationId();
    bool needsVariables() const;
	
	int frameNo() { return frameNo_; }
	int threadNo() { return threadNo_; }

private:
    bool    needsVariables_;
    int     frameNo_;
    int     threadNo_;
    QCString cache_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class WatchRoot : public LazyFetchItem
{
public:
    WatchRoot(VariableTree * parent);
    virtual ~WatchRoot();
	
	virtual int rtti() const { return RTTI_WATCH_ROOT; }
	
	virtual QString key(int column, bool /*ascending*/) const {
		return QString("%1%2").arg(RTTI_WATCH_ROOT).arg(text(column));
	}

	void setWatchExpression(char * buf, char * expr);
	void updateWatchExpression(int id, const QString& expr);
	
	void restorePartialProjectSession(const QDomElement* el);
	void savePartialProjectSession(QDomElement* el);
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class GlobalRoot : public LazyFetchItem
{
public:
    GlobalRoot(VariableTree * parent);
    virtual ~GlobalRoot();
	
	virtual int rtti() const { return RTTI_GLOBAL_ROOT; }
	
	virtual QString key(int column, bool /*ascending*/) const {
		return QString("%1%2").arg(RTTI_GLOBAL_ROOT).arg(text(column));
	}
	
    void setOpen(bool open);
    void setGlobals(char * globals);
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

}

#endif
