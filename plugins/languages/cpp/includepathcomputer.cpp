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
#include <interfaces/iproject.h>
#include <interfaces/icore.h>
#include <interfaces/iprojectcontroller.h>
#include <project/projectmodel.h>
#include <project/interfaces/ibuildsystemmanager.h>
#include <klocalizedstring.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/duchain.h>

using namespace KDevelop;

#define DEBUG_INCLUDE_PATHS 1

QList<KUrl> convertToUrls(const QList<IndexedString>& stringList) {
  QList<KUrl> ret;
  foreach(const IndexedString& str, stringList)
    ret << KUrl(str.str());
  return ret;
}

IncludePathComputer::IncludePathComputer(const KUrl& file, QList<KDevelop::ProblemPointer>* problems) : m_source(file), m_problems(problems), m_ready(false), m_gotPathsFromManager(false) {
}

void IncludePathComputer::computeForeground() {

  if(headerExtensions.contains(QFileInfo(m_source.path()).suffix())) {
    //This file is a header. Since a header doesn't represent a target, we just try to get the include-paths for the corresponding source-file, if there is one.
    KUrl newSource = CppLanguageSupport::self()->sourceOrHeaderCandidate(m_source, true);
    if(newSource.isValid())
      m_source = newSource;
  }

  if( m_source.isEmpty() ) {
    foreach( const QString& path, CppLanguageSupport::self()->standardIncludePaths()) {
        KUrl u(path);
        if(!m_hasPath.contains(u)) {
          m_ret << KUrl(path);
          m_hasPath.insert(u);
        }
    }
    m_ready = true;
    return;
  }

    

    foreach (KDevelop::IProject *project, KDevelop::ICore::self()->projectController()->projects()) {
        QList<KDevelop::ProjectFileItem*> files = project->filesForUrl(m_source);
        ProjectFileItem* file = 0;
        if( !files.isEmpty() )
            file = files.first();
        if (!file) {
//                 kDebug() << "Didn't find file for url:" << m_source << "in project" << project->name();
            continue;
        }

        KDevelop::IBuildSystemManager* buildManager = project->buildSystemManager();
        if (!buildManager) {
                kDebug() << "didn't get build manager for project:" << project->name();
            // We found the project, but no build manager!!
            continue;
        }


        m_projectName = project->name();
        m_projectDirectory = project->folder();
        m_effectiveBuildDirectory = m_buildDirectory = buildManager->buildDirectory(project->projectItem());
        kDebug(9007) << "Got build-directory from project manager:" << m_effectiveBuildDirectory;

        if(m_projectDirectory == m_effectiveBuildDirectory)
            m_projectDirectory = m_effectiveBuildDirectory = KUrl();

        KUrl::List dirs = buildManager->includeDirectories(file);

        m_gotPathsFromManager = true;

        kDebug(9007) << "Got " << dirs.count() << " include-paths from build-manager";

        foreach( KUrl dir, dirs ) {
            dir.adjustPath(KUrl::AddTrailingSlash);
            if(!m_hasPath.contains(dir))
              m_ret << dir;
            m_hasPath.insert(dir);
        }
    }

    if(!m_gotPathsFromManager)
      kDebug(9007) << "Did not find a build-manager for" << m_source;
}



void IncludePathComputer::computeBackground() {
    if(m_ready)
      return;
    
    KDevelop::ProblemPointer problem(new KDevelop::Problem);

    if( m_ret.isEmpty() || DEBUG_INCLUDE_PATHS ) {
        //Fallback-search using include-path resolver

        if(!m_effectiveBuildDirectory.isEmpty()) {
            ///@todo remote directories?
            m_includeResolver.setOutOfSourceBuildSystem(m_projectDirectory.path(), m_effectiveBuildDirectory.path());
        } else {
            if(!m_projectDirectory.isEmpty() && m_problems) {
                //Report that the build-manager did not return the build-directory, for debugging
                KDevelop::Problem* newProblem = new Problem;
                newProblem->setSource(KDevelop::ProblemData::Preprocessor);
                newProblem->setDescription(i18n("Build manager for project %1 did not return a build directory", m_projectName));
                newProblem->setExplanation(i18n("The include path resolver needs the build directory to resolve additional include paths. Consider setting up a build directory in the project manager if you have not done so yet."));
                newProblem->setFinalLocation(DocumentRange(m_source.pathOrUrl(), KTextEditor::Range::invalid()));
                (*m_problems) << KDevelop::ProblemPointer(newProblem);
            }
            m_includeResolver.resetOutOfSourceBuild();
        }
        CppTools::PathResolutionResult result = m_includeResolver.resolveIncludePath(m_source.path());
        if (result) {
            bool hadMissingPath = false;
            if( !m_gotPathsFromManager ) {
                foreach( const QString &res, result.paths ) {
                    KUrl r(res);
                    r.adjustPath(KUrl::AddTrailingSlash);
                    if(!m_hasPath.contains(r))
                      m_ret << r;
                    m_hasPath.insert(r);
                }
            } else {
                //Compare the includes found by the includepathresolver to the ones returned by the project-manager, and complain eaach missing path.
                foreach( const QString& res, result.paths ) {

                    KUrl r(res);
                    r.adjustPath(KUrl::AddTrailingSlash);

                    KUrl r2(res);
                    r2.adjustPath(KUrl::RemoveTrailingSlash);

                    if( !m_hasPath.contains(r) && !m_hasPath.contains(r2) ) {
                        hadMissingPath = true;
                        if(!m_hasPath.contains(r))
                          m_ret << r;
                        m_hasPath.insert(r);

                        kDebug(9007) << "Include-path was missing in list returned by build-manager, adding it: " << r.pathOrUrl();

                        if( m_problems ) {
                          KDevelop::ProblemPointer p(new KDevelop::Problem);
                          p->setSource(KDevelop::ProblemData::Preprocessor);
                          p->setDescription(i18n("Build-manager did not return an include-path" ));
                          p->setExplanation(i18n("The build-manager did not return the include-path %1, which could be resolved by the include-path resolver", r.pathOrUrl()));
                          p->setFinalLocation(DocumentRange(m_source.pathOrUrl(), KTextEditor::Cursor(0,0), KTextEditor::Cursor(0,0)));
                          *m_problems << p;
                        }
                    }
                }

                if( hadMissingPath ) {
                    QString paths;
                    foreach( const KUrl& u, m_ret )
                        paths += u.pathOrUrl() + "\n";

                    kDebug(9007) << "Total list of include-paths:\n" << paths << "\nEnd of list";
                }
            }
        }else{
            kDebug(9007) << "Failed to resolve include-path for \"" << m_source << "\":" << result.errorMessage << "\n" << result.longErrorMessage << "\n";
            problem->setSource(KDevelop::ProblemData::Preprocessor);
            problem->setDescription(i18n("Include-path resolver:") + " " + result.errorMessage);
            problem->setExplanation(i18n("Used build directory: \"%1\"\nInclude-path resolver: %2", m_effectiveBuildDirectory.pathOrUrl(), result.longErrorMessage));
            problem->setFinalLocation(DocumentRange(m_source.pathOrUrl(), KTextEditor::Range::invalid()));
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
            Q_ASSERT(envFile);
            m_ret = convertToUrls(envFile->includePaths());
            kDebug(9007) << "Took include-path for" << m_source << "from a random parsed duchain-version of it";
            foreach(const KUrl &url, m_ret)
              m_hasPath.insert(url);
        }
    }

    //Insert the standard-paths at the end
    foreach( const QString& path, CppLanguageSupport::self()->standardIncludePaths()) {
      KUrl u(path);
      if(!m_hasPath.contains(u))
        m_ret << KUrl(path);
      m_hasPath.insert(u);
    }

    m_ready = true;
}
