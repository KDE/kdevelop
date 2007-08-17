/*
* KDevelop C++ Language Support
*
* Copyright 2005 Matt Rogers <mattr@kde.org>
* Copyright 2006 Adam Treat <treat@kde.org>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU Library General Public License as
* published by the Free Software Foundation; either version 2 of the
* License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public
* License along with this program; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include "cpplanguagesupport.h"
#include <config.h>

#include <QDir>
#include <QFileInfo>
#include <QApplication>
#include <QExtensionFactory>
#include <QtDesigner/QExtensionFactory>

#include <kdebug.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kgenericfactory.h>
#include <kio/netaccess.h>
#include <kparts/part.h>
#include <kparts/partmanager.h>
#include <ktexteditor/document.h>

#include <icore.h>
#include <iproject.h>
#include <idocument.h>
#include <idocumentcontroller.h>
#include <ilanguage.h>
#include <ilanguagecontroller.h>
#include <iprojectcontroller.h>
#include <ibuildsystemmanager.h>

#include <projectmodel.h>
#include <backgroundparser.h>
#include <duchain.h>
#include <duchainlock.h>
#include <topducontext.h>
#include <smartconverter.h>
#include <symboltable.h>

#include "rpp/preprocessor.h"
#include "ast.h"
#include "parsesession.h"
#include "cpphighlighting.h"
#include "cppcodecompletion.h"
#include "cppeditorintegrator.h"
#include "usebuilder.h"
#include "typerepository.h"
#include "cppparsejob.h"
#include "environmentmanager.h"
#include "macroset.h"

#include "includepathresolver.h"
#include "setuphelpers.h"

//#define DEBUG

using namespace KDevelop;

CppLanguageSupport* CppLanguageSupport::m_self = 0;

typedef KGenericFactory<CppLanguageSupport> KDevCppSupportFactory;
K_EXPORT_COMPONENT_FACTORY( kdevcpplanguagesupport, KDevCppSupportFactory( "kdevcppsupport" ) )

CppLanguageSupport::CppLanguageSupport( QObject* parent, const QStringList& /*args*/ )
    : KDevelop::IPlugin( KDevCppSupportFactory::componentData(), parent ),
      KDevelop::ILanguageSupport()
{
    m_self = this;
    
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::ILanguageSupport )

    m_highlights = new CppHighlighting( this );
    m_cc = new CppCodeCompletion( this );
    m_standardMacros = new Cpp::MacroSet;
    m_standardIncludePaths = new QStringList;
    m_environmentManager = new Cpp::EnvironmentManager;
    {
        DUChainWriteLocker l(DUChain::lock());
        DUChain::self()->addParsingEnvironmentManager(m_environmentManager);
    }

    m_includeResolver = new CppTools::IncludePathResolver;
    // Retrieve the standard include paths & macro definitions for this machine.
    // Uses gcc commands to retrieve the information.
    CppTools::setupStandardIncludePaths(*m_standardIncludePaths);
    CppTools::setupStandardMacros(*m_standardMacros);
    
    connect( core()->documentController(),
             SIGNAL( documentLoaded( KDevelop::IDocument* ) ),
             this, SLOT( documentLoaded( KDevelop::IDocument* ) ) );
    connect( core()->documentController(),
             SIGNAL( documentClosed( KDevelop::IDocument* ) ),
             this, SLOT( documentClosed( KDevelop::IDocument* ) ) );
    connect( core()->documentController(),
             SIGNAL( documentStateChanged( KDevelop::IDocument* ) ),
             this, SLOT( documentChanged( KDevelop::IDocument* ) ) );
    connect( core()->documentController(),
             SIGNAL( documentContentChanged( KDevelop::IDocument* ) ),
             this, SLOT( documentChanged( KDevelop::IDocument* ) ) );
    connect( core()->documentController(),
             SIGNAL( documentActivated( KDevelop::IDocument* ) ),
             this, SLOT( documentActivated( KDevelop::IDocument* ) ) );
    connect( core()->projectController(),
             SIGNAL( projectOpened( KDevelop::IProject* ) ),
             this, SLOT( projectOpened( KDevelop::IProject* ) ) );
    connect( core()->projectController(),
             SIGNAL( projectClosing( KDevelop::IProject* ) ),
             this, SLOT( projectClosing( KDevelop::IProject* ) ) );

    // Initialise singletons, to prevent needing a mutex in their self() methods
    TypeRepository::self();
    SymbolTable::self();
}

CppLanguageSupport::~CppLanguageSupport()
{
    m_self = 0;
    // Remove any documents waiting to be parsed from the background paser.
    core()->languageController()->backgroundParser()->clear(this);

    // Remove the corresponding parsing environment from the DUChain.
    {
        DUChainWriteLocker l(DUChain::lock());
        DUChain::self()->removeParsingEnvironmentManager(m_environmentManager);
    }

    delete m_standardMacros;
    delete m_standardIncludePaths;
    delete m_environmentManager;
    delete m_includeResolver;
}

const Cpp::MacroSet& CppLanguageSupport::standardMacros() const {
    return *m_standardMacros;
}

CppLanguageSupport* CppLanguageSupport::self() {
    return m_self;
}

void CppLanguageSupport::documentChanged( KDevelop::IDocument* document )
{
    core()->languageController()->backgroundParser()->addDocument(document->url());
}

KDevelop::ParseJob *CppLanguageSupport::createParseJob( const KUrl &url )
{
    return new CPPParseJob( url, this );
}

void CppLanguageSupport::documentLoaded(KDevelop::IDocument* doc)
{
    kDebug( 9007 ) << "CppLanguageSupport::documentLoaded";

    // Convert any duchains to the smart equivalent first
    EditorIntegrator editor;
    SmartConverter sc(&editor, codeHighlighting());

    QList<TopDUContext*> chains = DUChain::self()->chainsForDocument(doc->url());

    foreach (TopDUContext* chain, chains) {
        sc.convertDUChain(chain);
    }
    if( chains.isEmpty() )
        core()->languageController()->backgroundParser()->addDocument(doc->url());
}

KDevelop::ICodeHighlighting *CppLanguageSupport::codeHighlighting() const
{
    return m_highlights;
}

void CppLanguageSupport::projectOpened(KDevelop::IProject *project)
{
    Q_UNUSED(project)
    // FIXME Add signals slots from the filemanager for:
    //       1. filesAddedToProject
    //       2. filesRemovedFromProject
    //       3. filesChangedInProject
}

void CppLanguageSupport::projectClosing(KDevelop::IProject *project)
{
    Q_UNUSED(project)
    //TODO: Anything to do here?!?!
}

KUrl::List CppLanguageSupport::findIncludePaths(const KUrl& source) const
{
    KUrl::List allPaths;
    
    foreach (KDevelop::IProject *project, core()->projectController()->projects()) {
        KDevelop::ProjectFileItem *file = project->fileForUrl(source);
        if (!file) {
            continue;
        }

        KDevelop::IBuildSystemManager* buildManager = project->buildSystemManager();
        if (!buildManager) {
            // We found the project, but no build manager!!
            continue;
        }

        KUrl::List dirs = buildManager->includeDirectories(file);
        
        foreach( KUrl dir, dirs ) {
            dir.adjustPath(KUrl::AddTrailingSlash);
            allPaths << dir;
        }
    }

    if( allPaths.isEmpty() ) {
        //Fallback-search using include-path resolver

        CppTools::PathResolutionResult result = m_includeResolver->resolveIncludePath(source.path());
        if (result) {
            foreach( QString res, result.paths ) {
                KUrl r(res);
                r.adjustPath(KUrl::AddTrailingSlash);
                allPaths << r;
            }
        }else{
            kDebug(9007) << "Failed to resolve include-path for \"" << source << "\":" << result.errorMessage << "\n" << result.longErrorMessage << "\n";
        }
        
    }
    
    if( allPaths.isEmpty() ) {
        ///Last chance: Take a parsed version of the file from the du-chain, and get its include-paths(We will then get the include-path that some time was used to parse the file)
        KDevelop::DUChainReadLocker readLock(KDevelop::DUChain::lock());
        TopDUContext* ctx = KDevelop::DUChain::self()->chainForDocument(source);
        if( ctx && ctx->parsingEnvironmentFile() ) {
            Cpp::EnvironmentFile* envFile = dynamic_cast<Cpp::EnvironmentFile*>(ctx->parsingEnvironmentFile().data());
            Q_ASSERT(envFile);
            allPaths = envFile->includePaths();
            kDebug(9007) << "Took include-path for" << source << "from a random parsed duchain-version of it";
        }
    }

    //Insert the standard-paths at the end
    foreach( QString path, *m_standardIncludePaths)
        allPaths << KUrl(path);

    
    //Clean the list for better search-performance(remove multiple paths)
    QMap<KUrl, bool> hadUrls;
    for( KUrl::List::iterator it = allPaths.begin(); it != allPaths.end(); ) {
        if( hadUrls.contains(*it) )
            it = allPaths.erase(it);
        else {
            hadUrls[*it] = true;
            ++it;
        }
    }
    
    return allPaths;
}

QPair<KUrl, KUrl> CppLanguageSupport::findInclude(const KUrl::List& includePaths, const KUrl& localPath, const QString& includeName, int includeType, const KUrl& skipPath) const {
    QPair<KUrl, KUrl> ret;
#ifdef DEBUG
    kDebug(9007) << "searching for include-file" << includeName;
    if( !skipPath.isEmpty() )
        kDebug(9007) << "skipping path" << skipPath;
#endif
    
    if (includeType == rpp::Preprocessor::IncludeLocal && localPath != skipPath) {
        QFileInfo info(QDir(localPath.path()), includeName);
        if (info.exists() && info.isReadable()) {
            //kDebug(9007) << "found include file:" << info.absoluteFilePath();
            ret.first = KUrl(info.absoluteFilePath());
            ret.second = localPath;
            return ret;
        }
    }

    //When a path is skipped, we will start searching exactly after that path
    bool needSkip = !skipPath.isEmpty();

restart:
    foreach( KUrl path, includePaths ) {
        if( needSkip ) {
            if( path == skipPath ) {
                needSkip = false;
                continue;
            }
        }
            
        QFileInfo info(QDir( path.path() ), includeName);
        
        if (info.exists() && info.isReadable()) {
            //kDebug(9007) << "found include file:" << info.absoluteFilePath();
            ret.first = KUrl(info.absoluteFilePath());
            ret.second = path.path();
            return ret;
        }
    }

    if( needSkip ) {
        //The path to be skipped was not found, so simply start from the begin, considering any path.
        needSkip = false;
        goto restart;
    }

    if( ret.first.isEmpty() ) {
        kDebug(9007) << "FAILED to find include-file" << includeName << "in paths:";
        foreach( KUrl path, includePaths )
            kDebug(9007) << path;
    }
    
    return ret;
}

QString CppLanguageSupport::name() const
{
    return "C++";
}

KDevelop::ILanguage *CppLanguageSupport::language()
{
    return core()->languageController()->language(name());
}

#include "cpplanguagesupport.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
