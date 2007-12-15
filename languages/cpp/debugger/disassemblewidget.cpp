/*
 * GDB Debugger Support
 *
 * Copyright 2000 John Birch <jbb@kdevelop.org>
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
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

#include "disassemblewidget.h"
#include "gdbcontroller.h"
#include "gdbcommand.h"
#include "debuggerplugin.h"

#include <kdebug.h>
#include <kdeversion.h>
#include <ktextedit.h>
#include <kglobalsettings.h>

#include <QShowEvent>
#include <QHideEvent>

#include <stdlib.h>
#include <klocale.h>
#include <KIcon>

using namespace GDBMI;

namespace GDBDebugger
{

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

DisassembleWidget::DisassembleWidget(CppDebuggerPlugin* plugin, GDBController* controller, QWidget *parent)
        : QTreeWidget(parent), controller_(controller),
        active_(false),
        lower_(0),
        upper_(0),
        address_(0)
{
    setToolTip(i18n("<b>Machine code display</b><p>"
                    "A machine code view into your running "
                    "executable with the current instruction "
                    "highlighted. You can step instruction by "
                    "instruction using the debuggers toolbar "
                    "buttons of \"step over\" instruction and "
                    "\"step into\" instruction."));
    setWindowIcon( KIcon("gear") );
    setWindowTitle(i18n("Disassemble View"));
    setFont(KGlobalSettings::fixedFont());
    setSelectionMode(SingleSelection);
    setColumnCount(ColumnCount);

    setHeaderLabels(QStringList() << i18n("Address") << i18n("Function") << i18n("Offset") << i18n("Instruction"));

    connect( controller,  SIGNAL(showStepInSource(const QString&, int, const QString&)),
             this,        SLOT(slotShowStepInSource(const QString&, int, const QString&)));

    connect(plugin, SIGNAL(reset()), this, SLOT(clear()));
    connect(plugin, SIGNAL(reset()), this, SLOT(slotDeactivate()));
}

/***************************************************************************/

DisassembleWidget::~DisassembleWidget()
{}

/***************************************************************************/

bool DisassembleWidget::displayCurrent()
{
    Q_ASSERT(address_ >= lower_ || address_ <= upper_);

    int line;
    for (line=0; line < topLevelItemCount(); line++)
    {
        QTreeWidgetItem* item = topLevelItem(line);
        unsigned long address = strtoul(item->text(Address).toLatin1(), 0, 0);
        if (address == address_)
        {
            // put cursor at start of line and highlight the line
            setCurrentItem(item);
            selectionModel()->select(indexFromItem(item), QItemSelectionModel::Select);
            return true;
        }
    }

    return false;
}

/***************************************************************************/

void DisassembleWidget::slotActivate(bool activate)
{
    kDebug(9012) << "Disassemble widget active: " << activate;

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
    kDebug(9012) << "DisasssembleWidget::slotShowStepInSource()";

    currentAddress_ = currentAddress;
    address_ = strtoul(currentAddress.toLatin1(), 0, 0);
    if (!active_)
        return;

    if (address_ < lower_ || address_ > upper_ || !displayCurrent())
        getNextDisplay();
}

/***************************************************************************/

void DisassembleWidget::getNextDisplay()
{
    kDebug(9012) << "DisasssembleWidget::getNextDisplay()";

    if (address_)
    {
        Q_ASSERT(!currentAddress_.isNull());

        QString cmd = QString("-s $pc -e \"$pc + 128\" -- 0");
        controller_->addCommandToFront( 
                        new GDBCommand(DataDisassemble, cmd, this, &DisassembleWidget::memoryRead ) );
    }
}

/***************************************************************************/

void DisassembleWidget::memoryRead(const GDBMI::ResultRecord& r)
{
  const GDBMI::Value& content = r["asm_insns"];
  QString rawdata;

  clear();

  for(int i = 0; i < content.size(); ++i)
  {
    const GDBMI::Value& line = content[i];

    QString addr = line["address"].literal();
    QString fct = line["func-name"].literal();
    QString offs = line["offset"].literal();
    QString inst = line["inst"].literal();

    addTopLevelItem(new QTreeWidgetItem(this, QStringList() << addr << fct << offs << inst));

    if (i == 0) {
      lower_ = strtoul(addr.toLatin1(), 0, 0);
    } else  if (i == content.size()-1) {
      upper_ = strtoul(addr.toLatin1(), 0, 0);
    }
  }

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

void DisassembleWidget::slotDeactivate()
{
    slotActivate(false);
}

/***************************************************************************/

}

#include "disassemblewidget.moc"
