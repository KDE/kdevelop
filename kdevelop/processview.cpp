/***************************************************************************
                             processview.h
                             -------------------

    copyright            : (C) 1999 by The KDevelop Team
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
    : QListBox(parent, name)
{
    setFocusPolicy(QWidget::NoFocus);

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


void ProcessView::slotReceivedOutput(KProcess *, char *buffer, int buflen)
{
    buf += QString::fromLatin1(buffer, buflen);
    
    int pos;
    while ( (pos = buf.find('\n')) != -1)
        {
            QString item = buf.left(pos);
            if (!item.isEmpty())
                insert(item);
            buf = buf.right(buf.length()-pos-1);
        }
    // TODO: emit a signal which is connected with a slot in CKDevelop
    // which calls o_tab_view->setCurrentTab(MESSAGES);
}


void ProcessView::slotProcessExited(KProcess *)
{
    emit processExited(childproc);
}


void ProcessView::insert(const QString &line)
{
    insertItem(line);
}
