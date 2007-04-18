/***************************************************************************
   copyright            : (C) 2007 by David Nolden
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
#ifndef INCLUDEPATHRESOLVER_H
#define INCLUDEPATHRESOLVER_H
#include <qstring.h> /* defines QString */
#include <qmap.h>

namespace CppTools {
  class FileModificationTimeWrapper;
  
  struct PathResolutionResult {
    PathResolutionResult( bool _success = false, const QString& _errorMessage = QString(), const QString& _longErrorMessage = QString() ) : success( _success ), errorMessage( _errorMessage ), longErrorMessage( _longErrorMessage )  {
    }
    bool success;
    QString errorMessage;
    QString longErrorMessage;
    
    QStringList path;

    operator bool() const {
      return success;
    };
  };

  class SourcePathInformation;

  ///One resolution-try can issue up to 4 make-calls in worst case
  class IncludePathResolver {
    public:
      ///Whether the Qt event-loop should be continued(using BlockingKProcess). This crashes if enabled in a non-foreground thread.
      IncludePathResolver( bool continueEventLoop = false );
      ///Same as below, but uses the directory of the file as working-directory. The argument must be absolute.
      PathResolutionResult resolveIncludePath( const QString& file );
      ///The include-path is only computed once for a whole directory, then it is cached using the modification-time of the Makefile.
      PathResolutionResult resolveIncludePath( const QString& file, const QString& workingDirectory );
      ///source and build must be absolute paths
      void setOutOfSourceBuildSystem( const QString& source, const QString& build );
    private:
      bool m_isResolving;
      bool m_continueEventLoop;
      struct CacheEntry {
        CacheEntry() : failed(false) {
        }
        QDateTime modificationTime;
        QStringList path;
        QString errorMessage, longErrorMessage;
        bool failed;
        QMap<QString,bool> failedFiles;
        QDateTime failTime;
      };
      typedef QMap<QString, CacheEntry> Cache;
      Cache m_cache;

      ///Executes the command, either using popen or BlockingKProcess
      PathResolutionResult getFullOutput( const QString& command, const QString& workingDirectory, QString& output ) const;
      bool executeCommandPopen ( const QString& command, const QString& workingDirectory, QString& result ) const;
      ///file should be the name of the target, without extension(because that may be different)
      PathResolutionResult resolveIncludePathInternal( const QString& file, const QString& workingDirectory, const QString& makeParameters, const SourcePathInformation& source );
      bool m_outOfSource;
      QString m_source;
      QString m_build;
  };
};

#endif
