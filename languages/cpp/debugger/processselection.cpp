/***************************************************************************
    begin                : Mon Sep 20 1999
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

#include "processselection.h"

#include <kglobalsettings.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <klistwidgetsearchline.h>


#include <QLabel>
#include <QLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include <QRegExp>
#include <QFocusEvent>

#include <kprocess.h>
#include <kshell.h>

#include <util/commandexecutor.h>

#include <unistd.h>
#include <sys/types.h>

#include "ui_dbgpsdlg.h"

namespace GDBDebugger
{

/***************************************************************************/

// Display a list of processes for the user to select one
// only display processes that they can do something with so if the user
// is root then display all processes
// For use with the internal debugger, but this dialog doesn't know anything
// about why it's doing it.

ProcessSelectionDialog::ProcessSelectionDialog(QWidget *parent)
    : KDialog(parent),      // modal
      psProc(0)
{
    m_ui = new Ui::ProcessSelection();
    m_ui->setupUi(mainWidget());
    m_ui->search->searchLine()->setTreeWidget( m_ui->pids );
    // Maybe allow to search for other things than command?
    m_ui->search->searchLine()->setSearchColumns( QList<int>() << 4 );
    setCaption(i18n("Attach to Process"));
    setButtons( KDialog::Ok | KDialog::Cancel );

    QStringList cmd;
    psProc = new KDevelop::CommandExecutor("ps");
#ifdef USE_SOLARIS
    cmt << "-opid";
    cmd << "-otty";
    cmd << "-os";
    cmd << "-otime";
    cmd << "-oargs";
    pidCmd = "ps -opid -otty -os -otime -oargs";

    if (getuid() == 0) {
        cmd << "-e";
        pidCmd += " -e";
    }
#else
    cmd << "x";
    pidCmd = "ps x";

    if (getuid() == 0) {
        cmd << "a";
        pidCmd += " a";
    }
#endif

    psProc->setArguments(cmd);
    connect( psProc, SIGNAL(completed()),
             SLOT(slotProcessExited()) );
    connect( psProc, SIGNAL(failed()),
             SLOT(slotProcessExited()) );
    connect( psProc, SIGNAL(receivedStandardOutput(const QStringList&)),
             SLOT(slotReceivedOutput(const QStringList&)) );

    psProc->start();

}

/***************************************************************************/

ProcessSelectionDialog::~ProcessSelectionDialog()
{
    delete psProc;
}

/***************************************************************************/

int ProcessSelectionDialog::pidSelected()
{
	return m_ui->pids->currentItem()->text(0).toInt();
}


/***************************************************************************/

void ProcessSelectionDialog::slotReceivedOutput(const QStringList& lines)
{
    static QRegExp ps_output_line("^\\s*(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(.+)");
    QStringList tmp = lines.filter(ps_output_line);
    foreach( QString line, lines )
    {
        ps_output_line.exactMatch( line );
        new QTreeWidgetItem(m_ui->pids,
                            QStringList()
                                << ps_output_line.cap(1)
                                << ps_output_line.cap(2)
                                << ps_output_line.cap(3)
                                << ps_output_line.cap(4)
                                << ps_output_line.cap(5));

    }
    // Need to set focus here too, as K3ListView will
    // 'steal' it otherwise.
    m_ui->search->searchLine()->setFocus();
}

/***************************************************************************/

void ProcessSelectionDialog::slotProcessExited()
{
    delete psProc;
    psProc = 0;
}

void ProcessSelectionDialog::focusIn(QFocusEvent*)
{
    m_ui->search->searchLine()->setFocus();
}

}

/***************************************************************************/
#include "processselection.moc"
