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
#include "gdbcommand.h"

#include <klocale.h>
#include <kdebug.h>

#include <qheader.h>
#include <qlistbox.h>
#include <qregexp.h>
#include <qstrlist.h>
#include <qpainter.h>


#include <ctype.h>


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

namespace GDBDebugger
{

FramestackWidget::FramestackWidget(GDBController* controller,
                                   QWidget *parent, 
                                   const char *name, WFlags f)
        : QListView(parent, name, f),
          viewedThread_(0),
          controller_(controller)
{
    setRootIsDecorated(true);
    setSorting(-1);
    setSelectionMode(Single);
    addColumn(QString::null); // Frame number
    addColumn(QString::null); // function name/address
    addColumn(QString::null); // source
    header()->hide();


    // FIXME: maybe, all debugger components should derive from
    // a base class that does this connect.
    connect(controller, SIGNAL(event(GDBController::event_t)),
            this,       SLOT(slotEvent(GDBController::event_t)));

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

    stackDepth_.clear();
}

/***************************************************************************/

void FramestackWidget::slotSelectionChanged(QListViewItem *thisItem)
{
    ThreadStackItem *thread = dynamic_cast<ThreadStackItem*> (thisItem);
    if (thread)
    {
        controller_->selectFrame(0, thread->threadNo());
    }
    else
    {
        FrameStackItem *frame = dynamic_cast<FrameStackItem*> (thisItem);
        if (frame)
        {
            if (frame->text(0) == "...")
            {   
                getBacktrace(frame->frameNo(), frame->frameNo() + frameChunk_);
            }
            else
            {
                controller_->
                    selectFrame(frame->frameNo(), frame->threadNo());
            }
        }
    }
}

/***************************************************************************/

void FramestackWidget::slotEvent(GDBController::event_t e)
{
    switch(e)
    {
        case GDBController::program_state_changed: 

            kdDebug(9012) << "Clearning framestack\n";
            clear();

            controller_->addCommand(
                new GDBCommand("-thread-list-ids",
                               this, &FramestackWidget::handleThreadList));

            break;
            

         case GDBController::thread_or_frame_changed: 

             if (viewedThread_)
             {
                 // For non-threaded programs frame switch is no-op
                 // as far as framestack is concerned.
                 // FIXME: but need to highlight the current frame.
                 
                 if (ThreadStackItem* item 
                     = findThread(controller_->currentThread()))
                 {
                     viewedThread_ = item;

                     if (!item->firstChild())
                     {
                         // No backtrace for this thread yet.
                         getBacktrace();
                     }
                 }
             }

            break;

        case GDBController::program_exited: 
        case GDBController::debugger_exited: 
        {
            clear();
            break;
        }        
        case GDBController::debugger_busy:
        case GDBController::debugger_ready:
        case GDBController::shared_library_loaded:
        case GDBController::program_running:
        case GDBController::connected_to_program:
            break;
    }
}

void FramestackWidget::getBacktrace(int min_frame, int max_frame)
{
    minFrame_ = min_frame;
    maxFrame_ = max_frame;

    controller_->addCommand(
        new GDBCommand(QString("-stack-info-depth %1").arg(max_frame+1),
                       this, 
                       &FramestackWidget::handleStackDepth));        
}

void FramestackWidget::handleStackDepth(const GDBMI::ResultRecord& r)
{
    int existing_frames = r["depth"].literal().toInt();

    has_more_frames = (existing_frames > maxFrame_);

    if (existing_frames < maxFrame_)
        maxFrame_ = existing_frames;

    controller_->addCommand(
        new GDBCommand(QString("-stack-list-frames %1 %2")
                       .arg(minFrame_).arg(maxFrame_),
                       this, &FramestackWidget::parseGDBBacktraceList));    
}

void FramestackWidget::getBacktraceForThread(int threadNo)
{
    unsigned currentThread = controller_->currentThread();
    if (viewedThread_)
    {
        // Switch to the target thread.
        controller_->addCommand(
            new GDBCommand(QString("-thread-select %1")
                           .arg(threadNo).ascii()));

        viewedThread_ = findThread(threadNo);
    }
    
    getBacktrace();

    if (viewedThread_)
    {
        // Switch back to the original thread.
        controller_->addCommand(
            new GDBCommand(QString("-thread-select %1")
                           .arg(currentThread).ascii()));
    }
}

void FramestackWidget::handleThreadList(const GDBMI::ResultRecord& r)
{
    // Gdb reply is: 
    //  ^done,thread-ids={thread-id="3",thread-id="2",thread-id="1"},
    // which syntactically is a tuple, but one has to access it
    // by index anyway.
    const GDBMI::TupleValue& ids = 
        dynamic_cast<const GDBMI::TupleValue&>(r["thread-ids"]);

    if (ids.results.size() > 1)
    {
        // Need to iterate over all threads to figure out where each one stands.
        // Note that this sequence of command will be executed in strict
        // sequences, so no other view can add its command in between and
        // get state for a wrong thread.
                        
        // Really threaded program.
        for(unsigned i = 0, e = ids.results.size(); i != e; ++i)
        {
            QString id = ids.results[i]->value->literal();

            controller_->addCommand(
                new GDBCommand(QString("-thread-select %1").arg(id).ascii(), 
                               this, &FramestackWidget::handleThread));
        }

        controller_->addCommand(
            new GDBCommand(QString("-thread-select %1")
                           .arg(controller_->currentThread()).ascii()));
    }

    // Get backtrace for the current thread. We need to do this
    // here, and not in event handler for program_state_changed, 
    // viewedThread_ is initialized by 'handleThread' before
    // backtrace handler is called.
    getBacktrace();
}

void FramestackWidget::handleThread(const GDBMI::ResultRecord& r)
{
    QString id = r["new-thread-id"].literal();
    int id_num = id.toInt();

    QString name_column;
    QString func_column;
    QString args_column;
    QString source_column;
    
    formatFrame(r["frame"], func_column, source_column);

    ThreadStackItem* thread = new ThreadStackItem(this, id_num);
    thread->setText(1, func_column);
    thread->setText(2, source_column);

    // The thread with a '*' is always the viewedthread

    if (id_num == controller_->currentThread())
    {
        viewedThread_ = thread;
        setSelected(viewedThread_, true);
    }
}


void FramestackWidget::parseGDBBacktraceList(const GDBMI::ResultRecord& r)
{
    if (!r.hasField("stack"))
        return;

    const GDBMI::Value& frames = r["stack"];    

    if (frames.empty())
        return;

    Q_ASSERT(dynamic_cast<const GDBMI::ListValue*>(&frames));

    // Remove "..." item, if there's one.
    QListViewItem* last;    
    if (viewedThread_)
    {
        last = viewedThread_->firstChild();
        if (last)
            while(last->nextSibling())
                last = last->nextSibling();
    }
    else 
    {
        last = lastItem();
    }
    if (last && last->text(0) == "...")
        delete last;

    int lastLevel;
    for(unsigned i = 0, e = frames.size(); i != e; ++i)
    {
        const GDBMI::Value& frame = frames[i];
      
        // For now, just produce string simular to gdb
        // console output. In future we might have a table,
        // or something better.
        QString frameDesc;

        QString name_column;
        QString func_column;
        QString source_column;

        QString level_s = frame["level"].literal();
        int level = level_s.toInt();

        name_column = "#" + level_s;

        formatFrame(frame, func_column, source_column);
        
        FrameStackItem* item;
        if (viewedThread_)
            item = new FrameStackItem(viewedThread_, level, name_column);
        else
            item = new FrameStackItem(this, level, name_column);
        lastLevel = level;

        item->setText(1, func_column);
        item->setText(2, source_column);        
    }
    if (has_more_frames)
    {
        QListViewItem* item;
        if (viewedThread_)
            item = new FrameStackItem(viewedThread_, lastLevel+1, "...");
        else
            item = new FrameStackItem(this, lastLevel+1, "...");
        item->setText(1, "(click to get more frames)");
    }

    currentFrame_ = 0;
    // Make sure the first frame in the stopped backtrace is selected
    // and open
    if (viewedThread_)
        viewedThread_->setOpen(true);
    else
    {
        if (FrameStackItem* frame = (FrameStackItem*) firstChild())
        {
            frame->setOpen(true);
            setSelected(frame, true);
        }
    }
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

void FramestackWidget::formatFrame(const GDBMI::Value& frame,
                                   QString& func_column,
                                   QString& source_column)
{
    func_column = source_column = "";

    if (frame.hasField("func"))
    {
        func_column += " " + frame["func"].literal();
    }
    else
    {
        func_column += " " + frame["address"].literal();
    }


    if (frame.hasField("file"))
    {
        source_column = frame["file"].literal();

        if (frame.hasField("line"))
        {
            source_column += ":" + frame["line"].literal();
        }
    }
    else if (frame.hasField("from"))
    {
        source_column = frame["from"].literal();
    }
}


void FramestackWidget::drawContentsOffset( QPainter * p, int ox, int oy,
                                           int cx, int cy, int cw, int ch )
{
    QListView::drawContentsOffset(p, ox, oy, cx, cy, cw, ch);

    int s1_x = header()->sectionPos(1);
    int s1_w = header()->sectionSize(1);

    QRect section1(s1_x, contentsHeight(), s1_w, viewport()->height());

    p->fillRect(section1, QColor("#e4f4fe"));
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

FrameStackItem::FrameStackItem(FramestackWidget *parent, 
                               unsigned frameNo,
                               const QString &name)
        : QListViewItem(parent, parent->lastChild()),
        frameNo_(frameNo),
        threadNo_(-1)
{
    setText(0, name);
}

// **************************************************************************

FrameStackItem::FrameStackItem(ThreadStackItem *parent, 
                               unsigned frameNo,
                               const QString &name)
        : QListViewItem(parent, parent->lastChild()),
        frameNo_(frameNo),
        threadNo_(parent->threadNo())
{
    setText(0, name);
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
#if 0
    if (open)
    {
        FramestackWidget* owner = (FramestackWidget*)listView();
        if (this->threadNo() != owner->viewedThread() &&
            this->frameNo() != owner->currentFrame_)
        {
            ((FramestackWidget*)listView())->slotSelectFrame(0, threadNo());
        }
    }
#endif
    QListViewItem::setOpen(open);
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

ThreadStackItem::ThreadStackItem(FramestackWidget *parent, unsigned threadNo)
: QListViewItem(parent),
  threadNo_(threadNo)
{
    setText(0, i18n("Thread %1").arg(threadNo_));
    setExpandable(true);
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
    {
        // Not that this will not switch to another thread (and won't show
        // position in that other thread). This will only get the frames.

        // Imagine you have 20 frames and you want to find one blocked on
        // mutex. You don't want a new source file to be opened for each
        // thread you open to find if that's the one you want to debug.        
        ((FramestackWidget*)listView())->getBacktraceForThread(threadNo());
    }

    if (open)
    {
        savedFunc_ = text(1);
        setText(1, "");
        savedSource_ = text(2);
        setText(2, "");
    }
    else
    {
        setText(1, savedFunc_);
        setText(2, savedSource_);
    }

    QListViewItem::setOpen(open);
}

void FrameStackItem::paintCell(QPainter * p, const QColorGroup & cg, 
                               int column, int width, int align )
{
    QColorGroup cg2(cg);
    if (column % 2)
    {
        cg2.setColor(QColorGroup::Base, QColor("#e4f4fe"));
    }
    QListViewItem::paintCell(p, cg2, column, width, align);
}

void ThreadStackItem::paintCell(QPainter * p, const QColorGroup & cg, 
                               int column, int width, int align )
{
    QColorGroup cg2(cg);
    if (column % 2)
    {
        cg2.setColor(QColorGroup::Base, QColor("#e4f4fe"));
    }
    QListViewItem::paintCell(p, cg2, column, width, align);
}


}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

#include "framestackwidget.moc"
