/***************************************************************************
    begin                : Sun Aug 8 1999                                           
    copyright            : (C) 1999 by John Birch
    email                : jb.nz@writeme.com
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

enum { VarNameCol = 0, ValueCol = 1 };
enum DataType { typeUnknown, typeValue, typePointer, typeReference,
                typeStruct, typeArray, typeQString, typeWhitespace,
                typeName };

class VarFrameRoot;
class WatchRoot;
class VarItem;
class VariableTree;
class DbgController;
class KLineEdit;


class VariableWidget : public QWidget
{
    Q_OBJECT

public:
    VariableWidget( QWidget *parent=0, const char *name=0 );
    void clear();

    VariableTree *varTree() const
    { return varTree_; }

    virtual void setEnabled(bool b);

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
    
    QListViewItem *lastChild() const;
    
    int activeFlag() const                { return activeFlag_; }
    void setActiveFlag()                  { activeFlag_++; }
    
    QListViewItem *findRoot(QListViewItem *item) const;
    VarFrameRoot *findFrame(int frameNo, int threadNo) const;
    WatchRoot *findWatch();
    void setCurrentThread(int currentThread)     { currentThread_ = currentThread; }

    // Remove items that are not active
    void trim();
    void trimExcessFrames();
    void setLocalViewState(bool localsOn, int frameNo, int threadNo);

signals:
    void toggleWatchpoint(const QString &varName);
    void selectFrame(int frameNo, int threadNo);
    void expandItem(VarItem *item);
    void expandUserItem(VarItem *item, const QCString &request);
    void setLocalViewState(bool localsOn);
    
public slots:
    void slotAddWatchVariable(const QString& watchVar);
    
private slots:
    void slotContextMenu(KListView *, QListViewItem *item);

private:
    int activeFlag_;
    int currentThread_;
    //DbgController *controller;

    friend class VarItem;
    friend class WatchRoot;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class TrimmableItem : public QListViewItem
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
    
    void setCache(const QCString& value);
    QCString getCache();
    
    void setOpen(bool open);
    void setText (int column, const QString& text);
    
private:
    void checkForRequests();
    void paintCell( QPainter *p, const QColorGroup &cg,
                    int column, int width, int align );
    
private:
    QCString  cache_;
    DataType  dataType_;
    bool      highlight_;
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
    void setParams(const QCString& params);
    
    void setOpen(bool open);
    
    void setFrameName(const QString &frameName) { setText(VarNameCol, frameName); setText(ValueCol, ""); }
    
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

#endif
