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

#include <kdevcore.h>
#include <kdevproject.h>
#include <kdevfilemanager.h>
#include <kdevprojectmodel.h>
#include <kdevprojectcontroller.h>
#include <kdevdocumentcontroller.h>
#include <kdevbackgroundparser.h>
#include "parsejob.h"

#include "csharplanguagesupport.h"

// #include "codeproxy.h"
// #include "codedelegate.h"

#include <kdebug.h>

typedef KGenericFactory<CSharpLanguageSupport> KDevCSharpSupportFactory;
K_EXPORT_COMPONENT_FACTORY( kdevcsharplanguagesupport,
                            KDevCSharpSupportFactory( "kdevcsharpsupport" ) )

CSharpLanguageSupport::CSharpLanguageSupport( QObject* parent,
                                              const QStringList& /*args*/ )
        : KDevLanguageSupport( KDevCSharpSupportFactory::instance(), parent )
{
    QString types = QLatin1String( "text/x-csharp" );
    m_mimetypes = types.split( "," );

    m_memoryPool = new parser::memory_pool_type;
    //     m_codeProxy = new CodeProxy( this );
    //     m_codeDelegate = new CodeDelegate( this );
    //     m_backgroundParser = new BackgroundParser( this );
    //     m_highlights = new CppHighlighting( this );

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
{
    delete m_memoryPool;
}

KDevCodeModel *CSharpLanguageSupport::codeModel( const KUrl &url ) const
{
    Q_UNUSED( url );
    return 0;
    //     if ( url.isValid() )
    //         return m_codeProxy->codeModel( url );
    //     else
    //         return m_codeProxy->codeModel( KDevCore::documentController() ->activeDocumentUrl() );
}

KDevCodeProxy *CSharpLanguageSupport::codeProxy() const
{
    return 0;
    //     return m_codeProxy;
}

KDevCodeDelegate *CSharpLanguageSupport::codeDelegate() const
{
    return 0;
    //     return m_codeDelegate;
}

KDevCodeRepository *CSharpLanguageSupport::codeRepository() const
{
    return 0;
}

KDevParseJob *CSharpLanguageSupport::createParseJob( const KUrl &url )
{
    return new ParseJob( url, this, m_memoryPool );
}

KDevParseJob *CSharpLanguageSupport::createParseJob( KDevDocument *document,
        KTextEditor::SmartRange *highlight )
{
    return new ParseJob( document, highlight, this, m_memoryPool );
}

QStringList CSharpLanguageSupport::mimeTypes() const
{
    return m_mimetypes;
}

void CSharpLanguageSupport::documentLoaded( KDevDocument* file )
{
    if ( supportsDocument( file ) )
        KDevCore::backgroundParser() ->addDocument( file->url(), file );
}

void CSharpLanguageSupport::documentClosed( KDevDocument* file )
{
    if ( supportsDocument( file ) )
        KDevCore::backgroundParser() ->removeDocumentFile( file );
}

void CSharpLanguageSupport::documentActivated( KDevDocument* file )
{
    Q_UNUSED( file );
}

void CSharpLanguageSupport::projectOpened()
{
    // FIXME This should add the project files to the backgroundparser
}

void CSharpLanguageSupport::projectClosed()
{
    // FIXME This should remove the project files from the backgroundparser
}

#include "csharplanguagesupport.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
