/***************************************************************************
Copyright 2006 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef LOGLEVEL_H
#define LOGLEVEL_H

#include <QIcon>
enum LogLevel {
  Info = 1,
  Warning = 2,
  Error = 4,
  Debug = 8
};

QIcon iconFromLevel( LogLevel lv );

#endif
