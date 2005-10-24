// **************************************************************************
// *                                                                        *
// * Copyright 2005 by Vladimir Prus <ghost@cs.msu.su>                      *
// *   This program is free software; you can redistribute it and/or modify *
// *   it under the terms of the GNU General Public License as published by *
// *   the Free Software Foundation; either version 2 of the License, or    *
// *   (at your option) any later version.                                  *
// *                                                                        *
// **************************************************************************

#ifndef CALLBACKS_VP_2005_09_08
#define CALLBACKS_VP_2005_09_08

/** This file defines abstract classes which are used by the gdb controller
    the send data to the interested parties.
    When asking gdbcontroller, we provide an instance of a class derived
    from interfaces, and gdb will eventually calls us back.
*/

namespace GDBDebugger
{
    class MemoryCallback
    {
    public:
        virtual void memoryContentAvailable(unsigned start, unsigned amount,
                                            char* data) = 0;
    };

    class ValueCallback
    {
    public:
        virtual void updateValue(char* buf) = 0;
    };

}


#endif
