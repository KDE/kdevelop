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

#include <kdebug.h>
#include <kinstance.h>
#include <kstandarddirs.h>

#include <kdevdocumentcontroller.h>

#include "backgroundparser.h"
#include "cppsupportfactory.h"
#include "cpplanguagesupport.h"
#include "cpphighlighting.h"

#include "parser/codemodel.h"
#include "codeproxy.h"
#include "codedelegate.h"

#include <kdebug.h>

CppLanguageSupport::CppLanguageSupport( QObject* parent,
                                        const QStringList& /*args*/ )
    : KDevLanguageSupport( CppLanguageSupportFactory::info(), parent )
{
    QString types =
        QLatin1String( "text/x-chdr,text/x-c++hdr,text/x-csrc,text/x-c++src" );
    m_mimetypes = types.split( "," );

    m_codeProxy = new CodeProxy( this );
    m_codeDelegate = new CodeDelegate( this );
    m_backgroundParser = new BackgroundParser( this );
    m_highlights = new CppHighlighting( this );

    connect( documentController(), SIGNAL( documentLoaded(KDevDocument*) ),
             this, SLOT( documentLoaded(KDevDocument*) ) );
    connect( documentController(), SIGNAL( documentClosed(KDevDocument*) ),
             this, SLOT( documentClosed(KDevDocument*) ) );
    connect( documentController(), SIGNAL( documentActivated(KDevDocument*) ),
             this, SLOT( documentActivated(KDevDocument*) ) );
}

CppLanguageSupport::~CppLanguageSupport()
{
}

KDevCodeModel *CppLanguageSupport::codeModel( const KUrl &url ) const
{
    if ( url.isValid() )
        return m_codeProxy->codeModel( url );
    else
        return m_codeProxy->codeModel( documentController() ->activeDocumentUrl() );
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

QStringList CppLanguageSupport::mimeTypes() const
{
    return m_mimetypes;
}

void CppLanguageSupport::documentLoaded( KDevDocument* file )
{
    if ( supportsDocument( file ) )
        m_backgroundParser->addDocument( file->url(), file );
}

void CppLanguageSupport::documentClosed( KDevDocument* file )
{
    if ( supportsDocument( file ) )
        m_backgroundParser->removeDocumentFile( file );
}

void CppLanguageSupport::documentActivated( KDevDocument* file )
{
    Q_UNUSED( file );
}

CppHighlighting * CppLanguageSupport::codeHighlighting( ) const
{
    return m_highlights;
}

#include "cpplanguagesupport.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
