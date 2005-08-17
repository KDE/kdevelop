/***************************************************************************
                          disassemble.cpp  -  description
                             -------------------
    begin                : Tues Jan 3 2000
    copyright            : (C) 2000 by John Birch
    email                : jbb@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "disassemblewidget.h"

#include <kdebug.h>
#include <keditcl.h>
#include <kglobalsettings.h>

#include <q3dict.h>
#include <q3header.h>
//Added by qt3to4:
#include <QHideEvent>
#include <QShowEvent>

#include <stdlib.h>

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

namespace JAVADebugger
{

DisassembleWidget::DisassembleWidget(QWidget *parent, const char *name)
    : KEdit(parent, name),
      active_(false),
      lower_(0),
      upper_(0),
      address_(0)
{
    setFont(KGlobalSettings::fixedFont());
}

/***************************************************************************/

DisassembleWidget::~DisassembleWidget()
{
}

/***************************************************************************/

bool DisassembleWidget::displayCurrent()
{
    Q_ASSERT(address_ >= lower_ || address_ <= upper_);

    int line;
    for (line=0; line < numLines(); line++) {
        if (textLine(line).startsWith(currentAddress_)) {
            // put cursor at start of line and highlight the line
            setCursorPosition(line+1, 0);
            setCursorPosition(line, 0, true);
            return true;;
        }
    }

    return false;
}

/***************************************************************************/

void DisassembleWidget::slotBPState(const Breakpoint *)
{
    if (!active_)
        return;
}

/***************************************************************************/

void DisassembleWidget::slotDisassemble(char *buf)
{
    if (!active_)
        return;

    clear();
    // Skip the first line (just header info)
    char *start = strchr(buf, '\n');

    // Make sure there is something there
    if (start) {
        insertLine(start+1);
        // Skip the last two lines (just trailer info)
        removeLine(numLines()-1);
        removeLine(numLines()-1);

        if (numLines()) {
            lower_ = strtol(textLine(0).latin1(), 0, 0);
            upper_ = strtol(textLine(numLines()-1).latin1(), 0, 0);
            displayCurrent();
        } else {
            lower_ = 0;
            upper_ = 0;
        }
    }
}

/***************************************************************************/

void DisassembleWidget::slotActivate(bool activate)
{
    kdDebug(9012) << "Disassemble widget active: " << activate << endl;

    if (active_ != activate) {
        active_ = activate;
        if (active_)
            getNextDisplay();
    }
}

/***************************************************************************/

void DisassembleWidget::slotShowStepInSource(const QString &, int,
                                             const QString &currentAddress)
{
    kdDebug(9012) << "DisasssembleWidget::slotShowStepInSource()" << endl;

    currentAddress_ = currentAddress;
    const char *t = currentAddress_.latin1();
    address_ = strtol(t, 0, 0);
    if (!active_)
        return;

    if (address_ < lower_ || address_ > upper_ || !displayCurrent())
        getNextDisplay();
}

/***************************************************************************/

void DisassembleWidget::getNextDisplay()
{
    kdDebug(9012) << "DisasssembleWidget::getNextDisplay()" << endl;

    if (address_) {
        Q_ASSERT(!currentAddress_.isNull());

        // restrict this to a managable size - some functions are _big_
        QString endAddress;
        endAddress.sprintf("0x%x", (uint)address_+128);
        emit disassemble(currentAddress_, endAddress);
    }
    else
        emit disassemble("", "");
}


void DisassembleWidget::showEvent(QShowEvent*)
{
    slotActivate(true);
}


void DisassembleWidget::hideEvent(QHideEvent*)
{
    slotActivate(false);
}

/***************************************************************************/
}

#include "disassemblewidget.moc"
