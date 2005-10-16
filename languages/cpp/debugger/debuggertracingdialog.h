/***************************************************************************
 *   Copyright (C) 2005 by Vladimir Prus                                   *
 *   ghost@cs.msu.su                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef DEBUGGER_TRACING_DIALOG_VP_2005_08_22
#define DEBUGGER_TRACING_DIALOG_VP_2005_08_22

#include "debuggertracingdialogbase.h"


namespace GDBDebugger
{
    class Breakpoint;

    class DebuggerTracingDialog : public DebuggerTracingDialogBase
    {
        Q_OBJECT
    public:
        DebuggerTracingDialog(Breakpoint* bp,
                              QWidget* parent, const char* name = 0);

    private slots:
        void enableOrDisable(int);
        void enableOrDisableCustomFormat(int);

    private:
        void accept();

    private:
        Breakpoint* bp_;
    };
}

#endif
