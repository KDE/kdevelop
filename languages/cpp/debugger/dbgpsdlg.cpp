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

#include "dbgpsdlg.h"

#include <kbuttonbox.h>
#include <kdialog.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kprocess.h>
#include <kstdguiitem.h>
#include <kdeversion.h>
#include <klistview.h>
#include <klistviewsearchline.h>
#include <kmessagebox.h>

#include <qframe.h>
#include <qlabel.h>
#include <qlayout.h>

#include <qtoolbutton.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qheader.h>
#include <qtimer.h>

#include <unistd.h>
#include <sys/types.h>

namespace GDBDebugger
{

/***************************************************************************/

// Display a list of processes for the user to select one
// only display processes that they can do something with so if the user
// is root then display all processes
// For use with the internal debugger, but this dialog doesn't know anything
// about why it's doing it.

Dbg_PS_Dialog::Dbg_PS_Dialog(QWidget *parent, const char *name)
    : KDialog(parent, name, true),      // modal
      psProc_(0),
      pids_(new KListView(this)),
      pidLines_(QString())
{
    setCaption(i18n("Attach to Process"));

    pids_->addColumn("PID");
    pids_->addColumn("TTY");
    pids_->addColumn("STAT");
    pids_->addColumn("COMMAND");
    pids_->addColumn("TIME");


    QBoxLayout *topLayout = new QVBoxLayout(this, 5);

    searchLineWidget_ = new KListViewSearchLineWidget(pids_, this);
    topLayout->addWidget(searchLineWidget_);

    topLayout->addWidget(pids_);
    pids_->setFont(KGlobalSettings::fixedFont());

    KButtonBox *buttonbox = new KButtonBox(this, Qt::Horizontal);
    buttonbox->addStretch();
    QPushButton *ok       = buttonbox->addButton(KStdGuiItem::ok());
    QPushButton *cancel   = buttonbox->addButton(KStdGuiItem::cancel());
    buttonbox->layout();
    topLayout->addWidget(buttonbox);

    connect(ok,     SIGNAL(clicked()),  SLOT(accept()));
    connect(cancel, SIGNAL(clicked()),  SLOT(reject()));  

    // Default display to 40 chars wide, default height is okay
    resize( ((KGlobalSettings::fixedFont()).pointSize())*40, height());
    topLayout->activate();

    QTimer::singleShot(0, this, SLOT(slotInit()));

}

/***************************************************************************/

Dbg_PS_Dialog::~Dbg_PS_Dialog()
{
    delete psProc_;
}

/***************************************************************************/

int Dbg_PS_Dialog::pidSelected()
{
	return pids_->currentItem()->text(0).toInt();
}

/***************************************************************************/
void Dbg_PS_Dialog::slotInit()
{
    psProc_ = new KShellProcess("/bin/sh");
#ifdef USE_SOLARIS
    *psProc_ << "ps";
    *psProc_ << "-opid";
    *psProc_ << "-otty";
    *psProc_ << "-os";
    *psProc_ << "-otime";
    *psProc_ << "-oargs";
    pidCmd_ = "ps -opid -otty -os -otime -oargs";

    if (getuid() == 0) {
        *psProc_ << "-e";
        pidCmd_ += " -e";
    }
#else
    *psProc_ << "ps";
    *psProc_ << "x";
    pidCmd_ = "ps x";

    if (getuid() == 0) {
        *psProc_ << "a";
        pidCmd_ += " a";
    }
#endif

    connect( psProc_, SIGNAL(processExited(KProcess *)),                SLOT(slotProcessExited()) );
    connect( psProc_, SIGNAL(receivedStdout(KProcess *, char *, int)),  SLOT(slotReceivedOutput(KProcess *, char *, int)) );

    psProc_->start(KProcess::NotifyOnExit, KProcess::Stdout);
}

/***************************************************************************/

void Dbg_PS_Dialog::slotReceivedOutput(KProcess */*proc*/, char *buffer, int buflen)
{
    pidLines_ += QString::fromLocal8Bit(buffer, buflen);
}

/***************************************************************************/

void Dbg_PS_Dialog::slotProcessExited()
{
    delete psProc_;
    psProc_ = 0;

    pidLines_ += '\n';

    int start = pidLines_.find('\n', 0);  // Skip the first line (header line)
    int pos;

    static QRegExp ps_output_line("^\\s*(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+(.+)");
    while ( (pos = pidLines_.find('\n', start)) != -1) {        

        QString item = pidLines_.mid(start, pos-start);
        if (!item.isEmpty() && item.find(pidCmd_) == -1)
        {
            if(ps_output_line.search(item) == -1)
            {
                KMessageBox::error(
                    this, 
                    // FIXME: probably should XML-escape 'item' before passing it
                    // to 'arg'.
                    i18n("<b>Could not parse output from the <tt>ps</tt> command!</b>"
                         "<p>The following line could not be parsed:"
                         "<b><tt>%1</tt>").arg(item),
                    i18n("Internal error"));
                break;
            }
            
            new QListViewItem(pids_, 
                              ps_output_line.cap(1),
                              ps_output_line.cap(2),
                              ps_output_line.cap(3),
                              ps_output_line.cap(4),
                              ps_output_line.cap(5));
        }

        start = pos+1;    
    }
    // Need to set focus here too, as KListView will
    // 'steal' it otherwise.
    searchLineWidget_->searchLine()->setFocus();
}

void Dbg_PS_Dialog::focusIn(QFocusEvent*)
{
    searchLineWidget_->searchLine()->setFocus();
}

}

/***************************************************************************/
#include "dbgpsdlg.moc"
