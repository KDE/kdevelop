/*
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "includepathcomputer.h"
#include "cpputils.h"

#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>

#include <project/projectmodel.h>
#include <project/interfaces/ibuildsystemmanager.h>

#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/topducontext.h>
#include "cppduchain/environmentmanager.h"

#include <KLocalizedString>

using namespace KDevelop;

const bool enableIncludePathResolution = true;

IncludePathComputer::IncludePathComputer(const QString& file)
  : m_source(file)
  , m_ready(false)
  , m_gotPathsFromManager(false)
{
}

void IncludePathComputer::computeForeground()
{
  if (CppUtils::headerExtensions().contains(QFileInfo(m_source).suffix())) {
    // This file is a header. Since a header doesn't represent a target, we just try to get
    // the include-paths for the corresponding source-file, if there is one.
    QString newSource = CppUtils::sourceOrHeaderCandidate(m_source, true);
    if (!newSource.isEmpty()) {
      m_source = newSource;
    }
  }

  if (m_source.isEmpty()) {
    kDebug() << "cannot compute include-paths without source-file";
    return;
  }

  const IndexedString indexedSource(m_source);
  foreach (IProject *project, ICore::self()->projectController()->projects()) {
    QList<ProjectFileItem*> files = project->filesForPath(indexedSource);
    if (files.isEmpty()) {
      continue;
    }

    IBuildSystemManager* buildManager = project->buildSystemManager();
    if (!buildManager) {
      // We found the project, but no build manager!!
      kDebug() << "didn't get build manager for project:" << project->name();
      continue;
    }

    ProjectFileItem* file = files.last();
    Path::List dirs;
    // A file might be defined in different targets.
    // Prefer file items defined inside a target with non-empty includes.
    foreach (ProjectFileItem* f, files) {
      if (!dynamic_cast<ProjectTargetItem*>(f->parent())) {
        continue;
      }
      file = f;
      dirs = buildManager->includeDirectories(f);
      if (!dirs.isEmpty()) {
        break;
      }
    }
    // otherwise we did not find a target, so just use any include paths provided by the manager
    if (dirs.isEmpty()) {
      dirs = buildManager->includeDirectories(file);
    }

    m_projectName = project->name();
    m_projectDirectory = project->path();
    m_effectiveBuildDirectory = m_buildDirectory = buildManager->buildDirectory(project->projectItem());
    kDebug(9007) << "Got build-directory from project manager:" << m_effectiveBuildDirectory;

    if (m_projectDirectory == m_effectiveBuildDirectory) {
      m_projectDirectory.clear();
      m_effectiveBuildDirectory.clear();
    }

    m_gotPathsFromManager = !dirs.isEmpty();
    kDebug(9007) << "Got " << dirs.count() << " include-paths from build-manager";
    foreach (const Path& dir, dirs) {
      addInclude(dir);
    }

    m_defines = buildManager->defines(file);
  }

  if (!m_gotPathsFromManager) {
    kDebug(9007) << "Did not find any include paths from project manager for" << m_source;
  }
}

static Path::List toPaths(const QStringList& paths)
{
  Path::List ret;
  ret.reserve(paths.size());
  foreach (const QString& path, paths) {
    ret << Path(path);
  }
  return ret;
}

void IncludePathComputer::computeBackground()
{
  if (m_ready) {
    return;
  }

  //Insert standard-paths
  static const Path::List standardIncludePaths = toPaths(CppUtils::standardIncludePaths());
  foreach (const Path& path, standardIncludePaths) {
    addInclude(path);
  }

  if (!enableIncludePathResolution) {
    m_ready = true;
    return;
  }

  if (m_effectiveBuildDirectory.isValid()) {
    m_includeResolver.setOutOfSourceBuildSystem(m_projectDirectory.toLocalFile(), m_effectiveBuildDirectory.toLocalFile());
  } else {
    if (m_projectDirectory.isValid()) {
      //Report that the build-manager did not return the build-directory, for debugging
      kDebug(9007) << "Build manager for project %1 did not return a build directory" << m_projectName;
    }
    m_includeResolver.resetOutOfSourceBuild();
  }

  m_includePathDependency = m_includeResolver.findIncludePathDependency(m_source);
  kDebug() << "current include path dependency state:" << m_includePathDependency.toString();

  // only look at make when we did not get any paths from the build manager
  m_includeResolver.enableMakeResolution(!m_gotPathsFromManager);
  CppTools::PathResolutionResult result = m_includeResolver.resolveIncludePath(m_source);

  m_includePathDependency = result.includePathDependency;
  kDebug() << "new include path dependency:" << m_includePathDependency.toString();

  foreach (const QString &res, result.paths) {
    addInclude(Path(res));
  }

  if (!result) {
    kDebug(9007) << "Failed to resolve include-path for \"" << m_source << "\":"
                  << result.errorMessage << "\n" << result.longErrorMessage << "\n";

    // Last chance: Take a parsed version of the file from the du-chain, and get its include-paths
    // We will then get the include-path that some time was used to parse the file
    // NOTE: this is important for library headers, as when we open them they are not opened in any project
    // nor do they have a build folder which we could use
    DUChainReadLocker readLock;
    foreach(const ParsingEnvironmentFilePointer& file, DUChain::self()->allEnvironmentFiles(IndexedString(m_source))) {
      auto envFile = dynamic_cast<const Cpp::EnvironmentFile*>(file.data());
      if (envFile && !envFile->includePaths().isEmpty()) {
        foreach(const IndexedString& path, envFile->includePaths()) {
          addInclude(Path(path.toUrl()));
        }
        kDebug(9007) << "Took include-path for" << m_source << "from a random parsed duchain-version of it";
        break;
      }
    }
  }

  m_ready = true;
}

void IncludePathComputer::addInclude(const Path& path)
{
  if (!m_hasPath.contains(path)) {
    m_ret << path;
    m_hasPath.insert(path);
  }
}
