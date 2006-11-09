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
#include "gdbcontroller.h"
#include "gdbcommand.h"

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

DisassembleWidget::DisassembleWidget(GDBController* controller, QWidget *parent, const char *name)
        : QTextEdit(parent, name), controller_(controller),
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
        unsigned long address = strtoul(text(line).latin1(), 0, 0);
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
    address_ = strtoul(currentAddress.latin1(), 0, 0);
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

        QString cmd = QString("-data-disassemble -s $pc -e \"$pc + 128\" -- 0");
        controller_->addCommandToFront( 
                        new GDBCommand( cmd, this, &DisassembleWidget::memoryRead ) );
    }
}

/***************************************************************************/

void DisassembleWidget::memoryRead(const GDBMI::ResultRecord& r)
{
  const GDBMI::Value& content = r["asm_insns"];
  QString rawdata;

  clear();

  for(unsigned i = 0; i < content.size(); ++i)
  {
    const GDBMI::Value& line = content[i];

    QString addr = line["address"].literal();
    QString fct = line["func-name"].literal();
    QString offs = line["offset"].literal();
    QString inst = line["inst"].literal();

    rawdata += QString(addr + "  " + fct+"+"+offs + "    " + inst + "\n");

    if (i == 0) {
      lower_ = strtoul(addr.latin1(), 0, 0);
    } else  if (i == content.size()-1) {
      upper_ = strtoul(addr.latin1(), 0, 0);
    }
  }

  append(rawdata);

  displayCurrent();
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
