/*
 * GDB Debugger Support
 *
 * Copyright 1999 John Birch <jbb@kdevelop.org>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
 * Copyright 2008 Vladimir Prus <ghost@cs.msu.su>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "stackmanager.h"

#include <klocale.h>
#include <kdebug.h>
#include <kglobalsettings.h>

#include "gdbparser.h"
#include "gdbcommand.h"
#include "gdbcontroller.h"

#include "util/treeitem.h"
#include "util/treemodel.h"

// #include "modeltest.h"

using namespace GDBMI;
using namespace GDBDebugger;

namespace GDBDebugger {

QString get_function_or_address(const GDBMI::Value &frame)
{
    if (frame.hasField("func"))
        return frame["func"].literal();
    else
        return frame["addr"].literal();
}

QString get_source(const GDBMI::Value &frame)
{
    if (frame.hasField("file"))
        return frame["file"].literal() + ":" +  frame["line"].literal();
    else if (frame.hasField("from"))
        return frame["from"].literal();
    else
        return "";
}

Frame::Frame(TreeModel* model, Thread* parent, const GDBMI::Value& frame)
: TreeItem(model, parent)
{
    updateSelf(frame);
}

void Frame::updateSelf(const GDBMI::Value& frame)
{
    id_ = frame["level"].toInt();
    setData(QVector<QString>() 
            << ("#" + frame["level"].literal())
            << get_function_or_address(frame)
            << get_source(frame));
}

Thread::Thread(TreeModel* model, TreeItem* parent, GDBController *controller,
               const GDBMI::Value& thread)
: TreeItem(model, parent), controller_(controller)
{
    id_ = thread["id"].toInt();
    
    updateSelf(thread, true);
    setHasMoreInitial(true);
}

void Thread::updateSelf(const GDBMI::Value& thread, bool initial)
{
    const GDBMI::Value& frame = thread["frame"];
    setData(QVector<QString>()
            << ("Thread " + thread["id"].literal())
            << get_function_or_address(frame)
            << get_source(frame));
    if (!initial)
        reportChange();
    
    if (isExpanded())
        fetchMoreChildren_1(true);
    else if (!initial) {
        /* We actually don't know if there are children or not.
           I don't really want to emit -stack-list-frames for each 
           thread, and -thread-info does not say if thread has more
           that one frame.
           So, mark this item as having children.  If there are none,
           which happens inside main, user will see the frames disappear
           when opening item.  It's better than showing the item as
           having no children, as otherwise user won't be able to
           expand it.  */
        setHasMore(true);        
    }
}

void Thread::fetchMoreChildren()
{
    fetchMoreChildren_1(false);
}

void Thread::fetchMoreChildren_1(bool clear) 
{
    /* We always ask GDB for:
       - the last frame we already have
       - 'step' more frames
       - one more frame
       
       We ask for the last present frame so that GDB does not give fits
       if the last frame we have is exactly the last frame target has.
       We ask for one more frame so that we know if there are more
       frames.  */
    int now;
    if (clear)
        now = 0;
    else 
        now = childItems.size();
    int next = now + step + 1;
    if (clear)
        now = 0;
    QString arg = QString("%1 %2").arg(now).arg(next);
    
    GDBCommand *c = new GDBCommand(StackListFrames, arg,
                                   this,
                                   &Thread::handleFrameList);
    c->setThread(id_);
    controller_->addCommand(c);
}

void Thread::handleFrameList(const GDBMI::ResultRecord& r)
{
    const GDBMI::Value& stack = r["stack"];
    int first = stack[0]["level"].toInt();
    if (first == 0)
    {
        /* For smooth update, after stepping we update first
           block of frames without previously clearing the
           existing ones. 
           Also note that we ignore the first stack frame
           here.  */
        int i;
        for (i = 0; i < step && (i+1) < stack.size(); ++i)
        {
            if (i < childItems.count())
            {
                Frame *f = static_cast<Frame *>(child(i));
                f->updateSelf(stack[i+1]);
            }
            else
                appendChild(new Frame(model(), this, stack[i+1]));
        }
        while (i < childItems.count())
            removeChild(i);
        
        setHasMore(stack.size() > step+1);
    }
    else
    {
        if (first  != childItems.size() + 1)
        {
            kDebug(9012) << "Got wrong frames\n";
            return;
        }
        for (int i = 0; i < step && i < stack.size(); ++i)
        {
            appendChild(new Frame(model(), this, stack[i]));
        }
        setHasMore(stack.size() > step);
    }
}

class DebugUniverse : public TreeItem
{
public:
    DebugUniverse(TreeModel* model, GDBController *controller, 
                  StackManager *stackManager)
    : TreeItem(model), controller_(controller), stackManager_(stackManager)
    {}

    void update()
    {
        controller_->addCommand(
            new GDBCommand(ThreadInfo, "",
                           this,
                           &DebugUniverse::handleThreadInfo));        
    }

    void fetchMoreChildren() {}
    
    using TreeItem::clear;

private:

    void handleThreadInfo(const GDBMI::ResultRecord& r)
    {
        const GDBMI::Value& threads = r["threads"];
        int current_id = r["current-thread-id"].toInt();

        // Collect the set of ids that are present in
        // target now.
        QSet<int> present;
        for (unsigned i = 0; i < threads.size(); ++i)
            present.insert(threads[i]["id"].toInt());

        // Remove threads that are no longer present
        for (int i = 0; i < childItems.size(); ++i)
        {
            Thread* t = static_cast<Thread *>(child(i));
            if (!present.contains(t->id()))
                removeChild(i);
        }

        int gidx, kidx;
        // Traverse GDB threads in backward order -- since GDB
        // reports them in backward order. We want UI to
        // show thread IDs in the natural order.
        // FIXME: make the code independent of whatever craziness
        // gdb might have tomorrow.

        gidx = threads.size()-1;
        kidx = 0;

        for (; gidx >= 0; --gidx)
        {
            bool updated = false;
            if (kidx < childItems.size())
            {
                Thread* t = static_cast<Thread *>(child(kidx));
                if (threads[gidx]["id"].toInt() == t->id())
                {
                    t->updateSelf(threads[gidx]);
                    updated = true;
                    ++kidx;
                }                
            }
            if (!updated)
                break;
        }
         
        for (; gidx >= 0; --gidx)
            appendChild(new Thread(model(), this,
                                   controller_, threads[gidx]));

        for (int i = 0; i < childItems.size(); ++i)
        {
            Thread* t = static_cast<Thread *>(child(i));
            if (t->id() == current_id)
            {
                emit stackManager_
                    ->selectThreadReally(model()->indexForItem(t, 0));
            }
        }

    }

    GDBController* controller_;    
    StackManager* stackManager_;
};
}

StackManager::StackManager(GDBController* controller)
  : controller_(controller), autoUpdate_(false)
{
    QVector<QString> header;
    header.push_back("ID");
    header.push_back("Function");
    header.push_back("Source");

    model_ = new TreeModel (header, this);
    universe_ = new DebugUniverse(model_, controller, this);
    model_->setRootItem(universe_);

    // new ModelTest(model_, this);
    
    // FIXME: maybe, all debugger components should derive from
    // a base class that does this connect.
    connect(controller, SIGNAL(event(event_t)),
            this,       SLOT(slotEvent(event_t)));
}

StackManager::~StackManager()
{
}

GDBController* StackManager::controller() const
{
    return controller_;
}

TreeModel *StackManager::model()
{
    return model_;
}

void StackManager::setAutoUpdate(bool b)
{
    if (!autoUpdate_ && b)
        universe_->update();
    autoUpdate_ = b;
}

void StackManager::updateThreads()
{   
    if (autoUpdate_)
        universe_->update();
}

void StackManager::slotEvent(event_t e)
{
    switch(e)
    {
        case program_state_changed:
            
            if (autoUpdate_)
                updateThreads();
            
            break;

         case thread_or_frame_changed:
            break;

        case program_exited:
        case debugger_exited: 
            universe_->clear();
            break;

        case debugger_busy:
        case debugger_ready:
        case program_running:
        case connected_to_program:
            break;
    }
}

#include "stackmanager.moc"
