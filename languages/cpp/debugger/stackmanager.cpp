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

class Frame : public TreeItem
{
public:
    Frame(TreeModel* model, TreeItem* parent, const GDBMI::Value& frame)
    : TreeItem(model, parent)
    {
        setData(QVector<QString>() 
                << ("#" + frame["level"].literal())
                << get_function_or_address(frame)
                << get_source(frame));
    }

    void fetchMoreChildren() {}
};


class Thread : public TreeItem
{
public:
    Thread(TreeModel* model, TreeItem* parent, GDBController *controller,
           const GDBMI::Value& thread)
    : TreeItem(model, parent), controller_(controller)
    {
        id_ = thread["id"].toInt();
        const GDBMI::Value& frame = thread["frame"];
        setData(QVector<QString>()
                << ("Thread " + thread["id"].literal())
                << get_function_or_address(frame)
                << get_source(frame));

        setHasMoreInitial(true);
    }

    void fetchMoreChildren() 
    {
        /* Note that the children are frames starting from the #1,
           with data for #0 shown in the thread item itself.  */
        int now = childItems.size() + 1;
        int next = now + step + 1;
        QString arg = QString("%1 %2").arg(now).arg(next);

        GDBCommand *c = new GDBCommand(StackListFrames, arg,
                                       this,
                                       &Thread::handleFrameList);
        c->setThread(id_);
        controller_->addCommand(c);
    }

    void handleFrameList(const GDBMI::ResultRecord& r)
    {
        const GDBMI::Value& stack = r["stack"];
        if (stack[0]["level"].toInt() != childItems.size() + 1)
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

    GDBController* controller_;
    int id_;

    static const int step = 5;
};


class DebugUniverse : public TreeItem
{
public:
    DebugUniverse(TreeModel* model, GDBController *controller)
    : TreeItem(model), controller_(controller)
    {}

    void update()
    {
        clear();
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
        int current_index;

        for (unsigned i = 0; i < threads.size(); ++i)
        {
            if (threads[i]["id"].toInt() == current_id)
                current_index = i;
            appendChild(new Thread(model(), this,
                                   controller_, threads[i]));
        }

        //static_cast<Thread*>(child(current_index))->fetchMoreChildren();
    }

    GDBController* controller_;    
};
}

StackManager::StackManager(GDBController* controller)
: controller_(controller)
{
    QVector<QString> header;
    header.push_back("ID");
    header.push_back("Function");
    header.push_back("Source");

    model_ = new TreeModel (header, this);
    universe_ = new DebugUniverse(model_, controller);
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
