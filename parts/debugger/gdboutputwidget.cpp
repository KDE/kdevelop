// *************************************************************************
//                          gdboutputwidget.cpp  -  description
//                             -------------------
//    begin                : 10th April 2003
//    copyright            : (C) 2003 by John Birch
//    email                : jbb@kdevelop.org
// **************************************************************************
//
// **************************************************************************
// *                                                                        *
// *   This program is free software; you can redistribute it and/or modify *
// *   it under the terms of the GNU General Public License as published by *
// *   the Free Software Foundation; either version 2 of the License, or    *
// *   (at your option) any later version.                                  *
// *                                                                        *
// **************************************************************************

#include "gdboutputwidget.h"

#include <kcombobox.h>
#include <kdebug.h>
#include <klocale.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qtextedit.h>

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

namespace GDBDebugger
{

/***************************************************************************/

GDBOutputWidget::GDBOutputWidget( QWidget *parent, const char *name) :
    QWidget(parent, name),
    gdbView_(0),
    userGDBCmdEditor_(0)
{

    gdbView_ = new QTextEdit (this, name);
    gdbView_->setReadOnly(true);

    QBoxLayout *userGDBCmdEntry = new QHBoxLayout();
    userGDBCmdEditor_ = new KHistoryCombo (this, "gdb-user-cmd-editor");

    QLabel *label = new QLabel(i18n("GDB Cmd:"), this);
    userGDBCmdEntry->addWidget(label);

    userGDBCmdEntry->addWidget(userGDBCmdEditor_);
    userGDBCmdEntry->setStretchFactor(userGDBCmdEditor_, 1);

    QVBoxLayout *topLayout = new QVBoxLayout(this, 2);
    topLayout->addWidget(gdbView_, 10);
    topLayout->addLayout(userGDBCmdEntry);

    connect( userGDBCmdEditor_, SIGNAL(returnPressed()), SLOT(slotGDBCmd()) );
}

/***************************************************************************/

GDBOutputWidget::~GDBOutputWidget()
{
    delete gdbView_;
    delete userGDBCmdEditor_;
}

/***************************************************************************/

void GDBOutputWidget::clear()
{
    if (gdbView_)
        gdbView_->clear();
}

/***************************************************************************/

void GDBOutputWidget::slotReceivedStdout(const char* line)
{
    if (strncmp(line, "(gdb) ", 5) == 0)
        gdbView_->append(QString("<font color=\"blue\">").append( line ).append("</font>") );
    else
        gdbView_->append(line);
}

/***************************************************************************/

void GDBOutputWidget::slotReceivedStderr(const char* line)
{
    gdbView_->append(QString("<font color=\"red\">").append( line ).append("</font>") );
}

/***************************************************************************/

void GDBOutputWidget::slotGDBCmd()
{
    QString GDBCmd(userGDBCmdEditor_->currentText());
    if (!GDBCmd.isEmpty())
    {
        userGDBCmdEditor_->clearEdit();
        userGDBCmdEditor_->addToHistory(GDBCmd);
        emit userGDBCmd(GDBCmd);
    }
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
}

#include "gdboutputwidget.moc"
