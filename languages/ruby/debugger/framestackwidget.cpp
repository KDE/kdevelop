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

#include "framestackwidget.h"
#include "rdbparser.h"

#include <klocale.h>
#include <kdebug.h>

#include <qheader.h>
#include <qlistbox.h>
#include <qregexp.h>
#include <qstrlist.h>
#include <qfileinfo.h>

#include <ctype.h>


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

namespace RDBDebugger
{

FramestackWidget::FramestackWidget(QWidget *parent, const char *name, WFlags f)
        : QListView(parent, name, f),
        viewedThread_(0)
{
    setRootIsDecorated(true);
    setSelectionMode(Single);
    addColumn(QString());
    setSorting(0);
    header()->hide();

    connect( this, SIGNAL(clicked(QListViewItem*)),
             this, SLOT(slotSelectionChanged(QListViewItem*)) );
}


/***************************************************************************/

FramestackWidget::~FramestackWidget()
{
}


// **************************************************************************

void FramestackWidget::clear()
{
    viewedThread_ = 0;
    QListView::clear();
}

/***************************************************************************/

void FramestackWidget::slotSelectionChanged(QListViewItem * item)
{
	if (item == 0) {
		return;
	}
	
	if (item->rtti() == RTTI_THREAD_STACK_ITEM) {
		ThreadStackItem * thread = (ThreadStackItem*) item;
        slotSelectFrame(1, thread->threadNo());
	} else if (item->rtti() == RTTI_FRAME_STACK_ITEM) {
		FrameStackItem * frame = (FrameStackItem*) item;
		slotSelectFrame(frame->frameNo(), frame->threadNo());
	}
	
	return;
}

/***************************************************************************/

void FramestackWidget::slotSelectFrame(int frameNo, int threadNo)
{
    FrameStackItem * frame = findFrame(frameNo, threadNo);
	
	if (frame != 0) {
		setSelected(frame, true);
    	emit selectFrame(frameNo, threadNo, frame->frameName());
	} else {
    	emit selectFrame(frameNo, threadNo, QString());
	}
}

/***************************************************************************/

void FramestackWidget::parseRDBThreadList(char *str)
{
	// on receipt of a thread list we must always clear the list.
    clear();
	
	QRegExp thread_re("(\\+)?\\s*(\\d+)\\s*(#<[^>]+>\\s*[^:]+:\\d+)");
	int pos = thread_re.search(str);
	viewedThread_ = 0;
	
    while (pos != -1) {
		ThreadStackItem* thread;
        thread = new ThreadStackItem(	this,
										thread_re.cap(2).toInt(),
										QString("%1 %2").arg(thread_re.cap(2)).arg(thread_re.cap(3)) );
		// The thread with a '+' is always the viewedthread
        if (thread_re.cap(1) == "+") {
            viewedThread_ = thread;
		}
		
		pos += thread_re.matchedLength();
		pos = thread_re.search(str, pos);
    }
	
	return;
}

/***************************************************************************/

void FramestackWidget::parseRDBBacktraceList(char *str)
{
	QRegExp	frame_re("#(\\d+) ([^:]+):(\\d+)(:in `([^\\n]+)')?");
	int	pos = frame_re.search(str);
	
    while (pos != -1) {
		QString	method(frame_re.cap(5));
		if (method == "") {
			method = "toplevel";
		} else {
			method.append("(...)");
		}
		
		int frameNo = frame_re.cap(1).toInt();
		QString frameName = QString("T%1#%2 %3").arg(viewedThread_->threadNo()).arg(frame_re.cap(1)).arg(method);
		new FrameStackItem(viewedThread_, frameNo, QString(frame_re.cap(0)), frameName);
		
		// Tell the Variable Tree that this frame is active
		emit frameActive(frameNo, viewedThread_->threadNo(), frameName);
				
		pos += frame_re.matchedLength();
		pos = frame_re.search(str, pos);
    }
	
	if (viewedThread_ != 0) {
		viewedThread_->setOpen(true);
	}
	
	return;
}

// **************************************************************************

ThreadStackItem *FramestackWidget::findThread(int threadNo)
{
    QListViewItem *sibling = firstChild();
    while (sibling != 0) {
        ThreadStackItem *thread = (ThreadStackItem*) sibling;
        if (thread->threadNo() == threadNo) {
            return thread;
        }
        sibling = sibling->nextSibling();
    }

    return 0;
}

// **************************************************************************

FrameStackItem *FramestackWidget::findFrame(int frameNo, int threadNo)
{
	ThreadStackItem * thread = findThread(threadNo);
	if (thread == 0) {
		kdDebug(9012) << "FramestackWidget::findFrame: no matching thread " << 
					frameNo << " thread: " << threadNo << endl;
		return 0;     // no matching thread?
	}

	QListViewItem * frameItem = thread->firstChild();

    while (frameItem != 0) {
        if (((FrameStackItem *) frameItem)->frameNo() == frameNo) {
            return (FrameStackItem *) frameItem;
		}

        frameItem = frameItem->nextSibling();
    }
	
    return 0;
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

// **************************************************************************

FrameStackItem::FrameStackItem(ThreadStackItem *parent, int frameNo, const QString &frameDesc, const QString& frameName)
        : QListViewItem(parent),
        frameNo_(frameNo),
        threadNo_(parent->threadNo()),
		frameName_(frameName)		
{
	setText(0, frameDesc);
	key_.sprintf("%.6d", frameNo_);
}

// **************************************************************************

FrameStackItem::~FrameStackItem()
{
}

// **************************************************************************

QString FrameStackItem::key(int /*column*/, bool /*ascending*/) const 
{

	return key_;
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

ThreadStackItem::ThreadStackItem(FramestackWidget *parent, int threadNo, const QString &threadDesc)
        : QListViewItem(parent),
        threadNo_(threadNo)
{
    setText(0, threadDesc);
    setExpandable(true);
}

// **************************************************************************

ThreadStackItem::~ThreadStackItem()
{
}

// **************************************************************************

void ThreadStackItem::setOpen(bool open)
{
    if (open)
        ((FramestackWidget*)listView())->slotSelectFrame(1, threadNo());

    QListViewItem::setOpen(open);
}

}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

#include "framestackwidget.moc"
