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
#include "dbgcontroller.h"

#include <kcombobox.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>

#include <qlabel.h>
#include <qlayout.h>
#include <q3textedit.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QFocusEvent>
#include <QHBoxLayout>
#include <QBoxLayout>

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

namespace GDBDebugger
{

/***************************************************************************/

GDBOutputWidget::GDBOutputWidget( QWidget *parent, const char *name) :
    QWidget(parent, name),
    m_userGDBCmdEditor(0),
    m_Interrupt(0),
    m_gdbView(0)
{

    m_gdbView = new Q3TextEdit (this, name);
    m_gdbView->setReadOnly(true);

    QBoxLayout *userGDBCmdEntry = new QHBoxLayout();
    m_userGDBCmdEditor = new KHistoryCombo (this);

    QLabel *label = new QLabel(i18n("&GDB cmd:"), this);
    label->setBuddy(m_userGDBCmdEditor);
    userGDBCmdEntry->addWidget(label);

    userGDBCmdEntry->addWidget(m_userGDBCmdEditor);
    userGDBCmdEntry->setStretchFactor(m_userGDBCmdEditor, 1);

    m_Interrupt = new QToolButton( this, "add breakpoint" );
    m_Interrupt->setSizePolicy ( QSizePolicy ( (QSizePolicy::SizeType)0,
                                         ( QSizePolicy::SizeType)0,
                                         0,
                                         0,
                                         m_Interrupt->sizePolicy().hasHeightForWidth())
                                         );
    m_Interrupt->setPixmap ( SmallIcon ( "player_pause" ) );
    userGDBCmdEntry->addWidget(m_Interrupt);
    QToolTip::add ( m_Interrupt, i18n ( "Pause execution of the app to enter gdb commands" ) );

    QVBoxLayout *topLayout = new QVBoxLayout(this, 2);
    topLayout->addWidget(m_gdbView, 10);
    topLayout->addLayout(userGDBCmdEntry);

    slotDbgStatus( "", s_dbgNotStarted);

    connect( m_userGDBCmdEditor, SIGNAL(returnPressed()), SLOT(slotGDBCmd()) );
    connect( m_Interrupt,        SIGNAL(clicked()),       SIGNAL(breakInto()));
}

/***************************************************************************/

GDBOutputWidget::~GDBOutputWidget()
{
    delete m_gdbView;
    delete m_userGDBCmdEditor;
}

/***************************************************************************/

void GDBOutputWidget::clear()
{
    if (m_gdbView)
        m_gdbView->clear();
}

/***************************************************************************/

void GDBOutputWidget::slotReceivedStdout(const char* line)
{
    if (strncmp(line, "(gdb) ", 5) == 0)
        m_gdbView->append(QString("<font color=\"blue\">").append( line ).append("</font>") );
    else
        m_gdbView->append(line);
}

/***************************************************************************/

void GDBOutputWidget::slotReceivedStderr(const char* line)
{
    m_gdbView->append(QString("<font color=\"red\">").append( line ).append("</font>") );
}

/***************************************************************************/

void GDBOutputWidget::slotGDBCmd()
{
    QString GDBCmd(m_userGDBCmdEditor->currentText());
    if (!GDBCmd.isEmpty())
    {
        m_userGDBCmdEditor->addToHistory(GDBCmd);
        m_userGDBCmdEditor->clearEdit();
        emit userGDBCmd(GDBCmd);
    }
}

/***************************************************************************/

void GDBOutputWidget::slotDbgStatus(const QString &, int statusFlag)
{
    if (statusFlag & s_dbgNotStarted)
    {
        m_Interrupt->setEnabled(false);
        m_userGDBCmdEditor->setEnabled(false);
        return;
    }

    if (statusFlag & s_appBusy)
    {
        m_Interrupt->setEnabled(true);
        m_userGDBCmdEditor->setEnabled(false);
    }
    else
    {
        m_Interrupt->setEnabled(false);
        m_userGDBCmdEditor->setEnabled(true);
    }
}

/***************************************************************************/

void GDBOutputWidget::focusInEvent(QFocusEvent */*e*/)
{
    m_userGDBCmdEditor->setFocus();
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
}


#include "gdboutputwidget.moc"

