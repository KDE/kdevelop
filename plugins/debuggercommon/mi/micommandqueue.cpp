/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "micommandqueue.h"

#include "mi.h"
#include "micommand.h"
#include "debuglog.h"

using namespace KDevMI::MI;

CommandQueue::CommandQueue() = default;
CommandQueue::~CommandQueue() = default;

void CommandQueue::enqueue(std::unique_ptr<MICommand> command)
{
    ++m_tokenCounter;
    if (m_tokenCounter == 0)
        m_tokenCounter = 1;
    command->setToken(m_tokenCounter);

    // take the time when this command was added to the command queue
    command->markAsEnqueued();

    if (command->flags() & (CmdImmediately | CmdInterrupt))
        ++m_immediatelyCounter;

    m_commandList.push_back(std::move(command));

    rationalizeQueue(m_commandList.back().get());
    dumpQueue();
}

void CommandQueue::dumpQueue() const
{
    qCDebug(DEBUGGERCOMMON) << "Pending commands" << m_commandList.size();
    unsigned commandNum = 0;
    for (const auto& command : m_commandList) {
        qCDebug(DEBUGGERCOMMON) << "Command" << commandNum << command->initialString();
        ++commandNum;
    }
}

void CommandQueue::rationalizeQueue(MICommand* command)
{
    if ((command->type() >= ExecAbort && command->type() <= ExecUntil) &&
         command->type() != ExecArguments ) {
        // Changing execution location, abort any variable updates and stack list updates

        auto predicate = [this](const auto& command) {
            const auto type = command->type();
            const auto isVariableUpdate
                = (type >= VarEvaluateExpression && type <= VarListChildren) || type == VarUpdate;
            const auto isStackListUpdate = type >= StackListArguments && type <= StackListLocals;
            if (isVariableUpdate || isStackListUpdate) {
                if (command->flags() & (CmdImmediately | CmdInterrupt))
                    --m_immediatelyCounter;
                return true;
            }
            return false;
        };
        m_commandList.erase(std::remove_if(m_commandList.begin(), m_commandList.end(), predicate), m_commandList.end());
    }
}

void CommandQueue::clear()
{
    m_commandList.clear();
    m_immediatelyCounter = 0;
}

int CommandQueue::count() const
{
    return m_commandList.size();
}

bool CommandQueue::isEmpty() const
{
    return m_commandList.empty();
}

bool CommandQueue::haveImmediateCommand() const
{
    return m_immediatelyCounter > 0;
}

std::unique_ptr<MICommand> CommandQueue::nextCommand()
{
    if (m_commandList.empty())
        return {};

    auto command = std::exchange(m_commandList.front(), {});
    m_commandList.pop_front();

    if (command->flags() & (CmdImmediately | CmdInterrupt))
        --m_immediatelyCounter;

    return command;
}

