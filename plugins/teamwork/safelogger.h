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

#ifndef SAFELOGGER_H
#define SAFELOGGER_H

#include <string>
#include <vector>

#include "network/safesharedptr.h"
#include "network/logger.h"

class LogSuffix;

/** This class can be used for very comfortable and safe logging: Just derive your own class from it initialize
 *  it with the logger-pointer, and give a log-prefix or override logPrefix(..) for flexible prefixing.
 *
 *  Then you can use 'out(...) << "your text"' or 'err() << "your text"' to log.
 * */
class SafeLogger {
  public:
    friend class LogSuffix;

    explicit SafeLogger( Teamwork::LoggerPointer logger, std::string logPrefix = "" );

    virtual ~SafeLogger();

    ///Returns a temporary stream-object(see documentation of Teamwork::LoggerPrinter) for the given log-level.
    Teamwork::LoggerPrinter out( Teamwork::Logger::Level level = Teamwork::Logger::Info );

    ///Returns a temporary stream-object(see documentation of Teamwork::LoggerPrinter) for error-messages.
    Teamwork::LoggerPrinter err();

    ///Returns the used logger
    Teamwork::LoggerPointer logger() const;

  protected:
    virtual std::string logPrefix();
  private:
    Teamwork::LoggerPointer m_logger;
    std::string m_logPrefix;
    std::vector<const char*> m_logSuffix;
};

/**This class can be used to temporarily extend the log-prefix(it can be created locally, and the prefix-suffix is removed as lifetime ends ends)
 * Warning: For performance-reasons the given text is not copied, so it must live at least as long as this object.
 * */
class LogSuffix {
  public:
    LogSuffix( const char* txt, SafeLogger* logger );
    ~LogSuffix();
  private:
    LogSuffix& operator = ( const LogSuffix& /*rhs */);
    LogSuffix( const LogSuffix& /*rhs */);
    SafeLogger* m_logger;
};

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on

#endif
