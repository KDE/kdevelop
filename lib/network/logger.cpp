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

#include "logger.h"
#include <iostream>

namespace Teamwork {

std::string Logger::levelToString( Level lv ) {
  switch ( lv ) {
    case Debug:
    return "debug";
    case Info:
    return "info";
    case Warning:
    return "warning";
    case Error:
    return "error";
  }
  return "unknown level";
}

///This can be overridden for custom logging
void Logger::log( const std::string& str , Level lv ) {
  std::cout << "log-level: \"" << levelToString( lv ) << "\" message:  \"" << str << "\"" << endl;
}

///This function is used when an error occured while locking the Logger, or in other dangerous error-cases. It must be thread-safe.
void Logger::safeErrorLog( const std::string& str, Level lv ) {
  std::cout << "problem while logging, statically logging: \"" << "log-level: \"" << levelToString( lv ) << "\" message:  \"" << str << "\"" << endl;
}

void LoggerPrinter::invalidate() {
  out_ = 0;
  logger_ = 0;
};
LoggerPrinter& LoggerPrinter::operator = ( const LoggerPrinter& /*rhs*/ ) {
  return * this;
}
LoggerPrinter::LoggerPrinter( const LoggerPointer& logger, Logger::Level level ) : logger_( logger ), out_( 0 ), level_( level ) {
  /*if( logger_ )*/ out_ =    new std::ostringstream();
}

LoggerPrinter::LoggerPrinter( const LoggerPrinter& rhs ) : logger_( rhs.logger_ ), out_( rhs.out_ ), level_ ( rhs.level_ ) {
  ( const_cast<LoggerPrinter&>( rhs ) ).invalidate();
}

LoggerPrinter::~LoggerPrinter() {
  if ( !out_ )
    return ; ///the logger has been copied and thereby invalidated
  std::string s = out_->str();
  if ( s.length() ) {
    if ( LoggerPointer::Locked l = logger_ ) {
      l->log( s, level_ );
    } else {
      if ( logger_ ) {
        logger_.getUnsafeData() ->safeErrorLog( s, level_ );
      } else {
        cerr << "(bad logger): " << s << endl;
      }
    }
  }
  delete out_;
}

}

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
