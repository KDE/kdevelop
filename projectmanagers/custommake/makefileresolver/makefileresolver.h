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

class QDir;
class QFile;

class FileModificationTimeWrapper;

struct PathResolutionResult
{
  PathResolutionResult(bool success = false, const QString& errorMessage = QString(), const QString& longErrorMessage = QString());

  bool success;
  QString errorMessage;
  QString longErrorMessage;

  KDevelop::ModificationRevisionSet includePathDependency;

  QStringList paths;

  void addPathsUnique(const PathResolutionResult& rhs);

  operator bool() const;
};

class SourcePathInformation;

///One resolution-try can issue up to 4 make-calls in worst case
class MakeFileResolver
{
  public:
    MakeFileResolver();
    ///Same as below, but uses the directory of the file as working-directory. The argument must be absolute.
    PathResolutionResult resolveIncludePath( const QString& file );
    ///The include-path is only computed once for a whole directory, then it is cached using the modification-time of the Makefile.
    ///source and build must be absolute paths
    void setOutOfSourceBuildSystem( const QString& source, const QString& build );
    ///resets to in-source build system
    void resetOutOfSourceBuild();

    static void clearCache();

    KDevelop::ModificationRevisionSet findIncludePathDependency(const QString& file);

    void enableMakeResolution(bool enable);
    PathResolutionResult processOutput(const QString& fullOutput, const QString& workingDirectory) const;

  private:
    PathResolutionResult resolveIncludePath( const QString& file, const QString& workingDirectory, int maxStepsUp = 20 );

    bool m_isResolving;
    bool m_outOfSource;

    QString mapToBuild(const QString &path) const;

    ///Executes the command using KProcess
    bool executeCommand( const QString& command, const QString& workingDirectory, QString& result ) const;
    ///file should be the name of the target, without extension(because that may be different)
    PathResolutionResult resolveIncludePathInternal( const QString& file, const QString& workingDirectory,
                                                      const QString& makeParameters, const SourcePathInformation& source, int maxDepth );
    PathResolutionResult processOutput(const QString& fullOutput, QRegExp& includeRx, const QString& workingDirectory) const;
    QString m_source;
    QString m_build;
};

#endif
