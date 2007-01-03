/* This file is part of the KDE project
   Copyright (C) 1999-2001 Bernd Gehrmann <bernd@kdevelop.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "processwidget.h"
#include "processlinemaker.h"

#include <kdeversion.h>
#include <qdir.h>
#include <kdebug.h>
#include <klocale.h>
#include <kprocess.h>
#include <qpainter.h>
#include <qapplication.h>


ProcessListBoxItem::ProcessListBoxItem(const QString &s, Type type)
    : QListBoxText(s), t(type)
{
    QString clean = s;
    clean.replace( QChar('\t'), QString("  ") );
    clean.replace( QChar('\n'), QString() );
    clean.replace( QChar('\r'), QString() );
    setText( clean );

    setCustomHighlighting(true);
}


bool ProcessListBoxItem::isCustomItem()
{
    return false;
}

static inline unsigned char normalize(int a)
{
    return (a < 0 ? 0 : a > 255 ? 255 : a);
}

static inline double blend1(double a, double b, double k)
{
    return a + (b - a) * k;
}

QColor ProcessListBoxItem::blend(const QColor &c1, const QColor &c2, double k) const
{
    if (k < 0.0) return c1;
    if (k > 1.0) return c2;

    int r = normalize((int)blend1((double)c1.red(),   (double)c2.red(),   k));
    int g = normalize((int)blend1((double)c1.green(), (double)c2.green(), k));
    int b = normalize((int)blend1((double)c1.blue(),  (double)c2.blue(),  k));

    return QColor(qRgb(r, g, b));
}

void ProcessListBoxItem::paint(QPainter *p)
{
    QColor dim, warn, err, back;
    if (listBox()) {
        const QColorGroup& group = listBox()->palette().active();
        if (isSelected()) {
            back = group.button();
            warn = group.buttonText();
        }
        else
        {
            back = group.base();
            warn = group.text();
        }
        err = group.linkVisited();
        dim = blend(warn, back);
    }
    else
    {
        warn = Qt::black;
        dim = Qt::darkBlue;
        err = Qt::darkRed;
        if (isSelected())
            back = Qt::lightGray;
        else
            back = Qt::white;
    }
    p->fillRect(p->window(), QBrush(back));
    p->setPen((t==Error)? err :
              (t==Diagnostic)? warn : dim);
    QListBoxText::paint(p);
}


ProcessWidget::ProcessWidget(QWidget *parent, const char *name)
    : KListBox(parent, name)
{
    setFocusPolicy(QWidget::NoFocus);

    // Don't override the palette, as that can mess up styles. Instead, draw
    // the background ourselves (see ProcessListBoxItem::paint).


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
    procLineMaker->clearBuffers();
    procLineMaker->blockSignals( false );

    clear();
    insertItem(new ProcessListBoxItem(command, ProcessListBoxItem::Diagnostic));
    childproc->clearArguments();
    if (!dir.isNull()) {
        kdDebug(9000) << "Changing to dir " << dir << endl;
        QDir::setCurrent(dir);
    }

    *childproc << command;
    childproc->start(KProcess::OwnGroup, KProcess::AllOutput);
}


void ProcessWidget::killJob( int signo )
{
    procLineMaker->blockSignals( true );
    
	childproc->kill( signo );
}


bool ProcessWidget::isRunning()
{
    return childproc->isRunning();
}


void ProcessWidget::slotProcessExited(KProcess *)
{
    childFinished(childproc->normalExit(), childproc->exitStatus());
    maybeScrollToBottom();
    emit processExited(childproc);
}


void ProcessWidget::insertStdoutLine(const QString &line)
{
    insertItem(new ProcessListBoxItem(line,
                                      ProcessListBoxItem::Normal));
    maybeScrollToBottom();
}


void ProcessWidget::insertStderrLine(const QString &line)
{
    insertItem(new ProcessListBoxItem(line,
                                      ProcessListBoxItem::Error));
    maybeScrollToBottom();
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
      if ( childproc->signalled() && childproc->exitSignal() == SIGSEGV )
      {
        s = i18n("*** Process aborted. Segmentation fault ***");
      }
      else
      {
        s = i18n("*** Process aborted ***");
      }
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

/** Should be called right after an insertItem(),
   will automatic scroll the listbox if it is already at the bottom
   to prevent automatic scrolling when the user has scrolled up
*/
void ProcessWidget::maybeScrollToBottom()
{
    if ( verticalScrollBar()->value() == verticalScrollBar()->maxValue() ) 
    {
        setBottomItem( count() -1 );
    }
}

#include "processwidget.moc"
