/***************************************************************************
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
#include <kdeversion.h>
#if KDE_VERSION > 305
# include <ktextedit.h>
#endif
#include <kglobalsettings.h>

#include <qdict.h>
#include <qheader.h>
#include <qtextedit.h>

#include <stdlib.h>

namespace GDBDebugger
{

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

DisassembleWidget::DisassembleWidget(QWidget *parent, const char *name)
        : QTextEdit(parent, name),
        active_(false),
        lower_(0),
        upper_(0),
        address_(0)
{
    setFont(KGlobalSettings::fixedFont());
    setReadOnly(true);
}

/***************************************************************************/

DisassembleWidget::~DisassembleWidget()
{}

/***************************************************************************/

bool DisassembleWidget::displayCurrent()
{
    Q_ASSERT(address_ >= lower_ || address_ <= upper_);

    int line;
    for (line=0; line < paragraphs(); line++)
    {
        int address = strtol(text(line).latin1(), 0, 0);
        if (address == address_)
        {
            // put cursor at start of line and highlight the line
            setCursorPosition(line, 0);
            setSelection(line,0,line+1,0,0);
            return true;
        }
    }

    return false;
}

/***************************************************************************/

void DisassembleWidget::slotBPState(const Breakpoint&)
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
    if (start)
    {
        append(start+1);
        // Skip the last two lines (just trailer info)
        removeParagraph(paragraphs()-1);
        removeParagraph(paragraphs()-1);

        if (paragraphs())
        {
            lower_ = strtol(text(0).latin1(), 0, 0);
            upper_ = strtol(text(paragraphs()-1).latin1(), 0, 0);
            displayCurrent();
        }
        else
        {
            lower_ = 0;
            upper_ = 0;
        }
    }
}

/***************************************************************************/

void DisassembleWidget::slotActivate(bool activate)
{
    kdDebug(9012) << "Disassemble widget active: " << activate << endl;

    if (active_ != activate)
    {
        active_ = activate;
        if (active_ && address_)
        {
            if (address_ < lower_ || address_ > upper_ || !displayCurrent())
                getNextDisplay();
        }
    }
}

/***************************************************************************/

void DisassembleWidget::slotShowStepInSource(   const QString &, int,
                                                const QString &currentAddress)
{
    kdDebug(9012) << "DisasssembleWidget::slotShowStepInSource()" << endl;

    currentAddress_ = currentAddress;
    address_ = strtol(currentAddress.latin1(), 0, 0);
    if (!active_)
        return;

    if (address_ < lower_ || address_ > upper_ || !displayCurrent())
        getNextDisplay();
}

/***************************************************************************/

void DisassembleWidget::getNextDisplay()
{
    kdDebug(9012) << "DisasssembleWidget::getNextDisplay()" << endl;

    if (address_)
    {
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
