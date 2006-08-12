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

#include "javalanguagesupport.h"

// #include "codeproxy.h"
// #include "codedelegate.h"

#include <kdebug.h>

typedef KGenericFactory<JavaLanguageSupport> KDevJavaSupportFactory;
K_EXPORT_COMPONENT_FACTORY( kdevjavalanguagesupport,
                            KDevJavaSupportFactory( "kdevjavasupport" ) )

JavaLanguageSupport::JavaLanguageSupport( QObject* parent,
        const QStringList& /*args*/ )
        : KDevLanguageSupport( KDevJavaSupportFactory::instance(), parent )
{
    QString types =
        QLatin1String( "text/x-java" );
    m_mimetypes = types.split( "," );

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

JavaLanguageSupport::~JavaLanguageSupport()
{
}

KDevCodeModel *JavaLanguageSupport::codeModel( const KUrl &url ) const
{
    Q_UNUSED( url );
    return 0;
    //     if ( url.isValid() )
    //         return m_codeProxy->codeModel( url );
    //     else
    //         return m_codeProxy->codeModel( KDevCore::documentController() ->activeDocumentUrl() );
}

KDevCodeProxy *JavaLanguageSupport::codeProxy() const
{
    return 0;
    //     return m_codeProxy;
}

KDevCodeDelegate *JavaLanguageSupport::codeDelegate() const
{
    return 0;
    //     return m_codeDelegate;
}

KDevCodeRepository *JavaLanguageSupport::codeRepository() const
{
    return 0;
}

KDevParseJob *JavaLanguageSupport::createParseJob( const KUrl &url )
{
    return new ParseJob( url, this );
}

KDevParseJob *JavaLanguageSupport::createParseJob( KDevDocument *document )
{
    return new ParseJob( document, this );
}

QStringList JavaLanguageSupport::mimeTypes() const
{
    return m_mimetypes;
}

void JavaLanguageSupport::documentLoaded( KDevDocument* file )
{
    if ( supportsDocument( file ) )
        KDevCore::backgroundParser() ->addDocument( file->url(), file );
}

void JavaLanguageSupport::documentClosed( KDevDocument* file )
{
    if ( supportsDocument( file ) )
        KDevCore::backgroundParser() ->removeDocumentFile( file );
}

void JavaLanguageSupport::documentActivated( KDevDocument* file )
{
    Q_UNUSED( file );
}

void JavaLanguageSupport::projectOpened()
{
    KUrl::List documentList;
    QList<KDevProjectFileItem*> files = KDevCore::activeProject()->allFiles();
    foreach ( KDevProjectFileItem * file, files )
    {
        if ( supportsDocument( file->url() ) /*&& file->url().fileName().endsWith( ".java" )*/ )
        {
            documentList.append( file->url() );
        }
    }
    KDevCore::backgroundParser() ->addDocumentList( documentList );
}

void JavaLanguageSupport::projectClosed()
{
    // FIXME This should remove the project files from the backgroundparser
}

#include "javalanguagesupport.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
