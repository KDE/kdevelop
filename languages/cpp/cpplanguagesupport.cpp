/*
* KDevelop C++ Language Support
*
* Copyright (c) 2005 Matt Rogers <mattr@kde.org>
* Copyright (c) 2006 Adam Treat <treat@kde.org>
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

#include "config.h"
/*
#ifdef HAVE_VALGRIND_H
#include <valgrind/valgrind.h>
#endif
*/
#include <QMutex>
#include <QMutexLocker>
#include <QApplication>
#include <QExtensionFactory>

#include <kdebug.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kgenericfactory.h>
#include <kio/netaccess.h>
#include <kparts/part.h>
#include <kparts/partmanager.h>

#include "icore.h"
#include "iproject.h"
#include "ilanguage.h"
#include "ilanguagecontroller.h"
#include "iprojectcontroller.h"
#include "ibuildsystemmanager.h"

#include "projectmodel.h"
#include "backgroundparser.h"

/*
#include <kdevcore.h>
#include <kdevproject.h>
#include <kdevfilemanager.h>
#include <kdevbuildmanager.h>
#include <kdevprojectmodel.h>
#include <kdevprojectcontroller.h>
#include <kdevdocumentcontroller.h>
#include <kdevbackgroundparser.h>
#include <kdevpersistenthash.h>
*/
#include "cpphighlighting.h"

// #include "parser/codemodel.h"
#include "parser/ast.h"
#include "parser/parsesession.h"

#include "duchain/duchain.h"
#include "duchain/topducontext.h"
#include "duchain/smartconverter.h"
#include "duchain/cppeditorintegrator.h"
#include "duchain/usebuilder.h"
#include "duchain/symboltable.h"
#include "duchain/typerepository.h"

#include "cppparsejob.h"
// #include "codeproxy.h"
// #include "codedelegate.h"
#include "cppcodecompletion.h"
#include <QtDesigner/QExtensionFactory>

typedef KGenericFactory<CppLanguageSupport> KDevCppSupportFactory;
K_EXPORT_COMPONENT_FACTORY( kdevcpplanguagesupport, KDevCppSupportFactory( "kdevcppsupport" ) )

CppLanguageSupport::CppLanguageSupport( QObject* parent,
                                        const QStringList& /*args*/ )
        : KDevelop::IPlugin( KDevCppSupportFactory::componentData(), parent ),
        KDevelop::ILanguageSupport()
{
    KDEV_USE_EXTENSION_INTERFACE( KDevelop::ILanguageSupport )
    m_highlights = new CppHighlighting( this );
    m_cc = new CppCodeCompletion( this );

/*    connect( KDevelop::Core::documentController(),
             SIGNAL( documentLoaded( KDevelop::Document* ) ),
             this, SLOT( documentLoaded( KDevelop::Document* ) ) );
    connect( KDevelop::Core::documentController(),
             SIGNAL( documentClosed( KDevelop::Document* ) ),
             this, SLOT( documentClosed( KDevelop::Document* ) ) );
    connect( KDevelop::Core::documentController(),
             SIGNAL( documentActivated( KDevelop::Document* ) ),
             this, SLOT( documentActivated( KDevelop::Document* ) ) );*/
    connect( core()->projectController(),
             SIGNAL( projectOpened( KDevelop::IProject* ) ),
             this, SLOT( projectOpened( KDevelop::IProject* ) ) );
    connect( core()->projectController(),
             SIGNAL( projectClosing( KDevelop::IProject* ) ),
             this, SLOT( projectClosing( KDevelop::IProject* ) ) );

    ///@todo these connects should be here until proper document controller system is in place
    connect( core()->partManager(), SIGNAL( partAdded(KParts::Part*)),
             this, SLOT( documentActivated(KParts::Part*) ) );
    connect( core()->partManager(), SIGNAL( partRemoved(KParts::Part*)),
             this, SLOT( documentClosed(KParts::Part*) ) );

    // Initialise singletons, to prevent needing a mutex in their self() methods
    TypeRepository::self();
    SymbolTable::self();
}

CppLanguageSupport::~CppLanguageSupport()
{
    // Ensure all parse jobs have finished before this object goes away
/*    lockAllParseMutexes();
    unlockAllParseMutexes();*/
}
/*
KDevelop::CodeModel *CppLanguageSupport::codeModel( const KUrl &url ) const
{
    if ( url.isValid() )
        return m_codeProxy->codeModel( url );
    else
        return m_codeProxy->codeModel( KDevelop::Core::documentController() ->activeDocumentUrl() );
}

KDevelop::CodeProxy *CppLanguageSupport::codeProxy() const
{
    return m_codeProxy;
}

KDevelop::CodeDelegate *CppLanguageSupport::codeDelegate() const
{
    return m_codeDelegate;
}

KDevelop::CodeRepository *CppLanguageSupport::codeRepository() const
{
    return 0;
}
*/
KDevelop::ParseJob *CppLanguageSupport::createParseJob( const KUrl &url )
{
    return new CPPParseJob( url, this );
}
/*
KDevelop::ParseJob *CppLanguageSupport::createParseJob( KDevelop::Document *document )
{
    return new CPPParseJob( document, this );
}

QStringList CppLanguageSupport::mimeTypes() const
{
    return m_mimetypes;
}

void CppLanguageSupport::documentLoaded( KDevelop::Document *document )
{
    if ( supportsDocument( document ) )
        KDevelop::Core::backgroundParser() ->addDocument( document );
}

void CppLanguageSupport::documentClosed( KDevelop::Document *document )
{
    if ( supportsDocument( document ) )
        KDevelop::Core::backgroundParser() ->removeDocument( document );
}

void CppLanguageSupport::documentActivated( KDevelop::Document *document )
{
    Q_UNUSED( document );
}
*/
void CppLanguageSupport::documentActivated(KParts::Part *part)
{
    kDebug( 9007 ) << "CppLanguageSupport::documentActivated" << endl;
    if (KParts::ReadOnlyPart *ropart = dynamic_cast<KParts::ReadOnlyPart*>(part))
    {
        kDebug( 9007 ) << "adding document to bgparser" << endl;
        language()->backgroundParser()->addDocument(ropart->url());
    }
}

void CppLanguageSupport::documentClosed(KParts::Part *part)
{
    kDebug( 9007 ) << "CppLanguageSupport::documentClosed" << endl;
    if (KParts::ReadOnlyPart *ropart = dynamic_cast<KParts::ReadOnlyPart*>(part))
    {
        kDebug( 9007 ) << "removing document from bgparser" << endl;
        language()->backgroundParser()->removeDocument(ropart->url());
    }
}

KDevelop::CodeHighlighting *CppLanguageSupport::codeHighlighting() const
{
    return m_highlights;
}

void CppLanguageSupport::projectOpened(KDevelop::IProject *project)
{
#ifdef HAVE_VALGRIND_H
/*    // If running on valgrind, don't background parse all project files...!!
    if (RUNNING_ON_VALGRIND > 0)
        return;*/
#endif

    // FIXME Add signals slots from the filemanager for:
    //       1. filesAddedToProject
    //       2. filesRemovedFromProject
    //       3. filesChangedInProject

    // FIXME this should be moved to the project itself
    KUrl::List documentList;
    foreach ( KDevelop::ProjectFileItem * file, project->files() )
    {
        ///@todo implement ILanguage::supportsDocument or smth like that
//         if ( language()->supportsDocument( file->url() ) )
//         {
            documentList.append( file->url() );
//         }
    }
    language()->backgroundParser()->addDocumentList( documentList );
}

void CppLanguageSupport::projectClosing(KDevelop::IProject *project)
{
    language()->backgroundParser()->clear(this);

    // FIXME I think this only happens on kdevelop close, but it would be good to figure it out
    // and fix it
    KUrl::List documentList;
    foreach ( KDevelop::ProjectFileItem * file, project->files() )
    {
        ///@todo implement ILanguage::supportsDocument or smth like that
//         if ( language()->supportsDocument( file->url() ) )
//         {
            language()->backgroundParser() ->removeDocument( file->url() );
//         }
    }

    language()->lockAllParseMutexes();

    // Now we can do destructive stuff

    DUChain::self()->clear();

    language()->unlockAllParseMutexes();
}
/*
void CppLanguageSupport::releaseAST( KDevelop::AST *ast)
{
    TranslationUnitAST* t = static_cast<TranslationUnitAST*>(ast);
    delete t->session;
    // The ast is in the memory pool which has been deleted as part of the session.
}
*/
KUrl CppLanguageSupport::findInclude(const KUrl &source, const QString& includeName )
{
    // TODO: require that the target which specified the original file be passed, so that the correct set of includes can be retrieved rather than all of them

    KUrl ret;

    foreach (KDevelop::IProject *project, core()->projectController()->projects())
    {
        if (!project->inProject(source))
            continue;

        KDevelop::IBuildSystemManager* buildManager =
            dynamic_cast<KDevelop::IBuildSystemManager*>(project->fileManager());
        if (!buildManager)
            continue;
        KDevelop::ProjectFileItem *file = project->fileForUrl(source);
        if (!file)
            continue;

        foreach (KUrl u, buildManager->includeDirectories(file)) {
            u.adjustPath( KUrl::AddTrailingSlash );

            KUrl newUrl (u, includeName);
            //kDebug(9007) << k_funcinfo << "checking for existance of " << newUrl << endl;
            if ( KIO::NetAccess::exists( newUrl, true, qApp->activeWindow() ) ) {
                ret = newUrl;
                break;
            }
        }
    }

    return ret;
}

void CppLanguageSupport::documentLoaded(TranslationUnitAST *ast, const KUrl & document)
{
    // Pretty heavy handed - find another way?
    // TODO: use the definition-use chain locking here, rather than in the builders?
    language()->lockAllParseMutexes();

    TopDUContext* context = DUChain::self()->chainForDocument(document);
    if (context) {
        CppEditorIntegrator editor(ast->session);
        {
            SmartConverter sc(&editor, m_highlights);
            sc.convertDUChain(context);

            if (!context->hasUses()) {
                UseBuilder ub(&editor);
                ub.buildUses(ast);
            }
        }
    }

    language()->unlockAllParseMutexes();
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
