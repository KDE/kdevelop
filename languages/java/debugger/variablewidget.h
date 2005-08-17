/***************************************************************************
                          vartree.h  -  description
                             -------------------
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

#include <klistview.h>
#include <qwidget.h>
//Added by qt3to4:
#include <Q3CString>

class KLineEdit;

namespace JAVADebugger
{

enum { VarNameCol = 0, ValueCol = 1 };
enum DataType { typeUnknown, typeValue, typePointer, typeReference,
                typeStruct, typeArray, typeQString, typeWhitespace,
                typeName };

class FrameRoot;
class WatchRoot;
class VarItem;
class VariableTree;
class DbgController;

class VariableWidget : public QWidget
{
    Q_OBJECT

public:
    VariableWidget( QWidget *parent=0, const char *name=0 );
    void clear();

    VariableTree *varTree() const
    { return varTree_; }

private slots:
    void slotAddWatchVariable();

private:
    VariableTree *varTree_;
    KLineEdit *watchVarEntry_;
    friend class VariableTree;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class VariableTree : public KListView
{
    Q_OBJECT

public:
    VariableTree( VariableWidget *parent, const char *name=0 );
    virtual ~VariableTree();

    Q3ListViewItem *lastChild() const;

    int activeFlag() const                { return activeFlag_; }
    void setActiveFlag()                  { activeFlag_++; }

    Q3ListViewItem *findRoot(Q3ListViewItem *item) const;
    FrameRoot *findFrame(int frameNo) const;
    WatchRoot *findWatch();

    // Remove items that are not active
    void trim();
    void trimExcessFrames();
    void setLocalViewState(bool localsOn, int frameNo);

signals:
    void toggleWatchpoint(const QString &varName);
    void selectFrame(int frameNo);
    void expandItem(VarItem *item);
    void expandUserItem(VarItem *item, const Q3CString &request);
    void setLocalViewState(bool localsOn);

public slots:
    void slotAddWatchVariable(const QString& watchVar);

private slots:
    void slotContextMenu(KListView *, Q3ListViewItem *item);

private:
    int activeFlag_;
    DbgController *controller;

    friend class VarItem;
    friend class WatchRoot;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class TrimmableItem : public Q3ListViewItem
{
public:
    TrimmableItem(VariableTree *parent);
    TrimmableItem(TrimmableItem *parent);

    virtual ~TrimmableItem();

    virtual void trim();
    virtual QString getName() const         { return QString(text(VarNameCol)); }
    virtual TrimmableItem *findMatch(const QString& match, DataType type) const;
    Q3ListViewItem *lastChild() const;
    int  rootActiveFlag() const;
    void setActive()                        { activeFlag_ = rootActiveFlag(); }
    bool isActive() const                   { return activeFlag_ == rootActiveFlag(); }
    QString getValue() const                { return QString(text(ValueCol)); }
    bool isTrimmable() const;
    void waitingForData ()                  { waitingForData_ = true; }

    virtual void updateValue(char */* buf */);
    virtual DataType getDataType() const;

    virtual void setCache(const Q3CString& value);
    virtual Q3CString getCache();
    virtual QString key( int column, bool ascending ) const;

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

    void setCache(const Q3CString& value);
    Q3CString getCache();

    void setOpen(bool open);
    void setText (int column, const QString& text);

private:
    void checkForRequests();
    void paintCell( QPainter *p, const QColorGroup &cg,
                    int column, int width, int align );

private:
    Q3CString  cache_;
    DataType  dataType_;
    bool      highlight_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class FrameRoot : public TrimmableItem
{
public:
    FrameRoot(VariableTree *parent, int frameNo);
    virtual ~FrameRoot();

    void setLocals(char *locals);
    void addLocal(QString name, QString type, QString value);
    void setParams(const Q3CString& params);

    void setOpen(bool open);

    int  getFrameNo() const                     { return frameNo_; }
    void setFrameName(const QString &frameName) { setText(VarNameCol, frameName); setText(ValueCol, ""); }

    bool needLocals() const                     { return needLocals_; }

private:
    bool    needLocals_;
    int     frameNo_;
    Q3CString params_;
    Q3CString locals_;
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
