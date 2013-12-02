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
#ifndef INCLUDEPATHRESOLVER_H
#define INCLUDEPATHRESOLVER_H

#include <QString>
#include <QStringList>
#include <language/editor/modificationrevisionset.h>

class KUrl;
class QDir;
class QFile;

namespace CppTools {
  class FileModificationTimeWrapper;

  struct PathResolutionResult {
    PathResolutionResult( bool _success = false, const QString& _errorMessage = QString(), const QString& _longErrorMessage = QString() ) : success( _success ), errorMessage( _errorMessage ), longErrorMessage( _longErrorMessage )  {
    }
    bool success;
    QString errorMessage;
    QString longErrorMessage;
    
    KDevelop::ModificationRevisionSet includePathDependency;

    QStringList paths;

    void addPathsUnique(const PathResolutionResult& rhs) {
      foreach(const QString& path, rhs.paths) {
        if(!paths.contains(path))
          paths.append(path);
      }
      includePathDependency += rhs.includePathDependency;
    }

    operator bool() const {
      return success;
    }
  };
  
  struct CustomIncludePathsSettings {
    QString storagePath; //Directory the custom file is in
    QString sourceDir;
    QString buildDir;
    QStringList paths;

    QString storageFile() const;
    bool isValid() const ;
    
    static CustomIncludePathsSettings read(const QString& storagePath);
    ///Finds a valid storage file above the given start path that contains custom include paht settings
    ///If no valid storage file is found, returns an empty string
    static QString find(const QString& startPath);
    ///Finds a storage-path for the given start path, and reads the custom include path settings
    ///If none were found, returns an invalid item
    static CustomIncludePathsSettings findAndRead(const QString& startPath);
    ///Same as findAndRead, but also convert all relative paths into absolute ones from the
    ///storage path.
    static CustomIncludePathsSettings findAndReadAbsolute(const QString& startPath);
    
    //Stores these settings exclusively, overwriting any old ones for the storage path
    bool write();
    //Deletes these settings
    bool delete_();
  };

  class SourcePathInformation;

  ///One resolution-try can issue up to 4 make-calls in worst case
  class IncludePathResolver {
    public:
      IncludePathResolver();
      ///Same as below, but uses the directory of the file as working-directory. The argument must be absolute.
      PathResolutionResult resolveIncludePath( const QString& file );
      ///The include-path is only computed once for a whole directory, then it is cached using the modification-time of the Makefile.
      PathResolutionResult resolveIncludePath( const QString& file, const QString& workingDirectory, int maxStepsUp = 20 );
      ///source and build must be absolute paths
      void setOutOfSourceBuildSystem( const QString& source, const QString& build );
      ///resets to in-source build system
      void resetOutOfSourceBuild();
      
      static void clearCache();
      
      KDevelop::ModificationRevisionSet findIncludePathDependency(const QString& file);
      
    private:
      bool m_isResolving;

      KUrl mapToBuild(const KUrl& url);

      /**Returns an invalid PathResolutionResult if it failed.
        *Returns a valid PathResolutionResult with empty directory list if.
        *Returns a PathResolutionResult with a filled path-list if the path-list could be read from somewhere(@todo Not implemented yet)
        * */
      PathResolutionResult validateDirectory(QDir& directory, QFile& file);

      ///Executes the command using KProcess
      bool executeCommand( const QString& command, const QString& workingDirectory, QString& result ) const;
      ///file should be the name of the target, without extension(because that may be different)
      PathResolutionResult resolveIncludePathInternal( const QString& file, const QString& workingDirectory, const QString& makeParameters, const SourcePathInformation& source, int maxDepth );
      bool m_outOfSource;
      QString m_source;
      QString m_build;
  };
}

#endif
