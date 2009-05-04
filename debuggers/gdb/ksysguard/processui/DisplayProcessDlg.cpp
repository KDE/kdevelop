/*
    KSysGuard, the KDE System Guard

        Copyright (C) 2007 Trent Waddington <trent.waddington@gmail.com>
	Copyright (c) 2008 John Tapsell <tapsell@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.


*/

#include "KMonitorProcessIO.h"
#include "DisplayProcessDlg.h"
#include "DisplayProcessDlg.moc"
#include "ui_DisplayProcessUi.h"

DisplayProcessDlg::DisplayProcessDlg(QWidget* parent, KSysGuard::Process *process)
	: KDialog( parent )
{
	setObjectName( "Display Process Dialog" );
	setModal( false );
	setCaption( i18n("Monitoring I/O for %1 (%2)", process->pid, process->name) );
	setButtons( Close );
	//enableLinkedHelp( true );
	showButtonSeparator( true );

	QWidget *widget = new QWidget(this);
	setMainWidget(widget);
	ui = new Ui_DisplayProcessUi();
	ui->setupUi(widget);

	ui->mTextEdit->setWhatsThis(i18n("The program '%1' (PID: %2) is being monitored for input and output through any file descriptor (stdin, stdout, stderr, open files, network connections, etc.).  Data being written by the process is shown in red and data being read by the process is shown in blue.", process->name, process->pid));
	if(!ui->mTextEdit->attach(process->pid)) {
		ui->btnDetach->setText(i18n("&Attach"));
		ui->btnDetach->setChecked(true);
		ui->btnPause->setText(i18n("&Pause"));
		ui->btnPause->setChecked(false);
		ui->btnPause->setEnabled(false);

	}
	connect(ui->btnPause, SIGNAL(toggled(bool)), this, SLOT(slotBtnPause(bool)));
	connect(ui->btnDetach, SIGNAL(toggled(bool)), this, SLOT(slotBtnDetach(bool)));
}

DisplayProcessDlg::~DisplayProcessDlg() {
	ui->mTextEdit->detach();
}
void DisplayProcessDlg::slotButtonClicked(int)
{
	ui->mTextEdit->detach();
	accept();
}

QSize DisplayProcessDlg::sizeHint() const {
	return QSize(600,600);
}

void DisplayProcessDlg::slotBtnPause(bool pause) {
	if(pause) {
		ui->mTextEdit->pauseProcesses();
		ui->btnPause->setText(i18n("&Resume"));
	} else {
		ui->mTextEdit->resumeProcesses();
		ui->btnPause->setText(i18n("&Pause"));
	}
}
void DisplayProcessDlg::slotBtnDetach(bool detach) {
	if(detach) {
		ui->btnDetach->setText(i18n("&Attach"));
		ui->btnDetach->setChecked(true);
		ui->btnPause->setText(i18n("&Pause"));
		ui->btnPause->setChecked(false);
		ui->btnPause->setEnabled(false);
		ui->mTextEdit->detach();
	} else {
		if(!ui->mTextEdit->reattach()) {
			//failed to attached
			ui->btnDetach->setText(i18n("&Attach"));
			ui->btnDetach->setChecked(true);
			ui->btnPause->setText(i18n("&Pause"));
			ui->btnPause->setChecked(false);
			ui->btnPause->setEnabled(false);
		} else
			ui->btnDetach->setText(i18n("&Detach"));
	}
}

