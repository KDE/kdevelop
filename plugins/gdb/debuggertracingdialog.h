/*
    SPDX-FileCopyrightText: 2005 Vladimir Prus <ghost@cs.msu.su>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DEBUGGER_TRACING_DIALOG_VP_2005_08_22
#define DEBUGGER_TRACING_DIALOG_VP_2005_08_22

#include "ui_debuggertracingdialog.h"
#include <QDialog>

namespace KDevMI {
namespace GDB
{
    class Breakpoint;

    class DebuggerTracingDialog : public QDialog, public Ui::DebuggerTracingDialog
    {
        Q_OBJECT
    public:
        DebuggerTracingDialog(Breakpoint* bp, QWidget* parent);

    private Q_SLOTS:
        void enableOrDisable(int);
        void enableOrDisableCustomFormat(int);

    private:
        void accept();

    private:
        Breakpoint* bp_;
    };

} // end of namespace GDB
} // end of namespace KDevMI

#endif
