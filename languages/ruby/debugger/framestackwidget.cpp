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
    setSorting(0);
    setSelectionMode(Single);
    addColumn(QString::null);
    header()->hide();

    connect( this, SIGNAL(clicked(QListViewItem*)),
             this, SLOT(slotSelectionChanged(QListViewItem*)) );
}


/***************************************************************************/

FramestackWidget::~FramestackWidget()
{}


// **************************************************************************

void FramestackWidget::clear()
{
    viewedThread_     = 0;

    QListView::clear();
}

/***************************************************************************/

void FramestackWidget::slotSelectionChanged(QListViewItem *thisItem)
{
    ThreadStackItem *thread = dynamic_cast<ThreadStackItem*> (thisItem);
    if (thread != 0)
    {
        slotSelectFrame(1, thread->threadNo());
    }
    else
    {
        FrameStackItem *frame = dynamic_cast<FrameStackItem*> (thisItem);
        if (frame != 0)
            slotSelectFrame(frame->frameNo(), frame->threadNo());
    }
}

/***************************************************************************/

// someone (the vartree :-)) wants us to select this frame.
void FramestackWidget::slotSelectFrame(int frameNo, int threadNo)
{
    FrameStackItem *frame = 0;
    if (threadNo != -1)
    {
        viewedThread_ = findThread(threadNo);
        if (viewedThread_ == 0)
        {
            Q_ASSERT(!viewedThread_);
            return;                 // fatal
        }
    }

    frame = findFrame(frameNo, threadNo);
    if (frame != 0)
        setSelected(frame, true);

    emit selectFrame(frameNo, threadNo, frame == 0);
}

/***************************************************************************/

void FramestackWidget::parseRDBThreadList(char *str)
{
    // on receipt of a thread list we must always clear the list.
    clear();
	QRegExp thread_re("(\\+)?\\s*(\\d+)\\s*(#<[^>]+>)\\s*([^:]+):(\\d+)");
    QString threadDesc("%1 %2 %3:%4");
	viewedThread_ = 0;
	
	int pos = thread_re.search(str, 0);
	
    while (pos != -1) {
        ThreadStackItem* thread = new ThreadStackItem(this, 
											threadDesc.arg(thread_re.cap(2)).arg(thread_re.cap(3))
											.arg(thread_re.cap(4)).arg(thread_re.cap(5)));
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
    // #0  Test::Test (this=0x8073b20, parent=0x0, name=0x0) at test.cpp:224
    // #1  0x804bba9 in main (argc=1, argv=0xbffff9c4) at main.cpp:24

    // If we don't have a thread program then clear the list.
    // We don't have to clear the list in a threaded programe because that's
    // already been done in the parseGDBThreadList() method.
    if (viewedThread_ == 0)
        clear();

    if(strlen(str) == 0)
        return;
    
    if (strncmp(str, "No stack.", strlen("No stack.")) == 0)
        return;
		
    while (char* end = strchr(str, '\n'))
    {
		char * hash = strchr(str, '#');
		if (hash != 0 && hash < end ) {
			str = hash;
		}
		
        if (*str == '#') {
            // make it a string
            *end = 0;
            QString frameDesc = QString(str);
            if (viewedThread_ != 0) {
                new FrameStackItem(viewedThread_, frameDesc);
            } else {
                 new FrameStackItem(this, frameDesc);
			}
        }
        str = end+1;                          // next string
    }

    // Make sure the first frame in the stopped backtrace is selected
    // and open
    if (viewedThread_ != 0)
        viewedThread_->setOpen(true);
    else
    {
        if (FrameStackItem* frame = (FrameStackItem*) firstChild())
            frame->setOpen(true);
    }
}

/***************************************************************************/

QString FramestackWidget::getFrameName(int frameNo, int threadNo)
{
    FrameStackItem *frame = findFrame(frameNo, threadNo);
    if (frame != 0)
    {
        QString frameStr = frame->text(0);
		QRegExp frame_re("^#(\\d+) ([^:]+):(\\d+)(:in `(.+)')?$");
		
		if (frame_re.search(frameStr, 0) == -1) {
    		return i18n("No stack");
		}
		
		QString method(frame_re.cap(5));
		if (method == "") {
			method = "toplevel";
		} else {
			method.append("(...)");
		}
		
        if (threadNo != -1)
        {
            QString frameName("T%1#%2 %3");
            return frameName.arg(threadNo).arg(frame_re.cap(1)).arg(method);
        }

        QString frameName("#%1 %2");
        return frameName.arg(frame_re.cap(1)).arg(method);
    }
    return i18n("No stack");
}

// **************************************************************************

ThreadStackItem *FramestackWidget::findThread(int threadNo)
{
    QListViewItem *sibling = firstChild();
    while (sibling != 0)
    {
        ThreadStackItem *thread = dynamic_cast<ThreadStackItem*> (sibling);
        if (thread != 0 && thread->threadNo() == threadNo)
        {
            return thread;
        }
        sibling = sibling->nextSibling();
    }

    return 0;
}

// **************************************************************************

FrameStackItem *FramestackWidget::findFrame(int frameNo, int threadNo)
{
    QListViewItem* frameItem = 0;

    if (threadNo != -1)
    {
        ThreadStackItem *thread = findThread(threadNo);
        if (thread == 0)
            return 0;     // no matching thread?
        frameItem = thread->firstChild();
    }
    else
        frameItem = firstChild();

    while (frameItem != 0)
    {
        if (((FrameStackItem*)frameItem)->frameNo() == frameNo)
            break;

        frameItem = frameItem->nextSibling();
    }
    return (FrameStackItem*)frameItem;
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

FrameStackItem::FrameStackItem(FramestackWidget *parent, const QString &frameDesc)
        : QListViewItem(parent),
        frameNo_(-1),
        threadNo_(-1)
{
	setText(VarNameCol, frameDesc);
    
	QRegExp num("(\\d+)");
    if (num.search(frameDesc,1)>=0) {
        frameNo_ = num.cap(1).toInt();
	}
}

// **************************************************************************

FrameStackItem::FrameStackItem(ThreadStackItem *parent, const QString &frameDesc)
        : QListViewItem(parent),
        frameNo_(-1),
        threadNo_(parent->threadNo())
{
	setText(VarNameCol, frameDesc);
    QRegExp num("(\\d+)");
    if (num.search(frameDesc,1)>=0)
        frameNo_ = num.cap(1).toInt();
}

// **************************************************************************

FrameStackItem::~FrameStackItem()
{}


// **************************************************************************

void FrameStackItem::setOpen(bool open)
{
    if (open)
        ((FramestackWidget*)listView())->slotSelectFrame(1, threadNo());

    QListViewItem::setOpen(open);
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

ThreadStackItem::ThreadStackItem(FramestackWidget *parent, const QString &threadDesc)
        : QListViewItem(parent, threadDesc),
        threadNo_(-1)
{
    setText(VarNameCol, threadDesc);
    setExpandable(true);
    QRegExp num("(\\d+)");
    if (num.search(threadDesc, 0) >= 0) {
        threadNo_ = num.cap(1).toInt();
	}
}

// **************************************************************************

ThreadStackItem::~ThreadStackItem()
{}


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
