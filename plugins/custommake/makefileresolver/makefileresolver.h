/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MAKEFILERESOLVER_H
#define MAKEFILERESOLVER_H

#include <QString>

#include <language/editor/modificationrevisionset.h>

#include <util/path.h>

struct PathResolutionResult
{
  explicit PathResolutionResult(bool success = false, const QString& errorMessage = QString(), const QString& longErrorMessage = QString());

  bool success;
  QString errorMessage;
  QString longErrorMessage;

  KDevelop::ModificationRevisionSet includePathDependency;

  KDevelop::Path::List paths;
  // the list of framework directories specified with explicit -iframework and/or -F arguments.
  // Mainly for OS X, but available everywhere to avoid #ifdefs and
  // because clang is an out-of-the-box cross-compiler.
  KDevelop::Path::List frameworkDirectories;
  QHash<QString, QString> defines;

  void mergeWith(const PathResolutionResult& rhs);

  operator bool() const;
};

class SourcePathInformation;

// reuse cached instances of strings, to share memory where possible
class StringInterner
{
public:
    QString internString(const QString& string);

private:
    QSet<QString> m_stringCache;
};

// reuse cached instances of paths, to share memory where possible
class PathInterner
{
public:
    PathInterner(const KDevelop::Path& base = {});

    KDevelop::Path internPath(const QString& path);

private:
    const KDevelop::Path m_base;
    QHash<QString, KDevelop::Path> m_pathCache;
};

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

    static QRegularExpression defineRegularExpression();
    static QHash<QString, QString> extractDefinesFromCompileFlags(const QString& compileFlags, StringInterner& stringInterner, QHash<QString, QString> defines);

  private:
    PathResolutionResult resolveIncludePath( const QString& file, const QString& workingDirectory, int maxStepsUp = 20 );

    bool m_isResolving = false;
    bool m_outOfSource = false;

    QString mapToBuild(const QString &path) const;

    ///Executes the command using KProcess
    bool executeCommand( const QString& command, const QString& workingDirectory, QString& result ) const;
    ///file should be the name of the target, without extension(because that may be different)
    PathResolutionResult resolveIncludePathInternal( const QString& file, const QString& workingDirectory,
                                                      const QString& makeParameters, const SourcePathInformation& source, int maxDepth );
    QString m_source;
    QString m_build;

    mutable StringInterner m_stringInterner;
    mutable PathInterner m_pathInterner;
};

#endif
