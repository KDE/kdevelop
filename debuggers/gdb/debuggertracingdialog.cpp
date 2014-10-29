/*
 * Dialog for configuring breakpoint tracing.
 *
 * Copyright 2006 Vladimir Prus <ghost@cs.msu.su>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "debuggertracingdialog.h"
#include "breakpoint.h"

#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <keditlistbox.h>
#include <kmessagebox.h>

/* WARNING: this code was not yet ported to KDevelop4 and is unused, but is 
   intended to be ported.  */

namespace GDBDebugger
{

    DebuggerTracingDialog
    ::DebuggerTracingDialog(Breakpoint* bp,
                            QWidget* parent)
    : QDialog(parent), bp_(bp)
    {
        setupUi(this);

        expressions->setButtons(KEditListBox::Add | KEditListBox::Remove);

        connect(enable, SIGNAL(stateChanged(int)),
                this, SLOT(enableOrDisable(int)));        

        connect(enableCustomFormat, SIGNAL(stateChanged(int)),
                this, SLOT(enableOrDisableCustomFormat(int)));

        enable->setChecked(bp_->tracingEnabled());
        expressions->setItems(bp_->tracedExpressions());
        enableCustomFormat->setChecked(bp_->traceFormatStringEnabled());
        customFormat->setText(bp_->traceFormatString());

        enableOrDisable(enable->isChecked());

        // Go away if the breakpoint does
        connect(bp_, SIGNAL(destroyed(QObject*)), this, SLOT(reject()));
    }

    void DebuggerTracingDialog::enableOrDisable(int state)
    {
        bool enable = (state == Qt::Checked);

        expressionsLabel->setEnabled(enable);
        expressions->setEnabled(enable);
        enableCustomFormat->setEnabled(enable);
        customFormat->setEnabled(enable && enableCustomFormat->isChecked());
    }

    void DebuggerTracingDialog::enableOrDisableCustomFormat(int state)
    {
        customFormat->setEnabled(state == Qt::Checked);
    }

    void DebuggerTracingDialog::accept()
    {
        // Check that if we use format string,
        // the number of expression is not larget than the number of
        // format specifiers
        bool ok = true;

        if (enableCustomFormat->isChecked())
        {
            QString s = customFormat->text();
            int percent_count = 0;
            for (int i = 0; i < s.length(); ++i)
                if (s[i] == '%')
                {
                    if (i+1 < s.length())
                    {
                        if (s[i+1] != '%')
                        {
                            ++percent_count;
                        }
                        else
                        {
                            // Double %
                            ++i;
                        }
                    }
                }

            if (percent_count < expressions->items().count())
            {
                ok = false;

                KMessageBox::error(
                    this,
                    "<b>Not enough format specifiers</b>"
                    "<p>The number of format specifiers in the custom format "
                    "string is less than the number of expressions. Either remove "
                    "some expressions or edit the format string.",
                    "Not enough format specifiers");
            }
        }

        if (ok)
        {
            bp_->setTracingEnabled(enable->isChecked());
            bp_->setTracedExpressions(expressions->items());
            bp_->setTraceFormatStringEnabled(enableCustomFormat->isChecked());
            bp_->setTraceFormatString(customFormat->text());
            QDialog::accept();
        }
    }

}
