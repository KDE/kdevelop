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

#include "framestackwidget.h"
#include "gdbparser.h"

#include <klocale.h>

#include <qheader.h>
#include <qlistbox.h>
#include <qregexp.h>
#include <qstrlist.h>

#include <ctype.h>


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

namespace GDBDebugger
{

FramestackWidget::FramestackWidget(QWidget *parent, const char *name, WFlags f)
        : QListView(parent, name, f),
        viewedThread_(0)
{
    setRootIsDecorated(true);
    setSorting(-1);
    setSelectionMode(Single);
    addColumn(QString::null);
    header()->hide();

    connect( this, SIGNAL(clicked(QListViewItem*)),
             this, SLOT(slotSelectionChanged(QListViewItem*)) );
}


/***************************************************************************/

FramestackWidget::~FramestackWidget()
{}

/***************************************************************************/

QListViewItem *FramestackWidget::lastChild() const
{
    QListViewItem* child = firstChild();
    if (child)
        while (QListViewItem* nextChild = child->nextSibling())
            child = nextChild;

    return child;
}

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
    if (thread)
    {
        slotSelectFrame(0, thread->threadNo());
    }
    else
    {
        FrameStackItem *frame = dynamic_cast<FrameStackItem*> (thisItem);
        if (frame)
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
        if (!viewedThread_)
        {
            Q_ASSERT(!viewedThread_);
            return;                 // fatal
        }
    }

    frame = findFrame(frameNo, threadNo);
    if (frame)
        setSelected(frame, true);

    emit selectFrame(frameNo, threadNo, !(frame));
}

void FramestackWidget::getBacktrace(int threadNo)
{
    if (threadNo != -1)
    {
        viewedThread_ = findThread(threadNo);
        if (!viewedThread_)
        {
            Q_ASSERT(!viewedThread_);
            return;                 // fatal
        }
    }

    emit produceBacktrace(threadNo);    
}


/***************************************************************************/

void FramestackWidget::parseGDBThreadList(char *str)
{
    // on receipt of a thread list we must always clear the list.
    clear();
    while (char *end = strchr(str, '\n'))
    {
        // make it a string and skip non-thread list strings
        *end = 0;
        if (*str == '*' || *str == ' ')
        {
            QString threadDesc = QString(str);
            ThreadStackItem* thread = new ThreadStackItem(this, str);
            // The thread with a '*' is always the viewedthread
            if (*str == '*')
                viewedThread_ = thread;
        }
        str = end+1;
    }
}

/***************************************************************************/

void FramestackWidget::parseGDBBacktraceList(char *str)
{
    // #0  Test::Test (this=0x8073b20, parent=0x0, name=0x0) at test.cpp:224
    // #1  0x804bba9 in main (argc=1, argv=0xbffff9c4) at main.cpp:24

    // If we don't have a thread program then clear the list.
    // We don't have to clear the list in a threaded programe because that's
    // already been done in the parseGDBThreadList() method.
    if (!viewedThread_)
        clear();

    if(!strlen(str))
        return;
    
    if (strncmp(str, "No stack.", 9) == 0)
        return;

    while (char* end = strchr(str, '\n'))
    {
        // Don't bother with extra data
        if (*str == '#')
        {
            // make it a string
            *end = 0;
            QString frameDesc = QString(str);
            if (viewedThread_)
                new FrameStackItem(viewedThread_, frameDesc);
            else
                new FrameStackItem(this, frameDesc);
        }
        str = end+1;                          // next string
    }

    currentFrame_ = 0;
    // Make sure the first frame in the stopped backtrace is selected
    // and open
    if (viewedThread_)
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
    if (frame)
    {
        QString frameStr = frame->text(0);
        const char *frameData = frameStr.latin1();
        if (const char *paramStart = strchr(frameData, '('))
        {
            const char *fnstart = paramStart-2;
            while (fnstart > frameData)
            {
                if (isspace(*fnstart))
                    break;
                fnstart--;
            }
            if (threadNo != -1)
            {
                QString frameName("T%1#%2 %3(...)");
                return frameName.arg(threadNo).arg(frameNo)
                       .arg(QCString(fnstart, paramStart-fnstart+1));
            }

            QString frameName("#%1 %2(...)");
            return frameName.arg(frameNo).arg(
                                QCString(fnstart, paramStart-fnstart+1));
        }
    }
    return i18n("No stack");
}

// **************************************************************************

ThreadStackItem *FramestackWidget::findThread(int threadNo)
{
    QListViewItem *sibling = firstChild();
    while (sibling)
    {
        ThreadStackItem *thread = dynamic_cast<ThreadStackItem*> (sibling);
        if (thread && thread->threadNo() == threadNo)
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

    while (frameItem)
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
        : QListViewItem(parent, parent->lastChild()),
        frameNo_(-1),
        threadNo_(-1)
{
    setText(VarNameCol, frameDesc);
    QRegExp num("[0-9]*");
    int start;
    if ((start=num.search(frameDesc,1))>=0)
        frameNo_ = frameDesc.mid(start, num.matchedLength()).toInt();
}

// **************************************************************************

FrameStackItem::FrameStackItem(ThreadStackItem *parent, const QString &frameDesc)
        : QListViewItem(parent, parent->lastChild()),
        frameNo_(-1),
        threadNo_(parent->threadNo())
{
    setText(VarNameCol, frameDesc);
    QRegExp num("[0-9]*");
    int start;
    if ((start=num.search(frameDesc,1))>=0)
        frameNo_ = frameDesc.mid(start, num.matchedLength()).toInt();
}

// **************************************************************************

FrameStackItem::~FrameStackItem()
{}

// **************************************************************************

QListViewItem *FrameStackItem::lastChild() const
{
    QListViewItem* child = firstChild();
    if (child)
        while (QListViewItem* nextChild = child->nextSibling())
            child = nextChild;

    return child;
}

// **************************************************************************

void FrameStackItem::setOpen(bool open)
{    
    if (open)
    {
        FramestackWidget* owner = (FramestackWidget*)listView();
        if (this->threadNo() != owner->viewedThread() &&
            this->frameNo() != owner->currentFrame_)
        {
            ((FramestackWidget*)listView())->slotSelectFrame(0, threadNo());
        }
    }

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
    QRegExp num("[0-9]*");
    int start;
    if ((start=num.search(threadDesc,2))>=0)
        threadNo_ = threadDesc.mid(start, num.matchedLength()).toInt();
}

// **************************************************************************

ThreadStackItem::~ThreadStackItem()
{}

// **************************************************************************

QListViewItem *ThreadStackItem::lastChild() const
{
    QListViewItem* child = firstChild();
    if (child)
        while (QListViewItem* nextChild = child->nextSibling())
            child = nextChild;

    return child;
}

// **************************************************************************

void ThreadStackItem::setOpen(bool open)
{
    // If we're openining, and have no child yet, get backtrace from
    // gdb.
    if (open && !firstChild())
        ((FramestackWidget*)listView())->getBacktrace(threadNo());

    QListViewItem::setOpen(open);
}

}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

#include "framestackwidget.moc"
