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

class TrimmableItem;
class VarFrameRoot;
class GlobalRoot;
class WatchRoot;
class VarItem;
class VariableTree;
class DbgController;
class Breakpoint;

enum { VarNameCol = 0, ValueCol = 1, VarTypeCol = 2};
enum DataType { typeUnknown, typeValue, typeReference,
                typeArray, typeHash, typeWhitespace,
                typeName };
				
enum RttiValues { 
	RTTI_WATCH_ROOT		= 1001, 
	RTTI_GLOBAL_ROOT	= 1002, 
	RTTI_VAR_FRAME_ROOT	= 1003, 
	RTTI_TRIMMABLE_ITEM = 1004,
	RTTI_VAR_ITEM		= 1005,
	RTTI_WATCH_VAR_ITEM	= 1006
}; 
		   

class VariableWidget : public QWidget
{
    Q_OBJECT

public:
    VariableWidget( QWidget *parent=0, const char *name=0 );
    void clear();

    VariableTree *varTree() const
    { return varTree_; }

    virtual void setEnabled(bool b);

protected:
    virtual void focusInEvent(QFocusEvent *e);

public slots:
    void slotAddWatchVariable();
    void slotAddWatchVariable(const QString &ident);

private:
    VariableTree *varTree_;
    friend class VariableTree;

    KHistoryCombo *watchVarEditor_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class VariableTree : public KListView, public QToolTip
{
    Q_OBJECT
//we need this to be able to emit expandItem() from within TrimmableItem
friend class TrimmableItem;

public:
	VariableTree( VariableWidget *parent, const char *name=0 );
    virtual ~VariableTree();

    int activeFlag() const                { return activeFlag_; }
    void setActiveFlag()                  { activeFlag_++; }

    VarFrameRoot *findFrame(int frameNo, int threadNo) const;
	
    GlobalRoot *globalRoot();	
    WatchRoot *watchRoot();
	
    void resetWatchVars();
    void setCurrentThread(int currentThread)
                                        { currentThread_ = currentThread; }

    // Remove items that are not active
    void trim();
    void trimExcessFrames();
    void setLocalViewState(bool localsOn, int frameNo, int threadNo);

	// (from QToolTip) Display a tooltip when the cursor is over an item
	virtual void maybeTip(const QPoint &);

signals:
    void toggleWatchpoint(const QString &varName);
    void selectFrame(int frameNo, int threadNo);
    void expandItem(VarItem *item, const QCString &request);
    void setLocalViewState(bool localsOn);
    void addWatchVariable(const QString& expr, bool execute);
    void removeWatchVariable(int displayId);

    void varItemConstructed(VarItem *item);

public slots:
    void slotAddWatchVariable(const QString& watchVar);


private slots:
    void slotContextMenu(KListView *, QListViewItem *item);

private:
    int activeFlag_;
    int currentThread_;
	
	WatchRoot *		watchRoot_;
	GlobalRoot *	globalRoot_;

    friend class VarFrameRoot;
    friend class VarItem;
    friend class WatchRoot;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class TrimmableItem : public KListViewItem
{
public:
    TrimmableItem(VariableTree *parent);
    TrimmableItem(TrimmableItem *parent);

    virtual ~TrimmableItem();

	virtual int rtti() const { return RTTI_TRIMMABLE_ITEM; }
	
    virtual void trim();
    virtual QString name() const         { return text(VarNameCol); }
    virtual VarItem *findItemWithName(const QString& match) const;
    int  rootActiveFlag() const;
	void setActive()                        { activeFlag_ = rootActiveFlag(); }
    bool isActive() const                   { return activeFlag_ == rootActiveFlag(); }
    QString getValue() const                { return text(ValueCol); }
    bool isTrimmable() const;
    void waitingForData ()                  { waitingForData_ = true; }

    virtual void updateValue(char */* buf */);
    virtual DataType dataType() const;

    virtual void setCache(const QCString& value);
    virtual QCString cache();

protected:

    void paintCell( QPainter *p, const QColorGroup &cg,
                    int column, int width, int align );

private:
    int activeFlag_;
    bool waitingForData_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class VarItem : public TrimmableItem
{
public:
    VarItem( TrimmableItem *parent, const QString &varName, DataType dataType );

    virtual ~VarItem();
	
	virtual int rtti() const { return RTTI_VAR_ITEM; }
	virtual QString key(int column, bool ascending) const;

    QString fullName() const;
	
    DataType dataType() const;
	void setDataType(DataType dataType);

    void updateValue(char *data);

    void updateType();

    void setCache(const QCString& value);
    QCString cache();

    void setOpen(bool open);
    void setText (int column, const QString& text);

    // Returns the text to be displayed as tooltip (the value)
    QString tipText() const;

private:
    void checkForRequests();
    void paintCell( QPainter *p, const QColorGroup &cg,
                    int column, int width, int align );

private:
    QCString  cache_;
    DataType  dataType_;
    bool      highlight_;

    // the non-cast type of the variable
    QCString originalValueType_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class WatchVarItem : public VarItem
{
public:
    WatchVarItem( TrimmableItem *parent, const QString &varName, DataType dataType, int displayId = -1);

    virtual ~WatchVarItem();
	
	virtual int rtti() const { return RTTI_WATCH_VAR_ITEM; }
	
	void setDisplayId(int id);
	int displayId();
	
private:
    int  displayId_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class VarFrameRoot : public TrimmableItem
{
public:
    VarFrameRoot(VariableTree *parent, int frameNo, int threadNo);
    virtual ~VarFrameRoot();
	
	virtual int rtti() const { return RTTI_VAR_FRAME_ROOT; }
	
	virtual QString key(int column, bool /*ascending*/) const {
		return QString("%1%2").arg(RTTI_VAR_FRAME_ROOT).arg(text(column));
	}

    void addLocals(char *variables);
    void setLocals();
    void setOpen(bool open);

    void setFrameName(const QString &frameName)
                { setText(VarNameCol, frameName); setText(ValueCol, ""); }

    bool needLocals() const                     { return needLocals_; }
	
	int frameNo() { return frameNo_; }
	int threadNo() { return threadNo_; }

private:
    bool    needLocals_;
    int     frameNo_;
    int     threadNo_;
    QCString locals_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class WatchRoot : public TrimmableItem
{
public:
    WatchRoot(VariableTree *parent);
    virtual ~WatchRoot();
	
	virtual int rtti() const { return RTTI_WATCH_ROOT; }
	
	virtual QString key(int column, bool /*ascending*/) const {
		return QString("%1%2").arg(RTTI_WATCH_ROOT).arg(text(column));
	}

	void setDisplay(char * buf, char * expr);
	void updateWatchVariable(int id, const QString& expr);
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class GlobalRoot : public TrimmableItem
{
public:
    GlobalRoot(VariableTree *parent);
    virtual ~GlobalRoot();
	
	virtual int rtti() const { return RTTI_GLOBAL_ROOT; }
	
	virtual QString key(int column, bool /*ascending*/) const {
		return QString("%1%2").arg(RTTI_GLOBAL_ROOT).arg(text(column));
	}
	
    void setOpen(bool open);
    void setGlobals(char *globals);

private:
    QCString globals_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

}

#endif
