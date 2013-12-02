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

#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainutils.h>

#include <KLocalizedString>

#include <QDirIterator>
#include <QCoreApplication>

using namespace KDevelop;

#define DEBUG_INCLUDE_PATHS 1
const bool enableIncludePathResolution = true;

IncludePathComputer::IncludePathComputer(const QString& file, QList<ProblemPointer>* problems)
  : m_source(file)
  , m_problems(problems)
  , m_ready(false)
  , m_gotPathsFromManager(false)
{
}

void IncludePathComputer::computeForeground() {
  Q_ASSERT(QThread::currentThread() == qApp->thread());
  if(CppUtils::headerExtensions().contains(QFileInfo(m_source).suffix())) {
    //This file is a header. Since a header doesn't represent a target, we just try to get the include-paths for the corresponding source-file, if there is one.
    QString newSource = CppUtils::sourceOrHeaderCandidate(m_source, true);
    if(!newSource.isEmpty())
      m_source = newSource;
  }

  QFileInfo info(m_source);
  m_source = info.absoluteFilePath();

  if( m_source.isEmpty() ) {
    foreach( const QString& path, CppUtils::standardIncludePaths()) {
      addPath(path);
    }
    kDebug() << "cannot compute include-paths without source-file";
    return;
  }

  const IndexedString indexedSource(m_source);
    foreach (KDevelop::IProject *project, KDevelop::ICore::self()->projectController()->projects()) {
        QList<KDevelop::ProjectFileItem*> files = project->filesForPath(indexedSource);
        if (files.isEmpty())
            continue;

        ProjectFileItem* file = 0;
        //A file might be defined in different targets.
        //Prefer file items defined inside a target, at least
        foreach(ProjectFileItem* f, files) {
          file = f;
          if(dynamic_cast<ProjectTargetItem*>(f->parent()))
            break;
        }

        KDevelop::IBuildSystemManager* buildManager = project->buildSystemManager();
        if (!buildManager) {
                kDebug() << "didn't get build manager for project:" << project->name();
            // We found the project, but no build manager!!
            continue;
        }

        m_projectName = project->name();
        m_projectDirectory = project->path();
        m_effectiveBuildDirectory = m_buildDirectory = buildManager->buildDirectory(project->projectItem());
        kDebug(9007) << "Got build-directory from project manager:" << m_effectiveBuildDirectory;

        if(m_projectDirectory == m_effectiveBuildDirectory)
            m_projectDirectory = m_effectiveBuildDirectory = Path();

        Path::List dirs = buildManager->includeDirectories(file);

        m_defines = buildManager->defines(file);
        
        m_gotPathsFromManager = true;

        kDebug(9007) << "Got " << dirs.count() << " include-paths from build-manager";

        foreach( const Path& dir, dirs ) {
          addPath(dir.toLocalFile());
        }
    }

    if(!m_gotPathsFromManager)
      kDebug(9007) << "Did not find a build-manager for" << m_source;
}



void IncludePathComputer::computeBackground() {
    if(m_ready)
      return;
    
    if(m_effectiveBuildDirectory.isValid())
        m_includeResolver.setOutOfSourceBuildSystem(m_projectDirectory.toLocalFile(), m_effectiveBuildDirectory.toLocalFile());
    
    QList<QString> standardPaths = CppUtils::standardIncludePaths();
    
    m_includePathDependency = m_includeResolver.findIncludePathDependency(m_source);
    kDebug() << "current include path dependency state:" << m_includePathDependency.toString();
    
    ///@todo Resolve include-paths either once per directory, or once per target. Do not iterate over the duchain.
    
    if(!m_ready) {
      //Try taking the include-paths from another file in the directory
      kDebug(9007) << "Did not get any include-paths for" << m_source;
      QFileInfo fileInfo(m_source);
      QDirIterator it(fileInfo.dir().path());
      while(it.hasNext()) {
        QString file = it.next();
        foreach(const QString& ext, CppUtils::sourceExtensions()) {
          if(file != fileInfo.fileName() && file.endsWith(ext)) {
            DUChainReadLocker lock(DUChain::lock(), 300);
            if(lock.locked()) {
              
              TopDUContext* context = KDevelop::DUChainUtils::standardContextForUrl(KUrl(fileInfo.dir().absoluteFilePath(file)), true);
              if(context && context->parsingEnvironmentFile() && context->parsingEnvironmentFile()->language() == KDevelop::IndexedString("C++") && !context->parsingEnvironmentFile()->needsUpdate()) {
                Cpp::EnvironmentFile* envFile = dynamic_cast<Cpp::EnvironmentFile*>(context->parsingEnvironmentFile().data());
                Q_ASSERT(envFile);
                if(!envFile->missingIncludeFiles().isEmpty() || envFile->includePathDependencies() != m_includePathDependency)
                  continue;
                if(envFile->includePaths().size() <= standardPaths.size() )
                  continue;
                foreach(const KDevelop::IndexedString& str, envFile->includePaths()) {
                  str.str();
                }
                kDebug(9007) << "took include-paths for" <<  m_source << "from duchain of" <<  file;
                m_ready = true;
                return;
              }
            }else{
              return;
            }
          }
        }
      }
    }
    
    if(m_ready)
      return;
    
    KDevelop::ProblemPointer problem(new KDevelop::Problem);

    if( (m_ret.isEmpty() || DEBUG_INCLUDE_PATHS) && enableIncludePathResolution ) {
        //Fallback-search using include-path resolver

        if(m_effectiveBuildDirectory.isValid()) {
            m_includeResolver.setOutOfSourceBuildSystem(m_projectDirectory.toLocalFile(), m_effectiveBuildDirectory.toLocalFile());
        } else {
            if(m_projectDirectory.isValid() && m_problems) {
                //Report that the build-manager did not return the build-directory, for debugging
                KDevelop::Problem* newProblem = new Problem;
                newProblem->setSource(KDevelop::ProblemData::Preprocessor);
                newProblem->setDescription(i18n("Build manager for project %1 did not return a build directory", m_projectName));
                newProblem->setExplanation(i18n("The include path resolver needs the build directory to resolve additional include paths. Consider setting up a build directory in the project manager if you have not done so yet."));
                newProblem->setFinalLocation(DocumentRange(IndexedString(m_source), SimpleRange::invalid()));
                (*m_problems) << KDevelop::ProblemPointer(newProblem);
            }
            m_includeResolver.resetOutOfSourceBuild();
        }
        CppTools::PathResolutionResult result = m_includeResolver.resolveIncludePath(m_source);
        m_includePathDependency = result.includePathDependency;
        kDebug() << "new include path dependency:" << m_includePathDependency.toString();
        
//         if (result) {
          if( !m_gotPathsFromManager ) {
              foreach( const QString &res, result.paths ) {
                addPath(res);
              }
          } else {
              //Compare the includes found by the includepathresolver to the ones returned by the project-manager, and complain eaach missing path.
              foreach( const QString& res, result.paths ) {
                  if( !m_hasPath.contains(res) ) {
                      addPath(res);

                      kDebug(9007) << "Include-path was missing in list returned by build-manager, adding it now. file was:" << m_source << "missing path:" << res;
                  }
              }
          }
        if(!result) {
            kDebug(9007) << "Failed to resolve include-path for \"" << m_source << "\":" << result.errorMessage << "\n" << result.longErrorMessage << "\n";
            problem->setSource(KDevelop::ProblemData::Preprocessor);
            problem->setDescription(i18n("Include path resolver: %1", result.errorMessage));
            problem->setExplanation(i18n("Used build directory: \"%1\"\nInclude path resolver: %2", m_effectiveBuildDirectory.pathOrUrl(), result.longErrorMessage));
            problem->setFinalLocation(DocumentRange(IndexedString(m_source), SimpleRange::invalid()));
            
        }
    }

    if( m_ret.isEmpty() && problem->source() != KDevelop::ProblemData::Unknown && m_problems )
      *m_problems << problem;

    if( m_ret.isEmpty() ) {
        ///Last chance: Take a parsed version of the file from the du-chain, and get its include-paths(We will then get the include-path that some time was used to parse the file)
        KDevelop::DUChainReadLocker readLock(KDevelop::DUChain::lock());
        TopDUContext* ctx = KDevelop::DUChain::self()->chainForDocument(m_source);
        if( ctx && ctx->parsingEnvironmentFile() ) {
            Cpp::EnvironmentFile* envFile = dynamic_cast<Cpp::EnvironmentFile*>(ctx->parsingEnvironmentFile().data());
            if(envFile)
            {
              kDebug(9007) << "Took include-path for" << m_source << "from a random parsed duchain-version of it";
              foreach(const IndexedString &indexedPath, envFile->includePaths()) {
                addPath(indexedPath.str());
              }
            }else{
              kWarning() << "Missing cpp environment-file for" << m_source;
            }
        }
    }

    //Insert the standard-paths at the end
    foreach( const QString& path, standardPaths) {
      addPath(path);
    }

    m_ready = true;
}

void IncludePathComputer::addPath(const QString& path)
{
  if (!m_hasPath.contains(path)) {
    m_hasPath.insert(path);
    m_ret.append(path);
  }
}
