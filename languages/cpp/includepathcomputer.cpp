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
#include "cpplanguagesupport.h"
#include "cpputils.h"
#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <project/projectmodel.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <klocalizedstring.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <QDirIterator>
#include <language/duchain/duchainutils.h>

using namespace KDevelop;

#define DEBUG_INCLUDE_PATHS 1
const bool enableIncludePathResolution = true;

static QList<KUrl> convertToUrls(const QList<IndexedString>& stringList)
{
  QList<KUrl> ret;
  foreach (const IndexedString& str, stringList) {
    ret << str.toUrl();
  }
  return ret;
}

IncludePathComputer::IncludePathComputer(const KUrl& file, QList<ProblemPointer>* problems)
  : m_source(file)
  , m_problems(problems)
  , m_ready(false)
  , m_gotPathsFromManager(false)
{
}

void IncludePathComputer::computeForeground()
{
  if (CppUtils::headerExtensions().contains(QFileInfo(m_source.toLocalFile()).suffix())) {
    // This file is a header. Since a header doesn't represent a target, we just try to get
    // the include-paths for the corresponding source-file, if there is one.
    KUrl newSource = CppUtils::sourceOrHeaderCandidate(m_source, true);
    if (newSource.isValid()) {
      m_source = newSource;
    }
  }

  if (m_source.isEmpty()) {
    foreach (const QString& path, CppUtils::standardIncludePaths()) {
      addInclude(KUrl(path));
    }
    kDebug() << "cannot compute include-paths without source-file";
    return;
  }

  foreach (IProject *project, ICore::self()->projectController()->projects()) {
    QList<ProjectFileItem*> files = project->filesForUrl(m_source);
    if (files.isEmpty()) {
      continue;
    }

    ProjectFileItem* file = 0;
    //A file might be defined in different targets.
    //Prefer file items defined inside a target, at least
    foreach (ProjectFileItem* f, files) {
      file = f;
      if (dynamic_cast<ProjectTargetItem*>(f->parent()))
        break;
    }

    IBuildSystemManager* buildManager = project->buildSystemManager();
    if (!buildManager) {
      // We found the project, but no build manager!!
      kDebug() << "didn't get build manager for project:" << project->name();
      continue;
    }

    m_projectName = project->name();
    m_projectDirectory = project->folder();
    m_effectiveBuildDirectory = m_buildDirectory = buildManager->buildDirectory(project->projectItem());
    kDebug(9007) << "Got build-directory from project manager:" << m_effectiveBuildDirectory;

    if (m_projectDirectory == m_effectiveBuildDirectory) {
      m_projectDirectory.clear();
      m_effectiveBuildDirectory.clear();
    }

    KUrl::List dirs = buildManager->includeDirectories(file);
    m_gotPathsFromManager = !dirs.isEmpty();
    kDebug(9007) << "Got " << dirs.count() << " include-paths from build-manager";
    foreach (KUrl dir, dirs) {
      addInclude(dir);
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

  const QList<QString>& standardPaths = CppUtils::standardIncludePaths();

  //Insert standard-paths
  foreach (const QString& path, standardPaths) {
    addInclude(KUrl(path));
  }

  if (m_gotPathsFromManager) {
    // return early and trust the include paths returned by the project manager
    return;
  }

  if (!m_effectiveBuildDirectory.isEmpty()) {
    m_includeResolver.setOutOfSourceBuildSystem(m_projectDirectory.toLocalFile(), m_effectiveBuildDirectory.toLocalFile());
  }

  m_includePathDependency = m_includeResolver.findIncludePathDependency(m_source.toLocalFile());
  kDebug() << "current include path dependency state:" << m_includePathDependency.toString();

  ///@todo Resolve include-paths either once per directory, or once per target. Do not iterate over the duchain.

  if (!m_ready) {
    static const IndexedString cpp("C++");
    //Try taking the include-paths from another file in the directory
    kDebug(9007) << "Did not get any include-paths for" << m_source;
    QFileInfo fileInfo(m_source.toLocalFile());
    QDirIterator it(fileInfo.dir().path());
    while(it.hasNext()) {
      QString file = it.next();
      foreach (const QString& ext, CppUtils::sourceExtensions()) {
        if (file != fileInfo.fileName() && file.endsWith(ext)) {
          DUChainReadLocker lock(DUChain::lock(), 300);
          if (lock.locked()) {
            TopDUContext* context = DUChainUtils::standardContextForUrl(KUrl(fileInfo.dir().absoluteFilePath(file)), true);
            if (context && context->parsingEnvironmentFile() && context->parsingEnvironmentFile()->language() == cpp
              && !context->parsingEnvironmentFile()->needsUpdate())
            {
              Cpp::EnvironmentFile* envFile = dynamic_cast<Cpp::EnvironmentFile*>(context->parsingEnvironmentFile().data());
              Q_ASSERT(envFile);
              if (!envFile->missingIncludeFiles().isEmpty() || envFile->includePathDependencies() != m_includePathDependency) {
                continue;
              }
              if (envFile->includePaths().size() <= standardPaths.size()) {
                continue;
              }
              foreach (const IndexedString& str, envFile->includePaths()) {
                m_ret << str.toUrl();
              }
              kDebug(9007) << "took include-paths for" <<  m_source << "from duchain of" <<  file;
              m_ready = true;
              return;
            }
          } else {
            return;
          }
        }
      }
    }
  }

  if (m_ready) {
    return;
  }

  if ((m_ret.isEmpty() || DEBUG_INCLUDE_PATHS) && enableIncludePathResolution) {
    //Fallback-search using include-path resolver

    if (!m_effectiveBuildDirectory.isEmpty()) {
        m_includeResolver.setOutOfSourceBuildSystem(m_projectDirectory.toLocalFile(), m_effectiveBuildDirectory.toLocalFile());
    } else {
      if (!m_projectDirectory.isEmpty() && m_problems) {
        //Report that the build-manager did not return the build-directory, for debugging
        Problem* newProblem = new Problem;
        newProblem->setSource(ProblemData::Preprocessor);
        newProblem->setDescription(i18n("Build manager for project %1 did not return a build directory", m_projectName));
        newProblem->setExplanation(i18n("The include path resolver needs the build directory to resolve additional include paths. "
                                        "Consider setting up a build directory in the project manager if you have not done so yet."));
        newProblem->setFinalLocation(DocumentRange(IndexedString(m_source), SimpleRange::invalid()));
        (*m_problems) << ProblemPointer(newProblem);
      }
      m_includeResolver.resetOutOfSourceBuild();
    }
    CppTools::PathResolutionResult result = m_includeResolver.resolveIncludePath(m_source.toLocalFile());
    m_includePathDependency = result.includePathDependency;
    kDebug() << "new include path dependency:" << m_includePathDependency.toString();

    foreach (const QString &res, result.paths) {
      addInclude(KUrl(res));
    }

    if (!result && m_problems) {
      kDebug(9007) << "Failed to resolve include-path for \"" << m_source << "\":"
                   << result.errorMessage << "\n" << result.longErrorMessage << "\n";
      ProblemPointer problem(new Problem);
      problem->setSource(ProblemData::Preprocessor);
      problem->setDescription(i18n("Include path resolver: %1", result.errorMessage));
      problem->setExplanation(i18n("Used build directory: \"%1\"\nInclude path resolver: %2",
                                    m_effectiveBuildDirectory.pathOrUrl(), result.longErrorMessage));
      problem->setFinalLocation(DocumentRange(IndexedString(m_source), SimpleRange::invalid()));
      *m_problems << problem;
    }
  }

  if (m_ret.isEmpty()) {
    // Last chance: Take a parsed version of the file from the du-chain, and get its include-paths
    // We will then get the include-path that some time was used to parse the file
    DUChainReadLocker readLock;
    TopDUContext* ctx = DUChain::self()->chainForDocument(m_source);
    if (ctx && ctx->parsingEnvironmentFile()) {
      Cpp::EnvironmentFile* envFile = dynamic_cast<Cpp::EnvironmentFile*>(ctx->parsingEnvironmentFile().data());
      if (envFile) {
        m_ret = convertToUrls(envFile->includePaths());
        m_hasPath += QSet<KUrl>::fromList(m_ret);
        kDebug(9007) << "Took include-path for" << m_source << "from a random parsed duchain-version of it";
      } else {
        kWarning() << "Missing cpp environment-file for" << m_source;
      }
    }
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
