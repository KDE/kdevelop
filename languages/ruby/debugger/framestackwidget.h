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

#ifndef _FRAMESTACKWIDGET_H_
#define _FRAMESTACKWIDGET_H_

#include <qlistview.h>
#include <qstringlist.h>

#include "rdbcontroller.h"

namespace RDBDebugger
{

class FramestackWidget;


class ThreadStackItem : public QListViewItem
{
public:
    ThreadStackItem(FramestackWidget *parent, int threadNo, const QString &threadDesc);
    virtual ~ThreadStackItem();

	virtual int rtti() const { return RTTI_THREAD_STACK_ITEM; }
	
	void setOpen(bool open);
    int threadNo() { return threadNo_; }

private:
	int threadNo_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class FrameStackItem : public QListViewItem
{
public:
    FrameStackItem(ThreadStackItem * parent, int frameNo, const QString & frameDesc, const QString & frameName);
    virtual ~FrameStackItem();

	virtual int rtti() const { return RTTI_FRAME_STACK_ITEM; }

    int frameNo() { return frameNo_; }
    int threadNo() { return threadNo_; }
	QString frameName() { return frameName_; }
	
private:
    int frameNo_;
    int threadNo_;
	QString frameName_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

/**
 * @author John Birch
 */
class FramestackWidget : public QListView
{
    Q_OBJECT

public:
    FramestackWidget( QWidget *parent=0, const char *name=0, WFlags f=0 );
    virtual ~FramestackWidget();

    void clear();

    void parseRDBThreadList(char *str);
    void parseRDBBacktraceList(char *str);

    ThreadStackItem *findThread(int threadNo);
    FrameStackItem *findFrame(int frameNo, int threadNo);

    int viewedThread()
    { return viewedThread_ ? viewedThread_->threadNo() : -1; }

public slots:
    void slotSelectFrame(int frameNo, int threadNo);
    void slotSelectionChanged(QListViewItem *thisItem);

signals:
    void selectFrame(int frameNo, int threadNo, const QString& frameName);
    void frameActive(int frameNo, int threadNo, const QString& frameName);

private:
    ThreadStackItem *viewedThread_;
};

}

#endif
