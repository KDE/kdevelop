/***************************************************************************
    begin                : Tue Oct 5 1999
    copyright            : (C) 1999 by John Birch
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

#include "memviewdlg.h"

#include <kbuttonbox.h>
#include <klineedit.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kstdguiitem.h>
#include <kguiitem.h>
#include <kdeversion.h>

#include <qlabel.h>
#include <qlayout.h>
#include <q3multilineedit.h>
#include <qpushbutton.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QGridLayout>
#include <QBoxLayout>

// **************************************************************************
//
// Dialog allows the user to enter
//  - A starting address
//  - An ending address
//
//  this can be in the form
//            functiom/method name
//            variable address (ie &Var, str)
//            Memory address 0x8040abc
//
//  When disassembling and you enter a method name without an
//  ending address then the whole method is disassembled.
//  No data means disassemble the method we're curently in.(from the
//  start of the method)
//
// click ok buton to send the request to gdb
// the output is returned (some time later) in the raw data slot
// and displayed as is, so it's rather crude, but it works!
// **************************************************************************

namespace GDBDebugger
{

MemoryViewDialog::MemoryViewDialog(QWidget *parent, const char *name)
    : KDialog(parent, QString(name), 0, Qt::Dialog),      // modal
      start_(new KLineEdit(this)),
      end_(new KLineEdit(this)),
      output_(new Q3MultiLineEdit(this))
{
    setCaption(i18n("Memory/Misc Viewer"));
    // Make the top-level layout; a vertical box to contain all widgets
    // and sub-layouts.
    QBoxLayout *topLayout = new QVBoxLayout(this, 5);

    QGridLayout *grid = new QGridLayout(2, 2, 5);
    topLayout->addLayout(grid);

    QLabel *label = new QLabel(start_, i18n("&Start:"), this);
    label->setBuddy(start_);
    grid->addWidget(label, 0, 0);
    grid->setRowStretch(0, 0);
    grid->addWidget(start_, 1, 0);
    grid->setRowStretch(1, 0);

    label = new QLabel(end_, i18n("Amount/&End address (memory/disassemble):"), this);
    label->setBuddy(end_);
    grid->addWidget(label, 0, 1);
    grid->addWidget(end_, 1, 1);

    label = new QLabel(i18n("Memory&View:"), this);
    label->setBuddy(output_);
    topLayout->addWidget(label, 0);
    topLayout->addWidget(output_, 5);
    output_->setFont(KGlobalSettings::fixedFont());

    KButtonBox *buttonbox = new KButtonBox(this, Qt::Horizontal, 5);
    QPushButton *memoryDump = buttonbox->addButton(i18n("&Memory"));
    QPushButton *disassemble = buttonbox->addButton(i18n("&Disassemble"));
    QPushButton *registers = buttonbox->addButton(i18n("&Registers"));
    QPushButton *libraries = buttonbox->addButton(i18n("&Libraries"));
    QPushButton *cancel = buttonbox->addButton(KStdGuiItem::cancel());
    memoryDump->setDefault(true);
    buttonbox->layout();
    topLayout->addWidget(buttonbox);

    start_->setFocus();

    connect(memoryDump, SIGNAL(clicked()), SLOT(slotMemoryDump()));
    connect(disassemble, SIGNAL(clicked()), SLOT(slotDisassemble()));
    connect(registers, SIGNAL(clicked()), SIGNAL(registers()));
    connect(libraries, SIGNAL(clicked()), SIGNAL(libraries()));
    connect(cancel, SIGNAL(clicked()), SLOT(reject()));
}

// **************************************************************************

MemoryViewDialog::~MemoryViewDialog()
{
}

// **************************************************************************

void MemoryViewDialog::slotRawGDBMemoryView(char *buf)
{
    // just display the resultant output from GDB in the edit box
    output_->clear();
    output_->insertLine(buf);
    output_->setCursorPosition(0,0);
}

// **************************************************************************

// get gdb to supply the disassembled data.
void MemoryViewDialog::slotDisassemble()
{
    QString start(start_->text());
    QString end(end_->text());
    emit disassemble(start, end);
}

// **************************************************************************

void MemoryViewDialog::slotMemoryDump()
{
    QString start(start_->text());
    QString size(end_->text());
    emit memoryDump(start, size);
}

// **************************************************************************
// **************************************************************************
// **************************************************************************

}

#include "memviewdlg.moc"
