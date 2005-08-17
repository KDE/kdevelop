// *************************************************************************
//                          rdboutputwidget.cpp  -  description
//                             -------------------
//    begin                : 10th April 2003
//    copyright            : (C) 2003 by John Birch
//    email                : jbb@kdevelop.org
//	
//                          Adapted for ruby debugging
//                          --------------------------
//    begin                : Mon Nov 1 2004
//   copyright            : (C) 2004 by Richard Dale
//    email                : Richard_Dale@tipitina.demon.co.uk
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

#include "rdboutputwidget.h"
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

namespace RDBDebugger
{

/***************************************************************************/

RDBOutputWidget::RDBOutputWidget( QWidget *parent, const char *name) :
    QWidget(parent, name),
    m_userRDBCmdEditor(0),
    m_Interrupt(0),
    m_rdbView(0)
{

    m_rdbView = new Q3TextEdit (this, name);
    m_rdbView->setReadOnly(true);

    QBoxLayout *userRDBCmdEntry = new QHBoxLayout();
    m_userRDBCmdEditor = new KHistoryCombo (this, "rdb-user-cmd-editor");

    QLabel *label = new QLabel(i18n("&RDB cmd:"), this);
    label->setBuddy(m_userRDBCmdEditor);
    userRDBCmdEntry->addWidget(label);

    userRDBCmdEntry->addWidget(m_userRDBCmdEditor);
    userRDBCmdEntry->setStretchFactor(m_userRDBCmdEditor, 1);

    m_Interrupt = new QToolButton( this, "add breakpoint" );
    m_Interrupt->setSizePolicy ( QSizePolicy ( (QSizePolicy::SizeType)0,
                                         ( QSizePolicy::SizeType)0,
                                         0,
                                         0,
                                         m_Interrupt->sizePolicy().hasHeightForWidth())
                                         );
    m_Interrupt->setPixmap ( SmallIcon ( "player_pause" ) );
    userRDBCmdEntry->addWidget(m_Interrupt);
    QToolTip::add ( m_Interrupt, i18n ( "Pause execution of the app to enter rdb commands" ) );

    QVBoxLayout *topLayout = new QVBoxLayout(this, 2);
    topLayout->addWidget(m_rdbView, 10);
    topLayout->addLayout(userRDBCmdEntry);

    slotDbgStatus( "", s_dbgNotStarted);

    connect( m_userRDBCmdEditor, SIGNAL(returnPressed()), SLOT(slotRDBCmd()) );
    connect( m_Interrupt,        SIGNAL(clicked()),       SIGNAL(breakInto()));
}

/***************************************************************************/

RDBOutputWidget::~RDBOutputWidget()
{
    delete m_rdbView;
    delete m_userRDBCmdEditor;
}

/***************************************************************************/

void RDBOutputWidget::clear()
{
    if (m_rdbView)
        m_rdbView->clear();
}

/***************************************************************************/

void RDBOutputWidget::slotReceivedStdout(const char* line)
{
    if (strncmp(line, "(rdb:", 5) == 0)
        m_rdbView->append(QString("<font color=\"blue\">").append( line ).append("</font>") );
    else
        m_rdbView->append(line);
}

/***************************************************************************/

void RDBOutputWidget::slotReceivedStderr(const char* line)
{
    m_rdbView->append(QString("<font color=\"red\">").append( line ).append("</font>") );
}

/***************************************************************************/

void RDBOutputWidget::slotRDBCmd()
{
    QString RDBCmd(m_userRDBCmdEditor->currentText());
    if (!RDBCmd.isEmpty())
    {
        m_userRDBCmdEditor->addToHistory(RDBCmd);
        m_userRDBCmdEditor->clearEdit();
        emit userRDBCmd(RDBCmd);
    }
}

/***************************************************************************/

void RDBOutputWidget::slotDbgStatus(const QString &, int statusFlag)
{
    if (statusFlag & s_dbgNotStarted)
    {
        m_Interrupt->setEnabled(false);
        m_userRDBCmdEditor->setEnabled(false);
        return;
    }

    if (statusFlag & s_appBusy)
    {
        m_Interrupt->setEnabled(true);
        m_userRDBCmdEditor->setEnabled(false);
    }
    else
    {
        m_Interrupt->setEnabled(false);
        m_userRDBCmdEditor->setEnabled(true);
    }
}

/***************************************************************************/

void RDBOutputWidget::focusInEvent(QFocusEvent */*e*/)
{
    m_userRDBCmdEditor->setFocus();
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
}


#include "rdboutputwidget.moc"

