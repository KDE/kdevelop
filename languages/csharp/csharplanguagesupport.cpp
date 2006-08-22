/* This file is part of KDevelop
Copyright (C) 2006 Adam Treat <treat@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public License
along with this library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
Boston, MA 02110-1301, USA.
*/

#include <kdebug.h>
#include <kinstance.h>
#include <kstandarddirs.h>
#include <kgenericfactory.h>

#include <kdevast.h>
#include "parser/csharp_parser.h"
#include "parser/csharp_serialize_visitor.h"

#include <kdevcore.h>
#include <kdevproject.h>
#include <kdevfilemanager.h>
#include <kdevprojectmodel.h>
#include <kdevprojectcontroller.h>
#include <kdevdocumentcontroller.h>
#include <kdevbackgroundparser.h>
#include "parsejob.h"

#include "csharplanguagesupport.h"

#include "codeproxy.h"
#include "codedelegate.h"

#include <kdebug.h>

using namespace csharp;

typedef KGenericFactory<CSharpLanguageSupport> KDevCSharpSupportFactory;
K_EXPORT_COMPONENT_FACTORY( kdevcsharplanguagesupport,
                            KDevCSharpSupportFactory( "kdevcsharpsupport" ) )

CSharpLanguageSupport::CSharpLanguageSupport( QObject* parent,
        const QStringList& /*args*/ )
        : KDevLanguageSupport( KDevCSharpSupportFactory::instance(), parent )
{
    QString types = QLatin1String( "text/x-csharp" );
    m_mimetypes = types.split( "," );

    m_codeProxy = new csharp::CodeProxy( this );
    m_codeDelegate = new csharp::CodeDelegate( this );
    //     m_highlights = new CSharpHighlighting( this );

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
             SIGNAL( projectClosed() ),
             this, SLOT( projectClosed() ) );
}

CSharpLanguageSupport::~CSharpLanguageSupport()
{}

KDevCodeModel *CSharpLanguageSupport::codeModel( const KUrl &url ) const
{
    if ( url.isValid() )
        return m_codeProxy->codeModel( url );
    else
        return m_codeProxy->codeModel( KDevCore::documentController() ->activeDocumentUrl() );
}

KDevCodeProxy *CSharpLanguageSupport::codeProxy() const
{
    return m_codeProxy;
}

KDevCodeDelegate *CSharpLanguageSupport::codeDelegate() const
{
    return m_codeDelegate;
}

KDevCodeRepository *CSharpLanguageSupport::codeRepository() const
{
    return 0;
}

KDevParseJob *CSharpLanguageSupport::createParseJob( const KUrl &url )
{
    return new ParseJob( url, this );
}

KDevParseJob *CSharpLanguageSupport::createParseJob( KDevDocument *document )
{
    return new ParseJob( document, this );
}

QStringList CSharpLanguageSupport::mimeTypes() const
{
    return m_mimetypes;
}

void CSharpLanguageSupport::read( KDevAST * ast, std::ifstream &in )
{
    //FIXME Need to attach the memory pool to the ast somehow so it is saved
    parser::memory_pool_type memory_pool;

    // This is how we read the AST from a file
    if ( in.is_open() )
    {
        serialize::read( &memory_pool, static_cast<ast_node*>( ast ), &in );
    }
}

void CSharpLanguageSupport::write( KDevAST * ast, std::ofstream &out )
{
    // This is how we save the AST to a file
    if ( out.is_open() )
    {
        serialize::write( static_cast<ast_node*>( ast ), &out );
    }
}

void CSharpLanguageSupport::documentLoaded( KDevDocument *document )
{
    if ( supportsDocument( document ) )
        KDevCore::backgroundParser() ->addDocument( document );
}

void CSharpLanguageSupport::documentClosed( KDevDocument *document )
{
    if ( supportsDocument( document ) )
        KDevCore::backgroundParser() ->removeDocument( document );
}

void CSharpLanguageSupport::documentActivated( KDevDocument *document )
{
    Q_UNUSED( document );
}

void CSharpLanguageSupport::projectOpened()
{
    KUrl::List documentList;
    QList<KDevProjectFileItem*> files = KDevCore::activeProject() ->allFiles();
    foreach ( KDevProjectFileItem * file, files )
    {
        if ( /*supportsDocument( file->url() ) &&*/ file->url().fileName().endsWith( ".cs" ) )
        {
            documentList.append( file->url() );
        }
    }
    KDevCore::backgroundParser() ->addDocumentList( documentList );
}

void CSharpLanguageSupport::projectClosed()
{
    // FIXME This should remove the project files from the backgroundparser
}

#include "csharplanguagesupport.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
