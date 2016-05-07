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

namespace MI
{
class MICommand;
}

namespace GDBDebugger
{

class CommandQueue
{
public:
    CommandQueue();
    ~CommandQueue();

    void enqueue(MI::MICommand* command);

    bool isEmpty() const;
    int count() const;
    void clear();

    /// Whether the queue contains a command with CmdImmediately or CmdInterrupt flags.
    bool haveImmediateCommand() const;

    /**
     * Retrieve and remove the next command from the list.
     */
    MI::MICommand* nextCommand();

private:
    void rationalizeQueue(MI::MICommand* command);
    void removeVariableUpdates();
  
    QList<MI::MICommand*> m_commandList;
    int m_immediatelyCounter = 0;
    uint32_t m_tokenCounter;
};

}

#endif // GDBCOMMANDQUEUE_H
