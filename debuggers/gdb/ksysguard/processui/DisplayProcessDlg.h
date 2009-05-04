/*
    KSysGuard, the KDE System Guard

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

#ifndef _DisplayProcessDlg_h_
#define _DisplayProcessDlg_h_

#include <kdialog.h>
#include <kprocess.h>
#include "processes.h"

class Ui_DisplayProcessUi;

class DisplayProcessDlg : public KDialog
{
	Q_OBJECT

public:
	DisplayProcessDlg(QWidget* parent, KSysGuard::Process *process);
	~DisplayProcessDlg();
	/** Returns the default size of the dialog.  Reimplmentation from QDialog */
	virtual QSize sizeHint() const;

public Q_SLOTS:
	/** Close button has been clicked.  Reimplementation from KDialog */
	virtual void slotButtonClicked(int);
	void slotBtnPause(bool pause);
	void slotBtnDetach(bool detach);

private:
	Ui_DisplayProcessUi *ui;
};

#endif

