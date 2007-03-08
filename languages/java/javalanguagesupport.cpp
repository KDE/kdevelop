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

#include "javalanguagesupport.h"

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
*/

#include "parsejob.h"
// #include "codeproxy.h"
// #include "codedelegate.h"

using namespace java;

typedef KGenericFactory<JavaLanguageSupport> KDevJavaSupportFactory;
K_EXPORT_COMPONENT_FACTORY( kdevjavalanguagesupport, KDevJavaSupportFactory( "kdevjavasupport" ) )

KDEV_USE_EXTENSION_INTERFACE_NS( KDevelop, ILanguageSupport, JavaLanguageSupport )

JavaLanguageSupport::JavaLanguageSupport( QObject* parent,
                                          const QStringList& /*args*/ )
        : KDevelop::IPlugin( KDevJavaSupportFactory::componentData(), parent )
        , KDevelop::ILanguageSupport()
{
    /*
    QString types =
        QLatin1String( "text/x-java" );
    m_mimetypes = types.split( "," );

    //     m_codeProxy = new CodeProxy( this );
    //     m_codeDelegate = new CodeDelegate( this );
    //     m_backgroundParser = new BackgroundParser( this );
    //     m_highlights = new CppHighlighting( this );

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

JavaLanguageSupport::~JavaLanguageSupport()
{
}

/*
KDevelop::CodeModel *JavaLanguageSupport::codeModel( const KUrl &url ) const
{
    Q_UNUSED( url );
    return 0;
    //     if ( url.isValid() )
    //         return m_codeProxy->codeModel( url );
    //     else
    //         return m_codeProxy->codeModel( KDevelop::Core::documentController() ->activeDocumentUrl() );
}

KDevelop::CodeProxy *JavaLanguageSupport::codeProxy() const
{
    return 0;
    //     return m_codeProxy;
}

KDevelop::CodeDelegate *JavaLanguageSupport::codeDelegate() const
{
    return 0;
    //     return m_codeDelegate;
}

KDevelop::CodeRepository *JavaLanguageSupport::codeRepository() const
{
    return 0;
}

KDevelop::ParseJob *JavaLanguageSupport::createParseJob( const KUrl &url )
{
    return new ParseJob( url, this );
}

KDevelop::ParseJob *JavaLanguageSupport::createParseJob( KDevelop::Document *document )
{
    return new ParseJob( document, this );
}

QStringList JavaLanguageSupport::mimeTypes() const
{
    return m_mimetypes;
}

void JavaLanguageSupport::documentLoaded( KDevelop::Document *document )
{
    if ( supportsDocument( document ) )
        KDevelop::Core::backgroundParser() ->addDocument( document );
}

void JavaLanguageSupport::documentClosed( KDevelop::Document *document )
{
    if ( supportsDocument( document ) )
        KDevelop::Core::backgroundParser() ->removeDocument( document );
}

void JavaLanguageSupport::documentActivated( KDevelop::Document *document )
{
    Q_UNUSED( document );
}

void JavaLanguageSupport::projectOpened()
{
    KUrl::List documentList;
    QList<KDevelop::ProjectFileItem*> files = KDevelop::Core::activeProject()->allFiles();
    foreach ( KDevelop::ProjectFileItem * file, files )
    {
        if ( supportsDocument( file->url() ) /*&& file->url().fileName().endsWith( ".java" )* / )
        {
            documentList.append( file->url() );
        }
    }
    KDevelop::Core::backgroundParser() ->addDocumentList( documentList );
}

void JavaLanguageSupport::projectClosed()
{
    // FIXME This should remove the project files from the backgroundparser
}
*/

QString JavaLanguageSupport::name() const
{
    return "Java";
}

QStringList JavaLanguageSupport::extensions() const
{
    return QStringList() << "ILanguageSupport";
}

#include "javalanguagesupport.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
