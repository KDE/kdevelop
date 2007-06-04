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

#include "safelogger.h"

LogSuffix& LogSuffix::operator = ( const LogSuffix& /*rhs */ ) {
  return * this;
}
LogSuffix::LogSuffix( const LogSuffix& /*rhs */ ) {}

LogSuffix::LogSuffix( const char* txt, SafeLogger* logger ) : m_logger( logger ) {
  m_logger->m_logSuffix.push_back( txt );
}

LogSuffix::~LogSuffix() {
  if ( !m_logger->m_logSuffix.empty() )
    m_logger->m_logSuffix.pop_back();
  else
    m_logger->err() << "error in SafeLogger::LogSuffix: tried to pop more than pushed";
}

SafeLogger::SafeLogger( Teamwork::LoggerPointer logger, std::string logPrefix ) : m_logger( logger ), m_logPrefix( logPrefix ) {}

SafeLogger::~SafeLogger() {}

Teamwork::LoggerPrinter SafeLogger::out( Teamwork::Logger::Level level ) {
  Teamwork::LoggerPrinter ret( m_logger, level );
  ret << logPrefix();
  uint sz = m_logSuffix.size();
  for ( uint a = 0; a < sz; a++ )
    ret << m_logSuffix[ a ];
  return ret;
}

Teamwork::LoggerPointer SafeLogger::logger() const {
  return m_logger;
}

Teamwork::LoggerPrinter SafeLogger::err() {
  Teamwork::LoggerPrinter ret( m_logger, Teamwork::Logger::Error );
  ret << logPrefix();
  uint sz = m_logSuffix.size();
  for ( uint a = 0; a < sz; a++ )
    ret << m_logSuffix[ a ];
  return ret;
}

std::string SafeLogger::logPrefix() {
  return m_logPrefix;
}

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
