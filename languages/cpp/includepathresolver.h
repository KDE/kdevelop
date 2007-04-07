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
#include "qstring.h" /* defines QString */

namespace CppTools {
  struct PathResolutionResult {
    PathResolutionResult( bool _success, const QString& _errorMessage = QString(), const QString& _longErrorMessage = QString() ) : success( _success ), errorMessage( _errorMessage ), longErrorMessage( _longErrorMessage )  {
    }
    bool success;
    QString errorMessage;
    QString longErrorMessage;
    
    QStringList path;

    operator bool() const {
      return success;
    };
  };
  
  class IncludePathResolver {
    public:
      IncludePathResolver();
      PathResolutionResult resolveIncludePath( const QString& file, const QString& workingDirectory );
      ///source and build must be absolute paths
      void setOutOfSourceBuildSystem( const QString& source, const QString& build );
    private:
      PathResolutionResult resolveIncludePathInternal( const QString& absoluteFile, const QString& workingDirectory, const QString& makeParameters );
      bool m_outOfSource;
      QString m_source;
      QString m_build;
  };
};

#endif