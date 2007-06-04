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
