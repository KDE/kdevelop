/***************************************************************************
                          stty.cpp  -  description
                             -------------------
    begin                : Mon Sep 13 1999
    copyright            : (C) 1999 by Judin Maxim
    email                : jb.nz@writeme.com

    This code has been taken, as is, from the KDEStudio project done by
    Judin Maxim <novaprint@mtu-net.ru>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "stty.h"

#include <qsocketnotifier.h>
#include <qstring.h>

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

STTY::STTY() :
  QObject(),
  out(0),
  err(0)
{
  fout = findTTY();
  ttySlave = QString(tty_slave);
  ferr = findTTY();

  if (fout >= 0 && ferr >= 0)
  {
    out = new QSocketNotifier(fout, QSocketNotifier::Read);
    connect( out, SIGNAL(activated(int)), this, SLOT(OutReceived(int)) );
    err = new QSocketNotifier(ferr, QSocketNotifier::Read);
    connect( err, SIGNAL(activated(int)), this, SLOT(OutReceived(int)) );
  }
  else
    ttySlave = "";
}

STTY::~STTY()
{
  if ( out )
  {
    ::close( fout );
    delete out;
  }

  if ( err )
  {
    ::close( ferr );
    delete err;
  }
}

int STTY::findTTY()
{
  int ptyfd = -1;
//  needGrantPty = TRUE;

  // Find a master pty that we can open ////////////////////////////////

#ifdef __sgi__
  ptyfd = open("/dev/ptmx",O_RDWR);
  if (ptyfd < 0)
    {
      perror("Can't open a pseudo teletype");
      return(-1);
    }
  strncpy(tty_slave, ptsname(ptyfd), 50);
  grantpt(ptyfd);
  unlockpt(ptyfd);
//  needGrantPty = FALSE;
#endif

  // first we try UNIX PTY's
#ifdef TIOCGPTN
  strcpy(pty_master,"/dev/ptmx");
  strcpy(tty_slave,"/dev/pts/");
  ptyfd = open(pty_master,O_RDWR);
  if (ptyfd >= 0) // got the master pty
  {
    int ptyno;
    if (ioctl(ptyfd, TIOCGPTN, &ptyno) == 0)
    { struct stat sbuf;
      sprintf(tty_slave,"/dev/pts/%d",ptyno);
      if (stat(tty_slave,&sbuf) == 0 && S_ISCHR(sbuf.st_mode))
      {
//        needGrantPty = FALSE;
      }
      else
      {
        close(ptyfd);
        ptyfd = -1;
      }
    }
    else
    {
      close(ptyfd);
      ptyfd = -1;
    }
  }
#endif

#if defined(_SCO_DS) || defined(__USLC__) /* SCO OSr5 and UnixWare */
  if (ptyfd < 0)
  { for (int idx = 0; idx < 256; idx++)
    { sprintf(pty_master, "/dev/ptyp%d", idx);
      sprintf(tty_slave, "/dev/ttyp%d", idx);
      if (access(tty_slave, F_OK) < 0) { idx = 256; break; }
      if ((ptyfd = open (pty_master, O_RDWR)) >= 0)
      { if (access (tty_slave, R_OK|W_OK) == 0) break;
        close(ptyfd); ptyfd = -1;
      }
    }
  }
#endif
  if (ptyfd < 0) // Linux, FIXME: Trouble on other systems?
  {
    for (const char* s3 = "pqrstuvwxyzabcde"; *s3 != 0; s3++)
    {
      for (const char* s4 = "0123456789abcdef"; *s4 != 0; s4++)
      {
        sprintf(pty_master,"/dev/pty%c%c",*s3,*s4);
        sprintf(tty_slave,"/dev/tty%c%c",*s3,*s4);
        if ((ptyfd = open(pty_master, O_RDWR)) >= 0)
        {
          if (geteuid() == 0 || access(tty_slave,R_OK|W_OK) == 0)
            break;

          close(ptyfd);
          ptyfd = -1;
        }
      }

      if (ptyfd >= 0)
        break;
    }
  }

  if (ptyfd >= 0)
  {
//  if (needGrantPty && !chownpty(ptyfd,TRUE))
//  {
//    fprintf(stderr,"konsole: chownpty failed for device %s::%s.\n",pty_master,tty_slave);
//    fprintf(stderr,"       : This means the session can be eavesdroped.\n");
//    fprintf(stderr,"       : Make sure konsole_grantpty is installed in\n");
//    fprintf(stderr,"       : %s and setuid root.\n",
//	    KGlobal::dirs()->findResourceDir("exe", "konsole").data());
//  }

    ::fcntl(ptyfd, F_SETFL, O_NDELAY);
  }

  return ptyfd;
}

void STTY::OutReceived(int f)
{
  char buf[1024];
  int n;

  // read until socket is empty. We shouldn't be receiving a continuous
  // stream of data, so the loop is unlikely to cause problems.
  while ((n = ::read(f, buf, sizeof(buf)-1)) > 0)
  {
    *(buf+n) = 0;         // a standard string
    if ( f == fout )
      emit OutOutput(buf);
    else
      emit ErrOutput(buf);
  }
}
