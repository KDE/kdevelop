/***************************************************************************
    begin                : Mon Sep 20 1999
    copyright            : (C) 1999 by John Birch
    email                : jbb@kdevelop.org
	
                          Adapted for ruby debugging
                          --------------------------
    begin                : Mon Nov 1 2004
    copyright            : (C) 2004 by Richard Dale
    email                : Richard_Dale@tipitina.demon.co.uk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DBGPSDLG_H_
#define _DBGPSDLG_H_

#include <kdialog.h>

class QListBox;
class KProcess;
class QLabel;

namespace RDBDebugger
{

/***************************************************************************/

class Dbg_PS_Dialog : public KDialog
{
    Q_OBJECT

public:
    Dbg_PS_Dialog( QWidget *parent=0, const char *name=0 );
    ~Dbg_PS_Dialog();

    int pidSelected();

private slots:
    void slotReceivedOutput(KProcess *proc, char *buffer, int buflen);
    void slotProcessExited();

private:
    KProcess* psProc_;
    QListBox* pids_;
    QLabel*   heading_;
    QString   pidLines_;
    QString   pidCmd_;
};

}

#endif
