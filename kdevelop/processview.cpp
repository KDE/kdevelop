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


ProcessListBoxItem::ProcessListBoxItem(const QString &s)
    : QListBoxText(s)
{}


bool ProcessListBoxItem::isErrorItem()
{
    return true;
}


void ProcessListBoxItem::paint(QPainter *p)
{
    p->setPen(Qt::darkRed);
    QListBoxText::paint(p);
}


ProcessView::ProcessView(QWidget *parent, const char *name)
    : QListBox(parent, name)
{
    setFocusPolicy(QWidget::NoFocus);

    childproc = new KShellProcess("/bin/sh");

    connect(childproc, SIGNAL(receivedStdout(KProcess*,char*,int)),
            this, SLOT(slotReceivedOutput(KProcess*,char*,int)) );
    
    connect(childproc, SIGNAL(receivedStderr(KProcess*,char*,int)),
            this, SLOT(slotReceivedError(KProcess*,char*,int)) );
    
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


bool ProcessView::isRunning()
{
    return childproc->isRunning();
}


void ProcessView::slotReceivedOutput(KProcess *, char *buffer, int buflen)
{
    buf += QString::fromLatin1(buffer, buflen);
    
    int pos;
    while ( (pos = buf.find('\n')) != -1)
        {
            QString item = buf.left(pos);
            if (!item.isEmpty())
                insertStdoutLine(item);
            buf = buf.right(buf.length()-pos-1);
        }
    // TODO: emit a signal which is connected with a slot in CKDevelop
    // which calls o_tab_view->setCurrentTab(MESSAGES);
}


void ProcessView::slotReceivedError(KProcess *, char *buffer, int buflen)
{
    QString item = QString::fromLatin1(buffer, buflen);
    insertStderrLine(item);
}


void ProcessView::slotProcessExited(KProcess *)
{
    emit processExited(childproc);
}


void ProcessView::insertStdoutLine(const QString &line)
{
    insertItem(line);
}


void ProcessView::insertStderrLine(const QString &line)
{
    insertItem(new ProcessListBoxItem(line));
}
