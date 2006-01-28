/*
 * KDevelop C++ Language Support
 *
 * Copyright (c) 2005 Matt Rogers <mattr@kde.org>
 * Copyright (c) 2005 Adam Treat <treat@kde.org>
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

CppLanguageSupport::CppLanguageSupport( QObject* parent,
                                        const char* name,
                                        const QStringList& /*args*/ )
    : KDevLanguageSupport( CppLanguageSupportFactory::info(), parent )
{
    setObjectName( name );
    m_features = KDevLanguageSupport::Classes |
                 KDevLanguageSupport::Structs |
                 KDevLanguageSupport::Functions |
                 KDevLanguageSupport::Variables |
                 KDevLanguageSupport::Namespaces |
                 KDevLanguageSupport::Signals |
                 KDevLanguageSupport::Slots |
                 KDevLanguageSupport::Declarations;

    QString types = QLatin1String( "text/x-chdr,text/x-c++hdr,text/x-csrc,text/x-c++src" );
    m_mimetypes = types.split( "," );

    m_backgroundParser = new BackgroundParser( this );

    connect( documentController(), SIGNAL( documentLoaded( const KUrl & ) ),
             this, SLOT( documentLoaded( const KUrl & ) ) );
    connect( documentController(), SIGNAL( documentClosed( const KUrl & ) ),
             this, SLOT( documentClosed( const KUrl & ) ) );
}

CppLanguageSupport::~CppLanguageSupport()
{
}

int CppLanguageSupport::features() const
{
    return m_features;
}

QStringList CppLanguageSupport::mimeTypes() const
{
    return m_mimetypes;
}

void CppLanguageSupport::documentLoaded( const KUrl &url )
{
    if ( isCppLanguageDocument( url ) )
        m_backgroundParser->addDocument( url );
}

void CppLanguageSupport::documentClosed( const KUrl &url )
{
    if ( isCppLanguageDocument( url ) )
        m_backgroundParser->removeDocument( url );
}

bool CppLanguageSupport::isCppLanguageDocument( const KUrl &url )
{
    KMimeType::Ptr mimetype = KMimeType::findByURL( url );
    foreach ( QString mime, m_mimetypes )
        if ( mimetype->is( mime ) )
            return true;
    return false;
}

#include "cpplanguagesupport.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
