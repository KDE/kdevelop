/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "makefileresolver.h"

#include "helper.h"

#include <memory>
#include <cstdio>
#include <iostream>

#include <QDir>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QRegularExpression>
#include <QRegExp>

#include <KProcess>
#include <KLocalizedString>

#include <serialization/indexedstring.h>
#include <util/pushvalue.h>
#include <util/path.h>

// #define VERBOSE

#if defined(VERBOSE)
#define ifTest(x) x
#else
#define ifTest(x)
#endif

const int maximumInternalResolutionDepth = 3;

using namespace std;
using namespace KDevelop;

namespace {
  ///After how many seconds should we retry?
  static const int CACHE_FAIL_FOR_SECONDS = 200;

  static const int processTimeoutSeconds = 30;

  struct CacheEntry
  {
    CacheEntry()
    { }
    ModificationRevisionSet modificationTime;
    Path::List paths;
    Path::List frameworkDirectories;
    QHash<QString, QString> defines;
    QString errorMessage, longErrorMessage;
    bool failed = false;
    QMap<QString,bool> failedFiles;
    QDateTime failTime;
  };
  using Cache = QMap<QString, CacheEntry>;

  static Cache s_cache;
  static QMutex s_cacheMutex;
}

  /**
   * Compatibility:
   * make/automake: Should work perfectly
   * cmake: Thanks to the path-recursion, this works with cmake(tested with version "2.4-patch 6"
   *        with kdelibs out-of-source and with kdevelop4 in-source)
   **/
  class SourcePathInformation
  {
  public:
    explicit SourcePathInformation(const QString& path)
      : m_path(path)
    {
    }

    QString createCommand(const QString& absoluteFile, const QString& workingDirectory, const QString& makeParameters) const
    {
      QString relativeFile = Path(workingDirectory).relativePath(Path(absoluteFile));
#ifndef Q_OS_FREEBSD
      // GNU make implicitly enables "-w" for sub-makes, we don't want that
      QLatin1String noPrintDirFlag = QLatin1String(" --no-print-directory");
#else
      QLatin1String noPrintDirFlag;
#endif
      return QLatin1String("make -k") + noPrintDirFlag + QLatin1String(" -W \'") + absoluteFile + QLatin1String("\' -W \'") + relativeFile + QLatin1String("\' -n ") + makeParameters;
    }

    bool hasMakefile() const
    {
        QFileInfo makeFile(m_path, QStringLiteral("Makefile"));
        return makeFile.exists();
    }

    QStringList possibleTargets(const QString& targetBaseName) const
    {
      const QStringList ret{
      ///@todo open the make-file, and read the target-names from there.
      //It would be nice if all targets could be processed in one call, the problem is the exit-status of make, so for now make has to be called multiple times.
        targetBaseName + QLatin1String(".o"),
        targetBaseName + QLatin1String(".lo"),
      //ret << targetBaseName + ".lo " + targetBaseName + ".o";
        targetBaseName + QLatin1String(".ko"),
      };
      return ret;
    }

  private:
      QString m_path;
  };

static void mergePaths(KDevelop::Path::List& destList, const KDevelop::Path::List& srcList)
{
    for (const Path& path : srcList) {
        if (!destList.contains(path))
            destList.append(path);
    }
}

void PathResolutionResult::mergeWith(const PathResolutionResult& rhs)
{
    mergePaths(paths, rhs.paths);
    mergePaths(frameworkDirectories, rhs.frameworkDirectories);
    includePathDependency += rhs.includePathDependency;
    for (auto it = rhs.defines.begin(), end = rhs.defines.end(); it != end; ++it) {
        defines.insert(it.key(), it.value());
    }
}

PathResolutionResult::PathResolutionResult(bool success, const QString& errorMessage, const QString& longErrorMessage)
    : success(success)
    , errorMessage(errorMessage)
    , longErrorMessage(longErrorMessage)
{}

PathResolutionResult::operator bool() const
{
    return success;
}

ModificationRevisionSet MakeFileResolver::findIncludePathDependency(const QString& file)
{
  QString oldSourceDir = m_source;
  QString oldBuildDir = m_build;

  Path currentWd(mapToBuild(file));

  ModificationRevisionSet rev;
  while (currentWd.hasParent()) {
    currentWd = currentWd.parent();
    QString path = currentWd.toLocalFile();
    QFileInfo makeFile(QDir(path), QStringLiteral("Makefile"));
    if (makeFile.exists()) {
      IndexedString makeFileStr(makeFile.filePath());
      rev.addModificationRevision(makeFileStr, ModificationRevision::revisionForFile(makeFileStr));
      break;
    }
  }

  setOutOfSourceBuildSystem(oldSourceDir, oldBuildDir);

  return rev;
}

bool MakeFileResolver::executeCommand(const QString& command, const QString& workingDirectory, QString& result) const
{
  ifTest(cout << "executing " << command.toUtf8().constData() << endl);
  ifTest(cout << "in " << workingDirectory.toUtf8().constData() << endl);

  KProcess proc;
  proc.setWorkingDirectory(workingDirectory);
  proc.setOutputChannelMode(KProcess::MergedChannels);

  QStringList args(command.split(QLatin1Char(' ')));
  QString prog = args.takeFirst();
  proc.setProgram(prog, args);

  int status = proc.execute(processTimeoutSeconds * 1000);
  result = QString::fromUtf8(proc.readAll());

  return status == 0;
}

MakeFileResolver::MakeFileResolver()

{
}

///More efficient solution: Only do exactly one call for each directory. During that call, mark all source-files as changed, and make all targets for those files.
PathResolutionResult MakeFileResolver::resolveIncludePath(const QString& file)
{
  if (file.isEmpty()) {
    // for unit tests with temporary files
    return PathResolutionResult();
  }

  QFileInfo fi(file);
  return resolveIncludePath(fi.fileName(), fi.absolutePath());
}

QString MakeFileResolver::mapToBuild(const QString &path) const
{
  QString wd = QDir::cleanPath(path);
  if (m_outOfSource) {
    if (wd.startsWith(m_source) && !wd.startsWith(m_build)) {
        //Move the current working-directory out of source, into the build-system
        wd = QDir::cleanPath(m_build + QLatin1Char('/') + QStringView{wd}.sliced(m_source.length()));
      }
  }
  return wd;
}

void MakeFileResolver::clearCache()
{
  QMutexLocker l(&s_cacheMutex);
  s_cache.clear();
}

PathResolutionResult MakeFileResolver::resolveIncludePath(const QString& file, const QString& _workingDirectory, int maxStepsUp)
{
  //Prefer this result when returning a "fail". The include-paths of this result will always be added.
  PathResolutionResult resultOnFail;

  if (m_isResolving)
    return PathResolutionResult(false, i18n("Tried include path resolution while another resolution process was still running"));

  //Make the working-directory absolute
  QString workingDirectory = _workingDirectory;

  if (QFileInfo(workingDirectory).isRelative()) {
    QUrl u = QUrl::fromLocalFile(QDir::currentPath());

    if (workingDirectory == QLatin1String("."))
      workingDirectory = QString();
    else if (workingDirectory.startsWith(QLatin1String("./")))
      workingDirectory.remove(0, 2);

    if (!workingDirectory.isEmpty()) {
      u = u.adjusted(QUrl::StripTrailingSlash);
      u.setPath(u.path() + QLatin1Char('/') + workingDirectory);
    }
    workingDirectory = u.toLocalFile();
  } else
    workingDirectory = _workingDirectory;

  ifTest(cout << "working-directory: " <<  workingDirectory.toLocal8Bit().data() << "  file: " << file.toLocal8Bit().data() << std::endl;)

  QDir sourceDir(workingDirectory);
  QDir dir = QDir(mapToBuild(sourceDir.absolutePath()));

  QFileInfo makeFile(dir, QStringLiteral("Makefile"));
  if (!makeFile.exists()) {
    if (maxStepsUp > 0) {
      //If there is no makefile in this directory, go one up and re-try from there
      QFileInfo fileName(file);
      QString localName = sourceDir.dirName();

      if (sourceDir.cdUp() && !fileName.isAbsolute()) {
        const QString checkFor = localName + QLatin1Char('/') + file;
        PathResolutionResult oneUp = resolveIncludePath(checkFor, sourceDir.path(), maxStepsUp-1);
        if (oneUp.success) {
          oneUp.mergeWith(resultOnFail);
          return oneUp;
        }
      }
    }

    if (!resultOnFail.errorMessage.isEmpty() || !resultOnFail.paths.isEmpty() || !resultOnFail.frameworkDirectories.isEmpty())
      return resultOnFail;
    else
      return PathResolutionResult(false, i18n("Makefile is missing in folder \"%1\"", dir.absolutePath()), i18n("Problem while trying to resolve include paths for %1", file));
  }

  PushValue<bool> e(m_isResolving, true);

  Path::List cachedPaths; //If the call doesn't succeed, use the cached not up-to-date version
  Path::List cachedFWDirs;
  QHash<QString, QString> cachedDefines;
  ModificationRevisionSet dependency;
  dependency.addModificationRevision(IndexedString(makeFile.filePath()), ModificationRevision::revisionForFile(IndexedString(makeFile.filePath())));
  dependency += resultOnFail.includePathDependency;
  Cache::iterator it;
  {
    QMutexLocker l(&s_cacheMutex);
    it = s_cache.find(dir.path());
    if (it != s_cache.end()) {
      cachedPaths = it->paths;
      cachedFWDirs = it->frameworkDirectories;
      cachedDefines = it->defines;
      if (dependency == it->modificationTime) {
        if (!it->failed) {
          //We have a valid cached result
          PathResolutionResult ret(true);
          ret.paths = it->paths;
          ret.frameworkDirectories = it->frameworkDirectories;
          ret.defines = it->defines;
          ret.mergeWith(resultOnFail);
          return ret;
        } else {
          //We have a cached failed result. We should use that for some time but then try again. Return the failed result if: (there were too many tries within this folder OR this file was already tried) AND The last tries have not expired yet
          if (/*(it->failedFiles.size() > 3 || it->failedFiles.find(file) != it->failedFiles.end()) &&*/ it->failTime
                  .secsTo(QDateTime::currentDateTimeUtc())
              < CACHE_FAIL_FOR_SECONDS) {
              PathResolutionResult ret(false); // Fake that the result is ok
              ret.errorMessage = i18n("Cached: %1", it->errorMessage);
              ret.longErrorMessage = it->longErrorMessage;
              ret.paths = it->paths;
              ret.frameworkDirectories = it->frameworkDirectories;
              ret.defines = it->defines;
              ret.mergeWith(resultOnFail);
              return ret;
          } else {
              // Try getting a correct result again
          }
        }
      }
    }
  }

  ///STEP 1: Prepare paths
  QString targetName;
  QFileInfo fi(file);

  QString absoluteFile = file;
  if (fi.isRelative())
    absoluteFile = workingDirectory + QLatin1Char('/') + file;
  absoluteFile = QDir::cleanPath(absoluteFile);

  int dot;
  if ((dot = file.lastIndexOf(QLatin1Char('.'))) == -1) {
    if (!resultOnFail.errorMessage.isEmpty() || !resultOnFail.paths.isEmpty() || !resultOnFail.frameworkDirectories.isEmpty())
      return resultOnFail;
    else
      return PathResolutionResult(false, i18n("Filename %1 seems to be malformed", file));
  }

  targetName = file.left(dot);

  QString wd = dir.path();
  if (QFileInfo(wd).isRelative()) {
    wd = QDir::cleanPath(QDir::currentPath() + QLatin1Char('/') + wd);
  }

  wd = mapToBuild(wd);

  SourcePathInformation source(wd);
  QStringList possibleTargets = source.possibleTargets(targetName);

  ///STEP 3: Try resolving the paths, by using once the absolute and once the relative file-path. Which kind is required differs from setup to setup.

  ///STEP 3.1: Try resolution using the absolute path
  PathResolutionResult res;
  //Try for each possible target
  res = resolveIncludePathInternal(absoluteFile, wd, possibleTargets.join(QLatin1Char(' ')), source, maximumInternalResolutionDepth);
  if (!res) {
    ifTest(cout << "Try for absolute file " << absoluteFile.toLocal8Bit().data() << " and targets " << possibleTargets.join(", ").toLocal8Bit().data()
                 << " failed: " << res.longErrorMessage.toLocal8Bit().data() << endl;)
  }

  res.includePathDependency = dependency;

  if (res.paths.isEmpty()) {
      res.paths = cachedPaths; //We failed, maybe there is an old cached result, use that.
      res.defines = cachedDefines;
  }
  // a build command could contain only one or more -iframework or -F specifications.
  if (res.frameworkDirectories.isEmpty()) {
      res.frameworkDirectories = cachedFWDirs;
  }

  {
    QMutexLocker l(&s_cacheMutex);
    if (it == s_cache.end())
      it = s_cache.insert(dir.path(), CacheEntry());

    CacheEntry& ce(*it);
    ce.paths = res.paths;
    ce.frameworkDirectories = res.frameworkDirectories;
    ce.modificationTime = dependency;

    if (!res) {
      ce.failed = true;
      ce.errorMessage = res.errorMessage;
      ce.longErrorMessage = res.longErrorMessage;
      ce.failTime = QDateTime::currentDateTimeUtc();
      ce.failedFiles[file] = true;
    } else {
      ce.failed = false;
      ce.failedFiles.clear();
    }
  }


  if (!res && (!resultOnFail.errorMessage.isEmpty() || !resultOnFail.paths.isEmpty() || !resultOnFail.frameworkDirectories.isEmpty()))
    return resultOnFail;

  return res;
}

static QRegularExpression includeRegularExpression()
{
  static const QRegularExpression expression(QStringLiteral(
    "\\s(--include-dir=|-I\\s*|-isystem\\s+|-iframework\\s+|-F\\s*)("
    "\\'.*\\'|\\\".*\\\"" //Matches "hello", 'hello', 'hello"hallo"', etc.
    "|"
    "((?:\\\\.)?([\\S^\\\\]?))+" //Matches /usr/I\ am\ a\ strange\ path/include
    ")(?=\\s)"
  ));
  Q_ASSERT(expression.isValid());
  return expression;
}

PathResolutionResult MakeFileResolver::resolveIncludePathInternal(const QString& file, const QString& workingDirectory,
                                                                      const QString& makeParameters, const SourcePathInformation& source,
                                                                      int maxDepth)
{
  --maxDepth;
  if (maxDepth < 0)
    return PathResolutionResult(false);

  QString fullOutput;
  executeCommand(source.createCommand(file, workingDirectory, makeParameters), workingDirectory, fullOutput);
  fullOutput.remove(QLatin1String{"\\\n"});

  ///@todo collect multiple outputs at the same time for performance-reasons
  QString firstLine = fullOutput;
  int lineEnd;
  if ((lineEnd = fullOutput.indexOf(QLatin1Char('\n'))) != -1)
    firstLine.truncate(lineEnd); //Only look at the first line of output

  /**
   * There's two possible cases this can currently handle.
   * 1.: gcc is called, with the parameters we are searching for (so we parse the parameters)
   * 2.: A recursive make is called, within another directory(so we follow the recursion and try again) "cd /foo/bar && make -f pi/pa/build.make pi/pa/po.o
   * */

  ///STEP 1: Test if it is a recursive make-call
  // Do not search for recursive make-calls if we already have include-paths available. Happens in kernel modules.
  if (!includeRegularExpression().match(fullOutput).hasMatch()) {
    QRegExp makeRx(QStringLiteral("\\bmake\\s"));
    int offset = 0;
    while ((offset = makeRx.indexIn(firstLine, offset)) != -1) {
      auto prefix = QStringView{firstLine}.first(offset).trimmed().toString();
      if (prefix.endsWith(QLatin1String("&&")) || prefix.endsWith(QLatin1Char(';')) || prefix.isEmpty()) {
        QString newWorkingDirectory = workingDirectory;
        ///Extract the new working-directory
        if (!prefix.isEmpty()) {
          if (prefix.endsWith(QLatin1String("&&")))
            prefix.chop(2);
          else if (prefix.endsWith(QLatin1Char(';')))
            prefix.chop(1);

          ///Now test if what we have as prefix is a simple "cd /foo/bar" call.

          //In cases like "cd /media/data/kdedev/4.0/build/kdevelop && cd /media/data/kdedev/4.0/build/kdevelop"
          //We use the second directory. For t hat reason we search for the last index of "cd "
          int cdIndex = prefix.lastIndexOf(QLatin1String("cd "));
          if (cdIndex != -1) {
            newWorkingDirectory = QStringView{prefix}.sliced(cdIndex + 3).trimmed().toString();
            if (QFileInfo(newWorkingDirectory).isRelative())
              newWorkingDirectory = workingDirectory + QLatin1Char('/') + newWorkingDirectory;
            newWorkingDirectory = QDir::cleanPath(newWorkingDirectory);
          }
        }

        if (newWorkingDirectory == workingDirectory) {
          return PathResolutionResult(false, i18n("Failed to extract new working directory"), i18n("Output was: %1", fullOutput));
        }

        QFileInfo d(newWorkingDirectory);
        if (d.exists()) {
          ///The recursive working-directory exists.
          QString makeParams = firstLine.mid(offset+5);
          if (!makeParams.contains(QLatin1Char(';')) && !makeParams.contains(QLatin1String("&&"))) {
            ///Looks like valid parameters
            ///Make the file-name absolute, so it can be referenced from any directory
            QString absoluteFile = file;
            if (QFileInfo(absoluteFile).isRelative())
              absoluteFile = workingDirectory +  QLatin1Char('/') + file;
            Path absolutePath(absoluteFile);
            ///Try once with absolute, and if that fails with relative path of the file
            SourcePathInformation newSource(newWorkingDirectory);
            PathResolutionResult res = resolveIncludePathInternal(absolutePath.toLocalFile(), newWorkingDirectory, makeParams, newSource, maxDepth);
            if (res)
              return res;

            return resolveIncludePathInternal(Path(newWorkingDirectory).relativePath(absolutePath), newWorkingDirectory, makeParams , newSource, maxDepth);
          }else{
            return PathResolutionResult(false, i18n("Recursive make call failed"), i18n("The parameter string \"%1\" does not seem to be valid. Output was: %2.", makeParams, fullOutput));
          }
        } else {
          return PathResolutionResult(false, i18n("Recursive make call failed"), i18n("The directory \"%1\" does not exist. Output was: %2.", newWorkingDirectory, fullOutput));
        }

      } else {
        return PathResolutionResult(false, i18n("Malformed recursive make call"), i18n("Output was: %1", fullOutput));
      }

      ++offset;
      if (offset >= firstLine.length()) break;
    }
  }

  ///STEP 2: Search the output for include-paths

  PathResolutionResult ret = processOutput(fullOutput, workingDirectory);
  if (ret.paths.isEmpty() && ret.frameworkDirectories.isEmpty())
    return PathResolutionResult(false, i18n("Could not extract include paths from make output"),
                                i18n("Folder: \"%1\"  Command: \"%2\"  Output: \"%3\"", workingDirectory,
                                     source.createCommand(file, workingDirectory, makeParameters), fullOutput));
  return ret;
}

QRegularExpression MakeFileResolver::defineRegularExpression()
{
  static const QRegularExpression pattern(
    QStringLiteral("-(D|U)([^\\s=]+)(?:=(?:\"(.*?)(?<!\\\\)\"|([^\\s]*)))?")
  );
  Q_ASSERT(pattern.isValid());
  return pattern;
}

static QString unescape(QStringView input)
{
  QString output;
  output.reserve(input.length());
  bool isEscaped = false;
  for (const QChar c : input) {
    if (!isEscaped && c == QLatin1Char('\\')) {
      isEscaped = true;
    } else {
      output.append(c);
      isEscaped = false;
    }
  }
  return output;
}

QHash<QString, QString> MakeFileResolver::extractDefinesFromCompileFlags(const QString& compileFlags, StringInterner& stringInterner, QHash<QString, QString> defines)
{
  const auto& defineRx = defineRegularExpression();
  auto it = defineRx.globalMatch(compileFlags);
  while (it.hasNext()) {
    const auto match = it.next();
    const auto isUndefine = match.capturedView(1) == QLatin1String("U");
    const auto key = stringInterner.internString(match.captured(2));
    if (isUndefine) {
      defines.remove(key);
      continue;
    }
    QString value;
    if (match.lastCapturedIndex() > 2) {
      value = unescape(match.capturedView(match.lastCapturedIndex()));
    }
    defines[key] = stringInterner.internString(value);
  }
  return defines;
}

PathResolutionResult MakeFileResolver::processOutput(const QString& fullOutput, const QString& workingDirectory) const
{
  PathResolutionResult ret(true);
  ret.longErrorMessage = fullOutput;
  ifTest(cout << "full output: " << qPrintable(fullOutput) << endl);

  {
    const auto& includeRx = includeRegularExpression();
    auto it = includeRx.globalMatch(fullOutput);
    while (it.hasNext()) {
      const auto match = it.next();
      QString path = match.captured(2);
      if (path.startsWith(QLatin1Char('"')) || (path.startsWith(QLatin1Char('\'')) && path.length() > 2)) {
          //probable a quoted path
          if (path.endsWith(path.at(0))) {
            //Quotation is ok, remove it
            path = path.mid(1, path.length() - 2);
          }
      }
      if (QDir::isRelativePath(path))
        path = workingDirectory + QLatin1Char('/') + path;
      const auto& internedPath = m_pathInterner.internPath(path);
      const auto type = match.capturedView(1);
      const auto isFramework = type.startsWith(QLatin1String("-iframework"))
        || type.startsWith(QLatin1String("-F"));
      if (isFramework) {
        ret.frameworkDirectories << internedPath;
      } else {
        ret.paths << internedPath;
      }
    }
  }

  ret.defines = extractDefinesFromCompileFlags(fullOutput, m_stringInterner, ret.defines);

  return ret;
}

void MakeFileResolver::resetOutOfSourceBuild()
{
  m_outOfSource = false;
}

void MakeFileResolver::setOutOfSourceBuildSystem(const QString& source, const QString& build)
{
  if (source == build) {
    resetOutOfSourceBuild();
    return;
  }
  m_outOfSource = true;
  m_source = QDir::cleanPath(source);
  m_build = QDir::cleanPath(m_build);
}

PathInterner::PathInterner(const Path& base)
  : m_base(base)
{
}

Path PathInterner::internPath(const QString& path)
{
    Path& ret = m_pathCache[path];
    if (ret.isEmpty() != path.isEmpty()) {
        ret = Path(m_base, path);
    }
    return ret;
}

QString StringInterner::internString(const QString& path)
{
    return *m_stringCache.insert(path);
}
