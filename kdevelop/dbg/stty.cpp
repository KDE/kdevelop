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

#include <fcntl.h>
#include <unistd.h>

#include <qstring.h>

STTY::STTY():QObject(),
  out(0),
  err(0)
{
  fout = findTTY();
  maintty = thisTTY;
  tty = QString(">") + thisTTY;

  ferr = findTTY();
  tty += QString(" 2>") + thisTTY;

  if ( fout && ferr ) {
    out = new QSocketNotifier(fout, QSocketNotifier::Read);
    connect( out, SIGNAL(activated(int)), this, SLOT(OutReceived(int)) );

    err = new QSocketNotifier(ferr, QSocketNotifier::Read);
    connect( err, SIGNAL(activated(int)), this, SLOT(OutReceived(int)) );
  } else {
    tty = "";
  }
}

STTY::~STTY()
{
  if ( out ) { ::close( fout ); delete out; }
  if ( err ) { ::close( ferr ); delete err; }
}

int STTY::findTTY()
{
  int ptyfd; char *s3, *s4;

  static char ptynam[] = "/dev/ptyxx";
  static char ttynam[] = "/dev/ttyxx";

  static char ptyc3[] = "pqrstuvwxyzabcde";
  static char ptyc4[] = "0123456789abcdef";

  // Find a master pty that we can open

  ptyfd = -1;
  for (s3 = ptyc3; *s3 != 0; s3++)
  {
    for (s4 = ptyc4; *s4 != 0; s4++)
    {
      ptynam[8] = ttynam[8] = *s3;
      ptynam[9] = ttynam[9] = *s4;
      if ((ptyfd = ::open(ptynam,O_RDWR)) >= 0)
      {
        if (::geteuid() == 0 || ::access(ttynam,R_OK|W_OK) == 0) break;
        ::close(ptyfd); ptyfd = -1;
      }
    }
    if (ptyfd >= 0) break;
  }
  if (ptyfd < 0) { return 0; }
  ::fcntl(ptyfd,F_SETFL,O_NDELAY);

  thisTTY = ttynam;
  return ptyfd;
}

void STTY::OutReceived(int f)
{
  char buf[1024];
  int n = ::read( f, buf, 1024 );
  if ( n != -1 ) {
    QString d(buf, n+1);
    if ( f == fout )
      emit OutOutput( d.data() );
    else
      emit ErrOutput( d.data() );
  }
}
