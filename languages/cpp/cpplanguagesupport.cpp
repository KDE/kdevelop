/*
 * KDevelop C++ Language Support
 *
 * Copyright (c) 2005 Matt Rogers <mattr@kde.org>
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
    //I don't particular like this long string, but it seems easier
    //this way
    QString types = QLatin1String( "text/x-chdr,text/x-c++hdr,text/x-csrc,text/x-c++src" );
    QStringList typesList = types.split( "," );
    foreach ( QString s, typesList )
    {
        kdDebug(9007) << k_funcinfo << "Attempting to add mimetype: " << s << endl;
        KMimeType::Ptr mimeType = KMimeType::mimeType( s );
        if ( mimeType->is( s ) )
            m_mimetypes.append( mimeType );
    }

}

CppLanguageSupport::~CppLanguageSupport()
{
}

int CppLanguageSupport::features() const
{
    return m_features;
}

KMimeType::List CppLanguageSupport::mimeTypes() const
{
    return m_mimetypes;
}

#include "cpplanguagesupport.moc"

//kate: space-indent on; indent-width 4;
