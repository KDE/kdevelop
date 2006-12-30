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

#ifndef _FRAMESTACKWIDGET_H_
#define _FRAMESTACKWIDGET_H_

#include <qlistview.h>
#include <qstringlist.h>

#include "gdbcontroller.h"
#include "mi/miparser.h"

#include <vector>

namespace GDBDebugger
{

class FramestackWidget;


class ThreadStackItem : public QListViewItem
{
public:
    ThreadStackItem(FramestackWidget *parent, 
                    unsigned threadNo);
    virtual ~ThreadStackItem();

    void setOpen(bool open);
    QListViewItem *lastChild() const;

    void paintCell(QPainter * p, const QColorGroup & cg, 
                   int column, int width, int align );

    int threadNo()
    { return threadNo_; }

private:
    int threadNo_;
    QString savedFunc_;
    QString savedSource_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class FrameStackItem : public QListViewItem
{
public:
    FrameStackItem(FramestackWidget *parent, 
                   unsigned frameNo,
                   const QString &name);
    FrameStackItem(ThreadStackItem *parent, 
                   unsigned frameNo,
                   const QString &name);
    virtual ~FrameStackItem();

    void setOpen(bool open);
    QListViewItem *lastChild() const;

    void paintCell(QPainter * p, const QColorGroup & cg, 
                   int column, int width, int align );

    int frameNo()
    { return frameNo_; }
    int threadNo()
    { return threadNo_; }
private:
    int frameNo_;
    int threadNo_;
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
    FramestackWidget( GDBController* controller,
                      QWidget *parent=0,                       
                      const char *name=0, WFlags f=0 );
    virtual ~FramestackWidget();

    QListViewItem *lastChild() const;
   
    ThreadStackItem *findThread(int threadNo);
    FrameStackItem *findFrame(int frameNo, int threadNo);

    int viewedThread()
    { return viewedThread_ ? viewedThread_->threadNo() : -1; }

protected:

    void drawContentsOffset( QPainter * p, int ox, int oy,
                             int cx, int cy, int cw, int ch );



private:
    /** Given gdb's 'frame' information, compute decent
        textual representation for display.

        The function is used both for frames and threads.
    */
    void formatFrame(const GDBMI::Value& frame,
                     QString& func_column,
                     QString& source_column);

    /** Cause gdb to produce backtrace for the current thread. 

        GDB reply will be route to parseArg and parseGDBBacktraceList,
        and will show up under viewedThread_ (if there are threads), or
        on top-level.
    */
    void getBacktrace(int min_frame = 0, int max_frame = frameChunk_);

    /** Obtains backtrace for the specified thread without chaning the current
        thread in gdb.

        Switches viewedThread_ to the specified thread, switches gdb thread,
        call getBacktrace(), and switches the current thread back.
    */        
    void getBacktraceForThread(int threadNo);
    friend class ThreadStackItem;


    void handleThreadList(const GDBMI::ResultRecord&);
    void handleThread(const GDBMI::ResultRecord&);
    void parseGDBBacktraceList(const GDBMI::ResultRecord&);
    void handleStackDepth(const GDBMI::ResultRecord& r);

public slots:
    void slotEvent(GDBController::event_t e);
    void slotSelectionChanged(QListViewItem *thisItem);

#if QT_VERSION < 300
private:
  QListViewItem* findItemWhichBeginsWith(const QString& text) const;
#endif

private:

    void clear();

private:

    ThreadStackItem *viewedThread_;
    int currentFrame_;
    GDBController* controller_;

    // Data to pass from 'getBacktrace' to 'handleStackDepth'
    int minFrame_;
    int maxFrame_;
    bool has_more_frames;

    friend class FrameStackItem;

    static const int frameChunk_ = 5;
};

}

#endif
