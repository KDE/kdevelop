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

namespace GDBDebugger
{

class FramestackWidget;


class ThreadStackItem : public QListViewItem
{
public:
    ThreadStackItem(FramestackWidget *parent, const QString &threadDesc);
    virtual ~ThreadStackItem();

    void setOpen(bool open);
    QListViewItem *lastChild() const;

    int threadNo()
    { return threadNo_; }

private:
  int threadNo_;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

class FrameStackItem : public QListViewItem
{
public:
    FrameStackItem(FramestackWidget *parent, const QString &frameDesc);
    FrameStackItem(ThreadStackItem *parent, const QString &frameDesc);
    virtual ~FrameStackItem();

    void setOpen(bool open);
    QListViewItem *lastChild() const;

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
    FramestackWidget( QWidget *parent=0, const char *name=0, WFlags f=0 );
    virtual ~FramestackWidget();

    QListViewItem *lastChild() const;
    void clear();

    void parseGDBThreadList(char *str);
    void parseGDBBacktraceList(char *str);

    ThreadStackItem *findThread(int threadNo);
    FrameStackItem *findFrame(int frameNo, int threadNo);

    QString getFrameName(int frameNo, int threadNo);

    int viewedThread()
    { return viewedThread_ ? viewedThread_->threadNo() : -1; }

    void getBacktrace(int threadNo);

public slots:
    void slotSelectFrame(int frameNo, int threadNo);
    void slotSelectionChanged(QListViewItem *thisItem);

signals:
    void produceBacktrace(int threadNo);
    void selectFrame(int frameNo, int threadNo, bool needFrames);

#if QT_VERSION < 300
private:
  QListViewItem* findItemWhichBeginsWith(const QString& text) const;
#endif

private:

    ThreadStackItem *viewedThread_;
    int currentFrame_;

    friend class FrameStackItem;
};

}

#endif
