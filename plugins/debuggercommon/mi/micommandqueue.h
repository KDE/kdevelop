/*
    SPDX-FileCopyrightText: 2007 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MICOMMANDQUEUE_H
#define MICOMMANDQUEUE_H

#include "dbgglobal.h"

#include <memory>
#include <deque>

namespace KDevMI { namespace MI {

class MICommand;

class CommandQueue
{
public:
    CommandQueue();
    ~CommandQueue();

    void enqueue(std::unique_ptr<MICommand> command);

    bool isEmpty() const;
    int count() const;
    void clear();

    /// Whether the queue contains a command with CmdImmediately or CmdInterrupt flags.
    bool haveImmediateCommand() const;

    /**
     * Retrieve and remove the next command from the list.
     * Returns @c nullptr if the list is empty.
     */
    std::unique_ptr<MICommand> nextCommand();

private:
    void rationalizeQueue(MICommand* command);
    void dumpQueue() const;

private:
    Q_DISABLE_COPY(CommandQueue)

    std::deque<std::unique_ptr<MICommand>> m_commandList;
    int m_immediatelyCounter = 0;
    uint32_t m_tokenCounter = 0;
};

} // end of namespace MI
} // end of namespace KDevMI

#endif // MICOMMANDQUEUE_H
