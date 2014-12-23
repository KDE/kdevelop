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

#ifndef GDBCOMMANDQUEUE_H
#define GDBCOMMANDQUEUE_H

#include <QList>

#include "gdbglobal.h"

namespace GDBDebugger
{

class GDBCommand;

class CommandQueue
{
public:
    CommandQueue();
    ~CommandQueue();

    void enqueue(GDBCommand* command, QueuePosition insertPosition);

    bool isEmpty() const;
    int count() const;
    void clear();

    /// Whether the queue contains a command with CmdImmediately or CmdInterrupt flags.
    bool haveImmediateCommand() const;

    /**
     * Retrieve and remove the next command from the list.
     */
    GDBCommand* nextCommand();

private:
    void rationalizeQueue(GDBCommand* command);
    void removeVariableUpdates();
  
    QList<GDBCommand*> m_commandList;
    int m_immediatelyCounter = 0;
    uint32_t m_tokenCounter;
};

}

#endif // GDBCOMMANDQUEUE_H
