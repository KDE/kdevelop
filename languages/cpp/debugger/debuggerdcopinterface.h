/***************************************************************************
 *   Copyright (C) 2003 Hamish Rodda <rodda@kde.org>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DEBUGGERDCOPINTERFACE_H
#define DEBUGGERDCOPINTERFACE_H

#include <dcopobject.h>

/**
 * Enables dcop communication with the debugger part.
 */
class DebuggerDCOPInterface : public DCOPObject
{
    K_DCOP
public:

k_dcop:
    virtual ASYNC slotDebugExternalProcess() = 0;
};

#endif
