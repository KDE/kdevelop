/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "processwidget.h"
#include "processlinemaker.h"

#include <qdir.h>
#include <kdebug.h>
#include <klocale.h>
#include <kprocess.h>
#include <qpainter.h>


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


ProcessWidget::ProcessWidget(QWidget *parent, const char *name)
    : QListBox(parent, name)
{
    setFocusPolicy(QWidget::NoFocus);
    QPalette pal = palette();
    pal.setColor(QColorGroup::HighlightedText,
                 pal.color(QPalette::Normal, QColorGroup::Text));
    pal.setColor(QColorGroup::Highlight,
                 pal.color(QPalette::Normal, QColorGroup::Mid));
    setPalette(pal);

    childproc = new KProcess();
    childproc->setUseShell(true);
    
    procLineMaker = new ProcessLineMaker( childproc );

    connect( procLineMaker, SIGNAL(receivedStdoutLine(const QString&)),
             this, SLOT(insertStdoutLine(const QString&) ));
    connect( procLineMaker, SIGNAL(receivedStderrLine(const QString&)),
             this, SLOT(insertStderrLine(const QString&) ));

    connect(childproc, SIGNAL(processExited(KProcess*)),
            this, SLOT(slotProcessExited(KProcess*) )) ;
}


ProcessWidget::~ProcessWidget()
{
    delete childproc;
    delete procLineMaker;
}


void ProcessWidget::startJob(const QString &dir, const QString &command)
{
    clear();
    insertItem(new ProcessListBoxItem(command, ProcessListBoxItem::Diagnostic));
    childproc->clearArguments();
    if (!dir.isNull()) {
        kdDebug(9000) << "Changing to dir " << dir << endl;
        QDir::setCurrent(dir);
    }

    *childproc << command;
    childproc->start(KProcess::NotifyOnExit, KProcess::AllOutput);
}


void ProcessWidget::killJob()
{
    childproc->kill();
}


bool ProcessWidget::isRunning()
{
    return childproc->isRunning();
}


void ProcessWidget::slotProcessExited(KProcess *)
{
    emit processExited(childproc);
    childFinished(childproc->normalExit(), childproc->exitStatus());
}


void ProcessWidget::insertStdoutLine(const QString &line)
{
    insertItem(new ProcessListBoxItem(line, ProcessListBoxItem::Normal));
}


void ProcessWidget::insertStderrLine(const QString &line)
{
    insertItem(new ProcessListBoxItem(line, ProcessListBoxItem::Error));
}


void ProcessWidget::childFinished(bool normal, int status)
{
    QString s;
    ProcessListBoxItem::Type t;
    
    if (normal) {
        if (status) {
            s = i18n("*** Exited with status: %1 ***").arg(status);
            t = ProcessListBoxItem::Error;
        } else {
            s = i18n("*** Exited normally ***");
            t = ProcessListBoxItem::Diagnostic;
        }
    } else {
        s = i18n("*** Process aborted ***");
        t = ProcessListBoxItem::Error;
    }
    
    insertItem(new ProcessListBoxItem(s, t));
}


QSize ProcessWidget::minimumSizeHint() const
{
    // I'm not sure about this, but when I don't use override minimumSizeHint(),
    // the initial size in clearly too small
    
    return QSize( QListBox::sizeHint().width(),
                  (fontMetrics().lineSpacing()+2)*4 );
}

#include "processwidget.moc"
