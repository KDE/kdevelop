/***************************************************************************
                      coutputwidget.cpp - the output window in KDevelop
                             -------------------                                         

    begin                : 5 Aug 1998                                        
    copyright            : (C) 1998 by Sandy Meier                         
    email                : smeier@rz.uni-potsdam.de                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#include <qdir.h>
#include <kprocess.h>
#include "processview.h"


ProcessView::ProcessView(QWidget *parent, const char *name)
    : QMultiLineEdit(parent, name)
{
    setFocusPolicy(QWidget::NoFocus);
    setReadOnly(TRUE);

    childproc = new KShellProcess("/bin/sh");

    connect(childproc, SIGNAL(receivedStdout(KProcess*,char*,int)),
            this, SLOT(slotReceivedOutput(KProcess*,char*,int)) );
    
    connect(childproc, SIGNAL(receivedStderr(KProcess*,char*,int)),
            this, SLOT(slotReceivedOutput(KProcess*,char*,int)) );
    
    connect(childproc, SIGNAL(processExited(KProcess*)),
            this, SLOT(slotProcessExited(KProcess*) )) ;
}


ProcessView::~ProcessView()
{
    delete childproc;
}


void ProcessView::prepareJob(const QString &dir)
{
    childproc->clearArguments();
    QDir::setCurrent(dir);
}


void ProcessView::startJob()
{
    childproc->start(KProcess::NotifyOnExit, KProcess::AllOutput);
}


void ProcessView::killJob()
{
    childproc->kill();
}


KProcess &ProcessView::operator<<(const QString& arg)
{
    return (*childproc) << arg;
}


void ProcessView::projectClosed()
{
    clear();
}


void ProcessView::slotReceivedOutput(KProcess *, char *buffer, int buflen)
{
    insert(QString::fromLatin1(buffer, buflen));
    // TODO: emit a signal which is connected with a slot in CKDevelop
    // which calls o_tab_view->setCurrentTab(MESSAGES);
}


void ProcessView::slotProcessExited(KProcess *)
{
    emit processExited(childproc);
}


void ProcessView::mouseReleaseEvent(QMouseEvent *e)
{
    int row, col;
    QMultiLineEdit::mouseReleaseEvent(e);
    getCursorPosition(&row, &col);
    emit rowSelected(row);
}
