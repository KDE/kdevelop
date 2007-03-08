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

#include "csharplanguagesupport.h"

#include <kdebug.h>
#include <kcomponentdata.h>
#include <kstandarddirs.h>
#include <kgenericfactory.h>

#include <QExtensionFactory>

/*
#include <kdevcore.h>
#include <kdevproject.h>
#include <kdevfilemanager.h>
#include <kdevprojectmodel.h>
#include <kdevprojectcontroller.h>
#include <kdevdocumentcontroller.h>
#include <kdevbackgroundparser.h>

#include <kdevast.h>
*/

// from the parser subdirectory
#include <csharp_parser.h>
#include <csharp_serialize_visitor.h>

#include "parsejob.h"
#include "codeproxy.h"
#include "codedelegate.h"

using namespace csharp;

typedef KGenericFactory<CSharpLanguageSupport> KDevCSharpSupportFactory;
K_EXPORT_COMPONENT_FACTORY( kdevcsharplanguagesupport, KDevCSharpSupportFactory( "kdevcsharpsupport" ) )

KDEV_USE_EXTENSION_INTERFACE_NS( KDevelop, ILanguageSupport, CSharpLanguageSupport )

CSharpLanguageSupport::CSharpLanguageSupport( QObject* parent,
                                              const QStringList& /*args*/ )
        : KDevelop::IPlugin( KDevCSharpSupportFactory::componentData(), parent )
        , KDevelop::ILanguageSupport()
{
    /*
    QString types = QLatin1String( "text/x-csharp" );
    m_mimetypes = types.split( "," );

    m_codeProxy = new csharp::CodeProxy( this );
    m_codeDelegate = new csharp::CodeDelegate( this );
    //     m_highlights = new CSharpHighlighting( this );

    connect( KDevelop::Core::documentController(),
             SIGNAL( documentLoaded( KDevelop::Document* ) ),
             this, SLOT( documentLoaded( KDevelop::Document* ) ) );
    connect( KDevelop::Core::documentController(),
             SIGNAL( documentClosed( KDevelop::Document* ) ),
             this, SLOT( documentClosed( KDevelop::Document* ) ) );
    connect( KDevelop::Core::documentController(),
             SIGNAL( documentActivated( KDevelop::Document* ) ),
             this, SLOT( documentActivated( KDevelop::Document* ) ) );
    connect( KDevelop::Core::projectController(),
             SIGNAL( projectOpened() ),
             this, SLOT( projectOpened() ) );
    connect( KDevelop::Core::projectController(),
             SIGNAL( projectClosed() ),
             this, SLOT( projectClosed() ) );
    */
}

CSharpLanguageSupport::~CSharpLanguageSupport()
{}

/*
KDevelop::CodeModel *CSharpLanguageSupport::codeModel( const KUrl &url ) const
{
    if ( url.isValid() )
        return m_codeProxy->codeModel( url );
    else
        return m_codeProxy->codeModel( KDevelop::Core::documentController() ->activeDocumentUrl() );
}

KDevelop::CodeProxy *CSharpLanguageSupport::codeProxy() const
{
    return m_codeProxy;
}

KDevelop::CodeDelegate *CSharpLanguageSupport::codeDelegate() const
{
    return m_codeDelegate;
}

KDevelop::CodeRepository *CSharpLanguageSupport::codeRepository() const
{
    return 0;
}

KDevelop::ParseJob *CSharpLanguageSupport::createParseJob( const KUrl &url )
{
    return new ParseJob( url, this );
}

KDevelop::ParseJob *CSharpLanguageSupport::createParseJob( KDevelop::Document *document )
{
    return new ParseJob( document, this );
}

QStringList CSharpLanguageSupport::mimeTypes() const
{
    return m_mimetypes;
}

void CSharpLanguageSupport::read( KDevelop::AST * ast, std::ifstream &in )
{
    //FIXME Need to attach the memory pool to the ast somehow so it is saved
    parser::memory_pool_type memory_pool;

    // This is how we read the AST from a file
    if ( in.is_open() )
    {
        serialize::read( &memory_pool, static_cast<ast_node*>( ast ), &in );
    }
}

void CSharpLanguageSupport::write( KDevelop::AST * ast, std::ofstream &out )
{
    // This is how we save the AST to a file
    if ( out.is_open() )
    {
        serialize::write( static_cast<ast_node*>( ast ), &out );
    }
}

void CSharpLanguageSupport::documentLoaded( KDevelop::Document *document )
{
    if ( supportsDocument( document ) )
        KDevelop::Core::backgroundParser() ->addDocument( document );
}

void CSharpLanguageSupport::documentClosed( KDevelop::Document *document )
{
    if ( supportsDocument( document ) )
        KDevelop::Core::backgroundParser() ->removeDocument( document );
}

void CSharpLanguageSupport::documentActivated( KDevelop::Document *document )
{
    Q_UNUSED( document );
}

void CSharpLanguageSupport::projectOpened()
{
    KUrl::List documentList;
    QList<KDevelop::ProjectFileItem*> files = KDevelop::Core::activeProject() ->allFiles();
    foreach ( KDevelop::ProjectFileItem * file, files )
    {
        if ( supportsDocument( file->url() ) /*&& file->url().fileName().endsWith( ".cs" )* / )
        {
            documentList.append( file->url() );
        }
    }
    KDevelop::Core::backgroundParser() ->addDocumentList( documentList );
}

void CSharpLanguageSupport::projectClosed()
{
    // FIXME This should remove the project files from the backgroundparser
}
                                             */

QString CSharpLanguageSupport::name() const
{
    return "C#";
}

QStringList CSharpLanguageSupport::extensions() const
{
    return QStringList() << "ILanguageSupport";
}

#include "csharplanguagesupport.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
