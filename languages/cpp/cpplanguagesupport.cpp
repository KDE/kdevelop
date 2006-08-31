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

#include "config.h"

#ifdef HAVE_VALGRIND_H
#include <valgrind/valgrind.h>
#endif

#include <QMutex>
#include <QMutexLocker>
#include <QApplication>

#include <kdebug.h>
#include <kinstance.h>
#include <kstandarddirs.h>
#include <kgenericfactory.h>
#include <kio/netaccess.h>

#include <kdevcore.h>
#include <kdevproject.h>
#include <kdevfilemanager.h>
#include <kdevbuildmanager.h>
#include <kdevprojectmodel.h>
#include <kdevprojectcontroller.h>
#include <kdevdocumentcontroller.h>
#include <kdevbackgroundparser.h>


#include "cpplanguagesupport.h"
#include "cpphighlighting.h"

#include "parser/codemodel.h"
#include "parser/ast.h"
#include "parser/parsesession.h"

#include "duchain/duchain.h"
#include "duchain/topducontext.h"
#include "duchain/smartconverter.h"
#include "duchain/cppeditorintegrator.h"
#include "duchain/usebuilder.h"

#include "parsejob.h"
#include "codeproxy.h"
#include "codedelegate.h"

#include <kdebug.h>

typedef KGenericFactory<CppLanguageSupport> KDevCppSupportFactory;
K_EXPORT_COMPONENT_FACTORY( kdevcpplanguagesupport, KDevCppSupportFactory( "kdevcppsupport" ) )

CppLanguageSupport::CppLanguageSupport( QObject* parent,
                                        const QStringList& /*args*/ )
        : KDevLanguageSupport( KDevCppSupportFactory::instance(), parent )
{
    QString types =
        QLatin1String( "text/x-chdr,text/x-c++hdr,text/x-csrc,text/x-c++src" );
    m_mimetypes = types.split( "," );

    m_codeProxy = new CodeProxy( this );
    m_codeDelegate = new CodeDelegate( this );
    m_highlights = new CppHighlighting( this );

    connect( KDevCore::documentController(),
             SIGNAL( documentLoaded( KDevDocument* ) ),
             this, SLOT( documentLoaded( KDevDocument* ) ) );
    connect( KDevCore::documentController(),
             SIGNAL( documentClosed( KDevDocument* ) ),
             this, SLOT( documentClosed( KDevDocument* ) ) );
    connect( KDevCore::documentController(),
             SIGNAL( documentActivated( KDevDocument* ) ),
             this, SLOT( documentActivated( KDevDocument* ) ) );
    connect( KDevCore::projectController(),
             SIGNAL( projectOpened() ),
             this, SLOT( projectOpened() ) );
    connect( KDevCore::projectController(),
             SIGNAL( projectClosing() ),
             this, SLOT( projectClosing() ) );
}

CppLanguageSupport::~CppLanguageSupport()
{
    // Ensure all parse jobs have finished before this object goes away
    lockAllParseMutexes();
    unlockAllParseMutexes();
}

KDevCodeModel *CppLanguageSupport::codeModel( const KUrl &url ) const
{
    if ( url.isValid() )
        return m_codeProxy->codeModel( url );
    else
        return m_codeProxy->codeModel( KDevCore::documentController() ->activeDocumentUrl() );
}

KDevCodeProxy *CppLanguageSupport::codeProxy() const
{
    return m_codeProxy;
}

KDevCodeDelegate *CppLanguageSupport::codeDelegate() const
{
    return m_codeDelegate;
}

KDevCodeRepository *CppLanguageSupport::codeRepository() const
{
    return 0;
}

KDevParseJob *CppLanguageSupport::createParseJob( const KUrl &url )
{
    return new CPPParseJob( url, this );
}

KDevParseJob *CppLanguageSupport::createParseJob( KDevDocument *document )
{
    return new CPPParseJob( document, this );
}

QStringList CppLanguageSupport::mimeTypes() const
{
    return m_mimetypes;
}

void CppLanguageSupport::documentLoaded( KDevDocument *document )
{
    if ( supportsDocument( document ) )
        KDevCore::backgroundParser() ->addDocument( document );
}

void CppLanguageSupport::documentClosed( KDevDocument *document )
{
    if ( supportsDocument( document ) )
        KDevCore::backgroundParser() ->removeDocument( document );
}

void CppLanguageSupport::documentActivated( KDevDocument *document )
{
    Q_UNUSED( document );
}

KDevCodeHighlighting *CppLanguageSupport::codeHighlighting() const
{
    return m_highlights;
}

void CppLanguageSupport::projectOpened()
{
#ifdef HAVE_VALGRIND_H
    // If running on valgrind, don't background parse all project files...!!
    if (RUNNING_ON_VALGRIND > 0)
        return;
#endif

    // FIXME Add signals slots from the filemanager for:
    //       1. filesAddedToProject
    //       2. filesRemovedFromProject
    //       3. filesChangedInProject

    // FIXME this should be moved to the project itself
    KUrl::List documentList;
    QList<KDevProjectFileItem*> files = KDevCore::activeProject()->allFiles();
    foreach ( KDevProjectFileItem * file, files )
    {
        if ( supportsDocument( file->url() ) )
        {
            documentList.append( file->url() );
        }
    }
    KDevCore::backgroundParser() ->addDocumentList( documentList );
}

void CppLanguageSupport::projectClosing()
{
    KDevCore::backgroundParser()->clear(this);

    // FIXME I think this only happens on kdevelop close, but it would be good to figure it out
    // and fix it
    if (KDevCore::activeProject()) {
        KUrl::List documentList;
        QList<KDevProjectFileItem*> files = KDevCore::activeProject()->allFiles();
        foreach ( KDevProjectFileItem * file, files )
        {
            if ( supportsDocument( file->url() ) )
            {
                KDevCore::backgroundParser() ->removeDocument( file->url() );
            }
        }
    }

    lockAllParseMutexes();

    // Now we can do destructive stuff

    DUChain::self()->clear();

    unlockAllParseMutexes();
}

void CppLanguageSupport::releaseAST( KDevAST *ast)
{
    TranslationUnitAST* t = static_cast<TranslationUnitAST*>(ast);
    delete t->session;
    // The ast is in the memory pool which has been deleted as part of the session.
}

KUrl CppLanguageSupport::findInclude( const QString& fileName )
{
    // TODO: require that the target which specified the original file be passed, so that the correct set of includes can be retrieved rather than all of them

    KUrl ret;

    if (KDevProject* project = KDevCore::activeProject()) {
        if (KDevBuildManager* buildManager = dynamic_cast<KDevBuildManager*>( project->fileManager() )) {
            foreach (KUrl u, buildManager->includeDirectories()) {
                u.adjustPath( KUrl::AddTrailingSlash );

                KUrl newUrl (u, fileName);
                //kDebug(9007) << k_funcinfo << "checking for existance of " << newUrl << endl;
                if ( KIO::NetAccess::exists( newUrl, true, qApp->activeWindow() ) ) {
                    ret = newUrl;
                    break;
                }
            }

            //if (!ret.isValid())
                //kWarning(7009) << k_funcinfo << "Could not find include " << fileName << " in  " << buildManager->includeDirectories() << endl;
        }
    }

    return ret;
}

void CppLanguageSupport::documentLoaded(KDevAST * ast, const KUrl & document)
{
    // Pretty heavy handed - find another way?
    lockAllParseMutexes();

    TopDUContext* context = DUChain::self()->chainForDocument(document);
    if (context) {
        TranslationUnitAST* t = static_cast<TranslationUnitAST*>(ast);
        CppEditorIntegrator editor(t->session);
        {
            SmartConverter sc(&editor);
            sc.convertDUChain(context);

            if (!context->hasUses()) {
                UseBuilder ub(&editor);
                ub.buildUses(t);
            }

            m_highlights->highlightDUChain(context);
        }
    }

    unlockAllParseMutexes();
}

#include "cpplanguagesupport.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
