/***************************************************************************
copyright            : (C) 2006 by David Nolden
email                : david.nolden.kdevelop@art-master.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef LOGGER_H
#define LOGGER_H

#include "networkfwd.h"
#include "safesharedptr.h"
#include <string>
#include <iostream>
#include <sstream>

namespace Teamwork {

///WARNING: The functions in this class will be called from many different threads, but only from one thread at a time(due to mutex-locking)
struct Logger : public SafeShared {
  enum Level {
    Debug = 1,
    Info = 2,
    Warning = 4,
    Error = 8
  };

  std::string levelToString( Level lv );

  ///This can be overridden for custom logging
  virtual void log( const std::string& str , Level lv );

  ///This function is used when an error occured while locking the Logger, or in other dangerous error-cases. It must be thread-safe.
  virtual void safeErrorLog( const std::string& str, Level lv );
};

///this class represents one line of logger-output. On its destruction the line is given to the logger. When it is copied, the copy-source is invalidated.
class LoggerPrinter {
  public:
    LoggerPrinter( const LoggerPointer& logger, Logger::Level level = Logger::Info );

    LoggerPrinter( const LoggerPrinter& rhs );

    template <class ItemType>
    LoggerPrinter& operator << ( const ItemType& item ) {
      if ( out_ )
        * out_ << item;
      else
        std::cout << item; //"invalidated logger is being used" << endl;*/

      return *this;
    }

    ~LoggerPrinter();
  private:
    LoggerPointer logger_;
    std::ostringstream* out_;
    Logger::Level level_;

    void invalidate();
    LoggerPrinter& operator = ( const LoggerPrinter& /*rhs*/ );
};
}

#endif

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
