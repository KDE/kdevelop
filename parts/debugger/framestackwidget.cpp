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

FramestackWidget::FramestackWidget(QWidget *parent, const char *name, WFlags f)
        : QListView(parent, name, f),
        viewedThread_(0),
        stoppedAtThread_(0),
        currentFrame_(0)
{
    setRootIsDecorated(true);
    setResizeMode(LastColumn);
    setSorting(-1);
    setSelectionMode(Single);
    addColumn(QString::null);
    header()->hide();

    connect( this, SIGNAL(selectionChanged(QListViewItem*)),
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
    stoppedAtThread_  = 0;
    currentFrame_     = 0;

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

        frame = findFrame(frameNo, threadNo);
        if (frame)
            setSelected(frame, true);
    }

    emit selectFrame(frameNo, threadNo, !(frame));
}

/***************************************************************************/

void FramestackWidget::parseGDBThreadList(char *str)
{
    // on receipt of a thread list we must always clear the list.
    threadList_.clear();
    while (char *end = strchr(str, '\n'))
    {
        *end = 0;                             // make it a string

        if (*str == '*' || *str == ' ')       // skip non-thread list strings
        {
            QString threadDesc = QString(str);
            threadList_.append(threadDesc);
        }
        str = end+1;                          // next string
    }
}

/***************************************************************************/

void FramestackWidget::parseGDBBacktraceList(char *str)
{
    //#0  Test::Test (this=0x8073b20, parent=0x0, name=0x0) at test.cpp:224
    //#1  0x804bba9 in main (argc=1, argv=0xbffff9c4) at main.cpp:24

    // just find out what the first line of str is and store that in firstLine
    // (store without the line number to enable comparison with firstLineOfLastBacktrace_)
    QString firstLineLeft;
    QString firstLine;
    bool bBreak = false;
    char* end = strchr(str, '\n');
    while (!bBreak && end)
    {
        *end = 0;                             // make it a string
        QString frameDesc = QString(str);
        if (*str == '#')
        {                     // Don't bother with extra data
            firstLine = frameDesc;
            int idx = frameDesc.findRev(':');
            if (idx != -1)
            {
                firstLineLeft = firstLine.left(idx);
            }
            bBreak = true;
            str = end+1;                          // next string
        }
        else
        {
            end = strchr(str, '\n');
            str = end+1;                          // next string
        }
    }

    bool bRegenerateAll = false;
    if ((!threadList_.isEmpty() &&
            threadList_.count() != (unsigned int)(childCount())) ||
            childCount() == 0)
    {
        // a new thread came or some threads have gone
        bRegenerateAll = true;
    }

    // not always we have to remove all listview items, do some magic to find that out
    if (bRegenerateAll || firstLineOfLastBacktrace_ != firstLineLeft)
    {
        if (threadList_.isEmpty())
        {
            clear(); // non-threaded
        }
        else
        {
            // TODO: it aint safe because viewedThread_
            // and thread of the currently processed str can be different.
            // Hmm...we need to know which thread is meant with the call of this method
            //-----------------
            // Since we just get the backtrace of a certain thread we must avoid removing all other possibly opened thread callstacks
            if (!bRegenerateAll && viewedThread_)
            {
                // just remove the listview items of the currently changed callstack
                while (viewedThread_->childCount() > 0)
                {
                    viewedThread_->takeItem(viewedThread_->firstChild());
                }
            }
            else
            {
                // rebuild everything
                clear();
                for (QStringList::Iterator it = threadList_.begin(); it != threadList_.end(); ++it)
                {
                    QString s = *it;
                    int idx = s.find(")");
                    if (idx != -1)
                    {
                        s = s.left(idx+1);
                    }
                    ThreadStackItem* thread = new ThreadStackItem(this, s);

                    // This indicates the current thread
                    if (s[0] == '*')
                    {
                        viewedThread_ = thread;
                        stoppedAtThread_ = thread;
                        thread->setOpen(true);
                    }
                }
            }
        }

        // insert the first line
        if (viewedThread_)
            new FrameStackItem(viewedThread_, firstLine);
        else
            new FrameStackItem(this, firstLine);

        // insert the rest of the lines
        while (char* end = strchr(str, '\n'))
        {
            *end = 0;                             // make it a string
            QString frameDesc = QString(str);
            if (*str == '#')
            {                     // Don't bother with extra data
                if (viewedThread_)
                    new FrameStackItem(viewedThread_, frameDesc);
                else
                    new FrameStackItem(this, frameDesc);
            }
            str = end+1;                          // next string
        }

        firstLineOfLastBacktrace_ = firstLineLeft;
    }
    else
    {
        // this is the optimized part, we do not have to rebuild the whole listview but only one item must be changed
#if QT_VERSION < 300
        QListViewItem* lvi = findItemWhichBeginsWith(firstLineLeft);
#else

        QListViewItem* lvi = findItem(firstLineLeft,0,Qt::BeginsWith);
#endif

        if (lvi)
        {
            lvi->setText(0,firstLine);
        }
    }

    //    // #0  Test::Test (this=0x8073b20, parent=0x0, name=0x0) at test.cpp:224
    //    // #1  0x804bba9 in main (argc=1, argv=0xbffff9c4) at main.cpp:24
    //
    //    // If we don't have a thread program then clear the list.
    //    if (!viewedThread_)
    //        clear();
    //
    //    while (char* end = strchr(str, '\n')) {
    //        *end = 0;                             // make it a string
    //        QString frameDesc = QString(str);
    //        if (*str == '#')                      // Don't bother with extra data
    //            if (viewedThread_)
    //                new FrameStackItem(viewedThread_, frameDesc);
    //            else
    //                new FrameStackItem(this, frameDesc);
    //        str = end+1;                          // next string
    //    }
}

/***************************************************************************/

#if QT_VERSION < 300
// this method is a slightly adapted downport of findItem of Qt-3.0.3,
// just necessary because it doesn't exist in Qt-2.
QListViewItem* FrameStack::findItemWhichBeginsWith(const QString& text) const
{
    if ( text.isEmpty() )
        return 0;

    QString itmtxt;
    QString comtxt = text;
    comtxt = comtxt.lower();

    QListViewItem* curItem = currentItem();
    if (!curItem)
        curItem = viewedThread_;
    QListViewItemIterator it(curItem);
    QListViewItem* sentinel = 0;
    QListViewItem* item;

    for (int pass = 0; pass < 2; pass++)
    {
        while ((item = it.current()) != sentinel)
        {
            itmtxt = item->text(0);
            itmtxt = itmtxt.lower();
            if ( itmtxt.startsWith(comtxt))
                return item;
            ++it;
        }

        it = QListViewItemIterator(firstChild());
        sentinel = curItem;
    }
    return 0;
}
#endif

/***************************************************************************/

QCString FramestackWidget::getFrameParams(int frameNo, int threadNo)
{
    if (FrameStackItem* frame = findFrame(frameNo, threadNo))
    {
        QString frameStr = frame->text(0);
        char *frameData = (char*) frameStr.latin1();
        if (char *paramStart = strchr(frameData, '('))
        {
            GDBParser *parser = GDBParser::getGDBParser();
            if (char *paramEnd = parser->skipDelim(paramStart, '(', ')'))
            {
                // allow for operator()(params)
                if (paramEnd == paramStart+2)
                {
                    if (*(paramEnd+1) == '(')
                    {
                        paramStart = paramEnd+1;
                        paramEnd = parser->skipDelim(paramStart, '(', ')');
                        if (!paramEnd)
                            return QCString();
                    }
                }

                // The parameters are contained _within_ the brackets.
                if (paramEnd-paramStart > 2)
                    return QCString(paramStart+1, paramEnd-paramStart-1);
            }
        }
    }

    return QCString();
}

/***************************************************************************/

QString FramestackWidget::getFrameName(int frameNo, int threadNo)
{
    if (FrameStackItem *frame = findFrame(frameNo, threadNo))
    {
        QString frameStr = frame->text(0);
        char *frameData = (char*) frameStr.latin1();
        if (char *paramStart = strchr(frameData, '('))
        {
            char *fnstart = paramStart-2;
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
            return frameName.arg(frameNo).arg(QCString(fnstart, paramStart-fnstart+1));
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

    if (frameItem == 0)
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
    int len;
    if ((start=num.match(frameDesc,1,&len))>=0)
        frameNo_ = frameDesc.mid(start,len).toInt();
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
    int len;
    if ((start=num.match(frameDesc,1,&len))>=0)
        frameNo_ = frameDesc.mid(start,len).toInt();
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
        ((FramestackWidget*)listView())->slotSelectFrame(0, threadNo());

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
    int len;
    if ((start=num.match(threadDesc,2,&len))>=0)
        threadNo_ = threadDesc.mid(start,len).toInt();
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
    if (open)
        ((FramestackWidget*)listView())->slotSelectFrame(0, threadNo());

    QListViewItem::setOpen(open);
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

#include "framestackwidget.moc"
