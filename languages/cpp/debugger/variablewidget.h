/***************************************************************************
    begin                : Sun Aug 8 1999
    copyright            : (C) 1999 by John Birch
    email                : jbb@kdevelop.org
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

#include "gdbcontroller.h"

#include <klistview.h>
#include <kcombobox.h>
#include <qwidget.h>
#include <qtooltip.h>

class KLineEdit;

namespace GDBDebugger
{

class TrimmableItem;
class VarFrameRoot;
class WatchRoot;
class VarItem;
class VariableTree;
class DbgController;

enum { VarNameCol = 0, ValueCol = 1, VarTypeCol = 2};
enum DataType { typeUnknown, typeValue, typePointer, typeReference,
                typeStruct, typeArray, typeQString, typeWhitespace,
                typeName };

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
//    KLineEdit *watchVarEntry_;
    friend class VariableTree;

    KHistoryCombo *watchVarEditor_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class VariableTree : public KListView, public QToolTip
{
    Q_OBJECT
//rgruber: we need this to be able to emit expandItem() from within TrimmableItem
friend class TrimmableItem;

public:
    VariableTree( VariableWidget *parent, const char *name=0 );
    virtual ~VariableTree();

    QListViewItem *lastChild() const;

    int activeFlag() const                { return activeFlag_; }
    void setActiveFlag()                  { activeFlag_++; }
    void setRadix(int r)                  { iOutRadix=r; }

    QListViewItem *findRoot(QListViewItem *item) const;
    VarFrameRoot *findFrame(int frameNo, int threadNo) const;
    WatchRoot *findWatch();
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
    void expandItem(TrimmableItem *item);
    void expandUserItem(VarItem *item, const QCString &request);
    void setLocalViewState(bool localsOn);

    // jw
    void varItemConstructed(VarItem *item);

    //rgr
    void toggleRadix(QListViewItem *item);
public slots:
    void slotAddWatchVariable(const QString& watchVar);

    //rgr
    void slotToggleRadix(QListViewItem *item);

private slots:
    void slotContextMenu(KListView *, QListViewItem *item);

    // jw
    void slotDoubleClicked(QListViewItem *item, const QPoint &pos, int c);

private:
    int activeFlag_;
    int currentThread_;
    int iOutRadix;
    //DbgController *controller;

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

    virtual void trim();
    virtual QString getName() const         { return text(VarNameCol); }
    virtual TrimmableItem *findMatch(const QString& match, DataType type) const;
    QListViewItem *lastChild() const;
    int  rootActiveFlag() const;
    void setActive()                        { activeFlag_ = rootActiveFlag(); }
    bool isActive() const                   { return activeFlag_ == rootActiveFlag(); }
    QString getValue() const                { return text(ValueCol); }
    bool isTrimmable() const;
    void waitingForData ()                  { waitingForData_ = true; }

    virtual void updateValue(char */* buf */);
    virtual DataType getDataType() const;

    virtual void setCache(const QCString& value);
    virtual QCString getCache();
    virtual QString key( int column, bool ascending ) const;

    // jw
    virtual void handleDoubleClicked(const QPoint &, int ) {}

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

    QString varPath() const;
    QString fullName() const;
    DataType getDataType() const;

    void updateValue(char *data);

    // jw
    void updateType(char *data);

    void setCache(const QCString& value);
    QCString getCache();

    void setOpen(bool open);
    void setText (int column, const QString& text);

    // jw - overriden from TrimmableItem to handle renaming
    void handleDoubleClicked(const QPoint &pos, int c);

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

class VarFrameRoot : public TrimmableItem
{
public:
    VarFrameRoot(VariableTree *parent, int frameNo, int threadNo);
    virtual ~VarFrameRoot();

    void setLocals(char *locals);
    void setParams(char *params);
    void setOpen(bool open);

    void setFrameName(const QString &frameName)
                { setText(VarNameCol, frameName); setText(ValueCol, ""); }

    bool needLocals() const                     { return needLocals_; }
    bool matchDetails(int frameNo, int threadNo);

private:
    bool    needLocals_;
    int     frameNo_;
    int     threadNo_;
    QCString params_;
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

    void requestWatchVars();
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

}

#endif
