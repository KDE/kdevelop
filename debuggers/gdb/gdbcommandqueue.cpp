// *************************************************************************
//                          gdbcommandqueue.cpp
//                             -------------------
//    begin                : Wed Dec 5, 2007
//    copyright            : (C) 2007 by Hamish Rodda
//    email                : rodda@kde.org
// **************************************************************************
//
// **************************************************************************
// *                                                                        *
// *   This program is free software; you can redistribute it and/or modify *
// *   it under the terms of the GNU General Public License as published by *
// *   the Free Software Foundation; either version 2 of the License, or    *
// *   (at your option) any later version.                                  *
// *                                                                        *
// **************************************************************************

#include "gdbcommandqueue.h"

#include "mi/gdbmi.h"
#include "gdbcommand.h"

using namespace GDBDebugger;
using namespace GDBMI;

CommandQueue::CommandQueue()
    : m_tokenCounter(0)
{
}

CommandQueue::~CommandQueue()
{
    qDeleteAll(m_commandList);
}

void CommandQueue::enqueue(GDBCommand* command, QueuePosition insertPosition)
{
    ++m_tokenCounter;
    if (m_tokenCounter == 0)
        m_tokenCounter = 1;
    command->setToken(m_tokenCounter);

    switch (insertPosition) {
        case QueueAtFront:
            m_commandList.prepend(command);
            break;
        case QueueAtEnd:
            m_commandList.append(command);
            break;
    }

    if (command->flags() & (CmdImmediately | CmdInterrupt))
        ++m_immediatelyCounter;

    rationalizeQueue(command);
}

void CommandQueue::rationalizeQueue(GDBCommand * command)
{
    if (command->type() >= ExecAbort && command->type() <= ExecUntil)
      // Changing execution location, abort any variable updates
      removeVariableUpdates();
}

void CommandQueue::removeVariableUpdates()
{
    QMutableListIterator<GDBCommand*> it = m_commandList;

    while (it.hasNext()) {
        GDBCommand* command = it.next();
        CommandType type = command->type();
        if ((type >= VarEvaluateExpression && type <= VarListChildren) || type == VarUpdate) {
            if (command->flags() & (CmdImmediately | CmdInterrupt))
                --m_immediatelyCounter;
            it.remove();
            delete command;
        }
    }
}

void CommandQueue::clear()
{
    qDeleteAll(m_commandList);
    m_commandList.clear();
    m_immediatelyCounter = 0;
}

int CommandQueue::count() const
{
    return m_commandList.count();
}

bool CommandQueue::isEmpty() const
{
    return m_commandList.isEmpty();
}

bool CommandQueue::haveImmediateCommand() const
{
    return m_immediatelyCounter > 0;
}

GDBCommand* CommandQueue::nextCommand()
{
    if (m_commandList.isEmpty())
        return nullptr;

    GDBCommand* command = m_commandList.takeAt(0);

    if (command->flags() & (CmdImmediately | CmdInterrupt))
        --m_immediatelyCounter;

    return command;
}

