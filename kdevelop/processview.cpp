/***************************************************************************
                             processview.cpp - base class for output views
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


ProcessListBoxItem::ProcessListBoxItem(const QString &s, Type type)
    : QListBoxText(s), t(type)
{}


bool ProcessListBoxItem::isCustomItem()
{
    return false;
}


void ProcessListBoxItem::paint(QPainter *p)
{
    p->setPen((t==Error)? Qt::darkRed :
              (t==Diagnostic)? Qt::black : Qt::darkBlue);
    QListBoxText::paint(p);
}


ProcessView::ProcessView(QWidget *parent, const char *name)
    : QListBox(parent, name)
{
    setFocusPolicy(QWidget::NoFocus);
    QPalette pal = palette();
    pal.setColor(QColorGroup::HighlightedText,
                 pal.color(QPalette::Normal, QColorGroup::Text));
    pal.setColor(QColorGroup::Highlight,
                 pal.color(QPalette::Normal, QColorGroup::Mid));
    setPalette(pal);

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
    // TODO: emit a signal which is connected with a slot in CKDevelop
    // which calls o_tab_view->setCurrentTab(MESSAGES) or so...
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
    // Flush stderr buffer
    if (!stderrbuf.isEmpty())
        {
            insertStderrLine(stderrbuf);
            stderrbuf = "";
        }
    
    stdoutbuf += QString::fromLatin1(buffer, buflen);
    int pos;
    while ( (pos = stdoutbuf.find('\n')) != -1)
        {
            QString line = stdoutbuf.left(pos);
            //            if (!item.isEmpty())
            insertStdoutLine(line);
            stdoutbuf.remove(0, pos+1);
        }
}


void ProcessView::slotReceivedError(KProcess *, char *buffer, int buflen)
{
    // Flush stdout buffer
    if (!stdoutbuf.isEmpty())
        {
            insertStdoutLine(stdoutbuf);
            stdoutbuf = "";
        }
    
    stderrbuf += QString::fromLatin1(buffer, buflen);
    int pos;
    while ( (pos = stderrbuf.find('\n')) != -1)
        {
            QString line = stderrbuf.left(pos);
            //            if (!item.isEmpty())
            insertStderrLine(line);
            stderrbuf.remove(0, pos+1);
        }
    //    QString item = QString::fromLatin1(buffer, buflen);
    //    insertStderrLine(item);
}


void ProcessView::slotProcessExited(KProcess *)
{
    emit processExited(childproc);
    childFinished(childproc->normalExit(), childproc->exitStatus());
}


void ProcessView::insertStdoutLine(const QString &line)
{
    insertItem(new ProcessListBoxItem(line, ProcessListBoxItem::Normal));
}


void ProcessView::insertStderrLine(const QString &line)
{
    insertItem(new ProcessListBoxItem(line, ProcessListBoxItem::Error));
}


void ProcessView::childFinished(bool /*normal*/, int /*status*/)
{
}


void ProcessView::compilationAborted()
{
    killJob();
}
