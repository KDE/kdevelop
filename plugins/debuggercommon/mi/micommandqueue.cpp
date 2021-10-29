/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "micommandqueue.h"

#include "mi.h"
#include "micommand.h"
#include "debuglog.h"

using namespace KDevMI::MI;

CommandQueue::CommandQueue()
{
}

CommandQueue::~CommandQueue()
{
    qDeleteAll(m_commandList);
}

void CommandQueue::enqueue(MICommand* command)
{
    ++m_tokenCounter;
    if (m_tokenCounter == 0)
        m_tokenCounter = 1;
    command->setToken(m_tokenCounter);

    // take the time when this command was added to the command queue
    command->markAsEnqueued();

    m_commandList.append(command);

    if (command->flags() & (CmdImmediately | CmdInterrupt))
        ++m_immediatelyCounter;

    rationalizeQueue(command);
    dumpQueue();
}

void CommandQueue::dumpQueue() const
{
    qCDebug(DEBUGGERCOMMON) << "Pending commands" << m_commandList.count();
    unsigned commandNum = 0;
    for (const MICommand* command : m_commandList) {
        qCDebug(DEBUGGERCOMMON) << "Command" << commandNum << command->initialString();
        ++commandNum;
    }
}

void CommandQueue::rationalizeQueue(MICommand* command)
{
    if ((command->type() >= ExecAbort && command->type() <= ExecUntil) &&
         command->type() != ExecArguments ) {
        // Changing execution location, abort any variable updates
        removeVariableUpdates();
        // ... and stack list updates
        removeStackListUpdates();
    }
}

void CommandQueue::removeVariableUpdates()
{
    QMutableListIterator<MICommand*> it = m_commandList;

    while (it.hasNext()) {
        MICommand* command = it.next();
        CommandType type = command->type();
        if ((type >= VarEvaluateExpression && type <= VarListChildren) || type == VarUpdate) {
            if (command->flags() & (CmdImmediately | CmdInterrupt))
                --m_immediatelyCounter;
            it.remove();
            delete command;
        }
    }
}

void CommandQueue::removeStackListUpdates()
{
    QMutableListIterator<MICommand*> it = m_commandList;

    while (it.hasNext()) {
        MICommand* command = it.next();
        CommandType type = command->type();
        if (type >= StackListArguments && type <= StackListLocals) {
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

MICommand* CommandQueue::nextCommand()
{
    if (m_commandList.isEmpty())
        return nullptr;

    MICommand* command = m_commandList.takeAt(0);

    if (command->flags() & (CmdImmediately | CmdInterrupt))
        --m_immediatelyCounter;

    return command;
}

