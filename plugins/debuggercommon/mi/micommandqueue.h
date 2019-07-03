// *************************************************************************
//                          micommandqueue.cpp
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

#ifndef MICOMMANDQUEUE_H
#define MICOMMANDQUEUE_H

#include "dbgglobal.h"

#include <QList>

namespace KDevMI { namespace MI {

class MICommand;

class CommandQueue
{
public:
    CommandQueue();
    ~CommandQueue();

    /// CommandQueue takes ownership of @p command.
    void enqueue(MICommand* command);

    bool isEmpty() const;
    int count() const;
    void clear();

    /// Whether the queue contains a command with CmdImmediately or CmdInterrupt flags.
    bool haveImmediateCommand() const;

    /**
     * Retrieve and remove the next command from the list.
     * Ownership of the command is transferred to the caller.
     * Returns @c nullptr if the list is empty.
     */
    MICommand* nextCommand();

private:
    void rationalizeQueue(MICommand* command);
    void removeVariableUpdates();
    void removeStackListUpdates();
    void dumpQueue() const;

private:
    Q_DISABLE_COPY(CommandQueue)

    QList<MICommand*> m_commandList;
    int m_immediatelyCounter = 0;
    uint32_t m_tokenCounter = 0;
};

} // end of namespace MI
} // end of namespace KDevMI

#endif // MICOMMANDQUEUE_H
