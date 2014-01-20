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

#include <KLocalizedString>

using namespace KDevelop;

const bool enableIncludePathResolution = true;

IncludePathComputer::IncludePathComputer(const KUrl& file)
  : m_source(file)
  , m_ready(false)
  , m_gotPathsFromManager(false)
{
}

void IncludePathComputer::computeForeground()
{
  if (CppUtils::headerExtensions().contains(QFileInfo(m_source.toLocalFile()).suffix())) {
    // This file is a header. Since a header doesn't represent a target, we just try to get
    // the include-paths for the corresponding source-file, if there is one.
    KUrl newSource = CppUtils::sourceOrHeaderCandidate(m_source.toLocalFile(), true);
    if (newSource.isValid()) {
      m_source = newSource;
    }
  }

  if (m_source.isEmpty()) {
    kDebug() << "cannot compute include-paths without source-file";
    return;
  }

  foreach (IProject *project, ICore::self()->projectController()->projects()) {
    QList<ProjectFileItem*> files = project->filesForUrl(m_source);
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
    /// TODO: port this mess to Path API
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

    m_projectName = project->name();
    m_projectDirectory = project->folder();
    m_effectiveBuildDirectory = m_buildDirectory = buildManager->buildDirectory(project->projectItem()).toUrl();
    kDebug(9007) << "Got build-directory from project manager:" << m_effectiveBuildDirectory;

    if (m_projectDirectory == m_effectiveBuildDirectory) {
      m_projectDirectory.clear();
      m_effectiveBuildDirectory.clear();
    }

    m_gotPathsFromManager = !dirs.isEmpty();
    kDebug(9007) << "Got " << dirs.count() << " include-paths from build-manager";
    foreach (const Path& dir, dirs) {
      addInclude(dir.toUrl());
    }

    m_defines = buildManager->defines(file);
  }

  if (!m_gotPathsFromManager) {
    kDebug(9007) << "Did not find any include paths from project manager for" << m_source;
  }
}

void IncludePathComputer::computeBackground()
{
  if (m_ready) {
    return;
  }

  //Insert standard-paths
  foreach (const QString& path, CppUtils::standardIncludePaths()) {
    addInclude(KUrl(path));
  }

  if (!enableIncludePathResolution) {
    m_ready = true;
    return;
  }

  if (!m_effectiveBuildDirectory.isEmpty()) {
    m_includeResolver.setOutOfSourceBuildSystem(m_projectDirectory.toLocalFile(), m_effectiveBuildDirectory.toLocalFile());
  } else {
    if (!m_projectDirectory.isEmpty()) {
      //Report that the build-manager did not return the build-directory, for debugging
      kDebug(9007) << "Build manager for project %1 did not return a build directory" << m_projectName;
    }
    m_includeResolver.resetOutOfSourceBuild();
  }

  m_includePathDependency = m_includeResolver.findIncludePathDependency(m_source.toLocalFile());
  kDebug() << "current include path dependency state:" << m_includePathDependency.toString();

  // only look at make when we did not get any paths from the build manager
  m_includeResolver.enableMakeResolution(!m_gotPathsFromManager);
  CppTools::PathResolutionResult result = m_includeResolver.resolveIncludePath(m_source.toLocalFile());

  m_includePathDependency = result.includePathDependency;
  kDebug() << "new include path dependency:" << m_includePathDependency.toString();

  foreach (const QString &res, result.paths) {
    addInclude(KUrl(res));
  }

  if (!result) {
    kDebug(9007) << "Failed to resolve include-path for \"" << m_source << "\":"
                  << result.errorMessage << "\n" << result.longErrorMessage << "\n";
  }

  m_ready = true;
}

void IncludePathComputer::addInclude(KUrl url)
{
  url.cleanPath();
  url.adjustPath(KUrl::AddTrailingSlash);
  if (!m_hasPath.contains(url)) {
    m_ret << url;
    m_hasPath.insert(url);
  }
}
