/*
    KSysGuard, the KDE System Guard

	Copyright (c) 1999 Chris Schlaeger <cs@kde.org>
	Copyright (c) 2007 John Tapsell <tapsell@kde.org>

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

#include <klocale.h>
#include <kdebug.h>

#include "ReniceDlg.moc"
#include <QListWidget>
#include <QButtonGroup>
#include "ui_ReniceDlgUi.h"
#include "processcore/process.h"

ReniceDlg::ReniceDlg(QWidget* parent, const QStringList& processes, int currentCpuPrio, int currentCpuSched, int currentIoPrio, int currentIoSched )
	: KDialog( parent )
{
	setObjectName( "Renice Dialog" );
	setModal( true );
	setCaption( i18n("Renice Process") );
	setButtons( Ok | Cancel );
	showButtonSeparator( true );
	previous_cpuscheduler = 0;

	connect( this, SIGNAL( okClicked() ), SLOT( slotOk() ) );

	if(currentIoSched == KSysGuard::Process::None) {
		// CurrentIoSched == 0 means that the priority is set automatically.
		// Using the formula given by the linux kernel Documentation/block/ioprio
		currentIoPrio = (currentCpuPrio+20)/5;
	}
	if(currentIoSched == (int)KSysGuard::Process::BestEffort && currentIoPrio == (currentCpuPrio+20)/5) {
		// Unfortunately, in linux you can't ever set a process back to being None.  So we fake it :)
		currentIoSched = KSysGuard::Process::None;
	}
	ioniceSupported = (currentIoPrio != -2);


	QWidget *widget = new QWidget(this);
	setMainWidget(widget);
	ui = new Ui_ReniceDlgUi();
	ui->setupUi(widget);
	ui->listWidget->insertItems(0, processes);

	cpuScheduler = new QButtonGroup(this);
	cpuScheduler->addButton(ui->radioNormal, (int)KSysGuard::Process::Other);
	cpuScheduler->addButton(ui->radioBatch, (int)KSysGuard::Process::Batch);
	cpuScheduler->addButton(ui->radioFIFO, (int)KSysGuard::Process::Fifo);
	cpuScheduler->addButton(ui->radioRR, (int)KSysGuard::Process::RoundRobin);
	if(currentCpuSched >= 0) { //negative means none of these
		QAbstractButton *sched = cpuScheduler->button(currentCpuSched);
		if(sched) {
			sched->setChecked(true); //Check the current scheduler
			previous_cpuscheduler = currentCpuSched;
		}
	}
	cpuScheduler->setExclusive(true);

	ioScheduler = new QButtonGroup(this);
	ioScheduler->addButton(ui->radioIONormal, (int)KSysGuard::Process::None);
	ioScheduler->addButton(ui->radioIdle, (int)KSysGuard::Process::Idle);
	ioScheduler->addButton(ui->radioRealTime, (int)KSysGuard::Process::RealTime);
	ioScheduler->addButton(ui->radioBestEffort, (int)KSysGuard::Process::BestEffort);
	if(currentIoSched >= 0) { //negative means none of these
		QAbstractButton *iosched = ioScheduler->button(currentIoSched);
		if(iosched)
		       	iosched->setChecked(true); //Check the current io scheduler
	}

	ioScheduler->setExclusive(true);

	if(ioniceSupported)
		ui->sliderIO->setValue(currentIoPrio);
	ui->sliderCPU->setValue(currentCpuPrio);

	ui->imgCPU->setPixmap( KIcon("cpu").pixmap(128, 128) );
	ui->imgIO->setPixmap( KIcon("drive-harddisk").pixmap(128, 128) );

	newCPUPriority = 40;

	connect(cpuScheduler, SIGNAL(buttonClicked(int)), this, SLOT(cpuSchedulerChanged(int)));
	connect(ioScheduler, SIGNAL(buttonClicked(int)), this, SLOT(updateUi()));
	connect(ui->sliderCPU, SIGNAL(valueChanged(int)), this, SLOT(cpuSliderChanged(int)));
	connect(ui->sliderIO, SIGNAL(valueChanged(int)), this, SLOT(ioSliderChanged(int)));
	
	updateUi();
}

void ReniceDlg::ioSliderChanged(int value) {
	ui->sliderIO->setToolTip(QString::number(value));
}

void ReniceDlg::cpuSchedulerChanged(int value) {
	if(value != previous_cpuscheduler) {
		if( (value == (int)KSysGuard::Process::Other || value == KSysGuard::Process::Batch) &&
		    (previous_cpuscheduler == (int)KSysGuard::Process::Fifo || previous_cpuscheduler == (int)KSysGuard::Process::RoundRobin)) {
			int slider = -ui->sliderCPU->value() * 2 / 5 + 20;
			setSliderRange();
			ui->sliderCPU->setValue( slider );
		} else if( (previous_cpuscheduler == (int)KSysGuard::Process::Other || previous_cpuscheduler == KSysGuard::Process::Batch) &&
		    (value == (int)KSysGuard::Process::Fifo || value == (int)KSysGuard::Process::RoundRobin)) {
			int slider = (-ui->sliderCPU->value() + 20) * 5 / 2; 
			setSliderRange();
			ui->sliderCPU->setValue( slider );
		}
	}
	previous_cpuscheduler = value;
	updateUi();
}

void ReniceDlg::cpuSliderChanged(int value) {
	if(cpuScheduler->checkedId() == (int)KSysGuard::Process::Other || cpuScheduler->checkedId() == (int)KSysGuard::Process::Batch) {
		if( ioScheduler->checkedId() == -1 || ioScheduler->checkedId() == (int)KSysGuard::Process::None) {
			//ionice is 'Normal', thus automatically calculated based on cpunice
			ui->sliderIO->setValue((value+20)/5);
		}
	}
	ui->sliderCPU->setToolTip(QString::number(value));
}

void ReniceDlg::updateUi() {
	bool cpuPrioEnabled = ( cpuScheduler->checkedId() != -1);
	bool ioPrioEnabled = ( ioniceSupported && ioScheduler->checkedId() != -1 && ioScheduler->checkedId() != (int)KSysGuard::Process::Idle && ioScheduler->checkedId() != (int)KSysGuard::Process::None);

	ui->sliderCPU->setEnabled(cpuPrioEnabled);
	ui->lblCpuLow->setEnabled(cpuPrioEnabled);
	ui->lblCpuHigh->setEnabled(cpuPrioEnabled);

	ui->sliderIO->setEnabled(ioPrioEnabled);
	ui->lblIOLow->setEnabled(ioPrioEnabled);
	ui->lblIOHigh->setEnabled(ioPrioEnabled);

	setSliderRange();
	cpuSliderChanged(ui->sliderCPU->value());
	ioSliderChanged(ui->sliderIO->value());
}

void ReniceDlg::setSliderRange() {
	if(cpuScheduler->checkedId() == (int)KSysGuard::Process::Other || cpuScheduler->checkedId() == (int)KSysGuard::Process::Batch) {
		//The slider is setting the priority, so goes from 19 to -20.  We cannot actually do this with a slider, so instead we go from -19 to 20, and negate later
		if(ui->sliderCPU->value() > 20) ui->sliderCPU->setValue(20);
		ui->sliderCPU->setInvertedAppearance(true);
		ui->sliderCPU->setMinimum(-19);
		ui->sliderCPU->setMaximum(20);
		ui->sliderCPU->setTickInterval(5);
	} else {
		if(ui->sliderCPU->value() < 1) ui->sliderCPU->setValue(1);
		ui->sliderCPU->setInvertedAppearance(false);
		ui->sliderCPU->setMinimum(1);
		ui->sliderCPU->setMaximum(99);
		ui->sliderCPU->setTickInterval(12);
	} 
}

void ReniceDlg::slotOk()
{
	newCPUPriority = ui->sliderCPU->value();
	newIOPriority = ui->sliderIO->value();
	newCPUSched = cpuScheduler->checkedId();
	newIOSched = ioScheduler->checkedId();
}
