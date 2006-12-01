/***************************************************************************
*   Copyright (C) 2006 by Andras Mantia                                   *
*   amantia@kde.org                                                       *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "blockingkprocess.h"

#include <qapplication.h>
#include <qtimer.h>

BlockingKProcess::BlockingKProcess(QObject *parent, const char *name)
  : KProcess(parent, name)
{
  m_stdOut = "";
  m_stdErr = "";
  m_timeoutValue = 60;
  m_timer = 0L;
  
  connect(this, SIGNAL(receivedStdout(KProcess *, char *, int)),
          this, SLOT(slotReceivedStdOut(KProcess *, char *, int)));
  connect(this, SIGNAL(receivedSterr(KProcess *, char *, int)),
          this, SLOT(slotReceivedStdErr(KProcess *, char *, int)));
  connect(this, SIGNAL(processExited(KProcess *)),
          this, SLOT(slotProcessExited(KProcess *)));  
}

BlockingKProcess::BlockingKProcess()
 : KProcess()
{
  m_stdOut = "";
  m_timeoutValue = 60;
  m_timer = 0L;
  connect(this, SIGNAL(receivedStdout(KProcess *, char *, int)),
          this, SLOT(slotReceivedStdOut(KProcess *, char *, int)));
  connect(this, SIGNAL(processExited(KProcess *)),
          this, SLOT(slotProcessExited(KProcess *)));  
}


BlockingKProcess::~BlockingKProcess()
{
}
bool BlockingKProcess::start(RunMode runmode, Communication comm)
{
  if (KProcess::start(runmode, comm))
  {
    m_timeout = false;
    m_timer = new QTimer();
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimeOut()));
    m_timer->start(m_timeoutValue*1000, true);
    enter_loop();
    delete m_timer;
    m_timer = 0L;
    return !m_timeout;
  } else
    return false;
}


void BlockingKProcess::slotReceivedStdOut(KProcess *, char *buffer, int buflen)
{
  m_stdOut += QString::fromLatin1(buffer, buflen);
}

void BlockingKProcess::slotReceivedStdErr(KProcess *, char *buffer, int buflen)
{
  m_stdErr += QString::fromLatin1(buffer, buflen);
}

void BlockingKProcess::slotProcessExited(KProcess *)
{
  qApp->exit_loop();
}

void BlockingKProcess::slotTimeOut()
{
  m_timeout = true;
  kill();
  qApp->exit_loop();
}


void qt_enter_modal( QWidget *widget );
void qt_leave_modal( QWidget *widget );

void BlockingKProcess::enter_loop()
{
  QWidget dummy(0,0,WType_Dialog | WShowModal);
  dummy.setFocusPolicy( QWidget::NoFocus );
  qt_enter_modal(&dummy);
  qApp->enter_loop();
  qt_leave_modal(&dummy);
}


#include "blockingkprocess.moc"
