/***************************************************************************
                          stty.h  -  description                              
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

#ifndef STTY_H
#define STTY_H

class QSocketNotifier;

#include <qobject.h>
#include <qstring.h>

class STTY : public QObject
{ Q_OBJECT
public: 
  STTY();
  ~STTY();

  QString getMainTTY()    { return maintty; };

private slots:
  void OutReceived(int);

signals:
  void OutOutput( const char* );
  void ErrOutput( const char* );

private:
  int findTTY();

private:
  int fout;
  int ferr;
  QSocketNotifier* out;
  QSocketNotifier* err;
  QString maintty;

  char ptynam[50]; // "/dev/ptyxx" | "/dev/ptmx"
  char ttynam[50]; // "/dev/ttyxx" | "/dev/pts/########..."
};

#endif
