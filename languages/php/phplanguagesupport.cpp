/*
 * KDevelop PHP Language Support
 *
 * Copyright (c) 2005 Escuder Nicolas <n.escuder@intra-links.com>
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

#include "phplanguagesupport.h"
#include "phpsupportfactory.h"

PHPLanguageSupport::PHPLanguageSupport(QObject *parent, const char *name, const QStringList &)
  : KDevLanguageSupport(PHPSupportFactory::info(), parent)
{
   setObjectName(QString::fromUtf8(name));

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

   connect( documentController(), SIGNAL( documentLoaded( const KUrl & ) ),
            this, SLOT( documentLoaded( const KUrl & ) ) );
   connect( documentController(), SIGNAL( documentClosed( const KUrl & ) ),
            this, SLOT( documentClosed( const KUrl & ) ) );
}

PHPLanguageSupport::~PHPLanguageSupport()
{
}

int PHPLanguageSupport::features() const
{
  return m_features;
}

QStringList PHPLanguageSupport::mimeTypes() const
{
  return m_mimetypes;
}

QString PHPLanguageSupport::formatTag(const Tag& tag) const
{
  return QString();
}

QString PHPLanguageSupport::formatModelItem(const KDevCodeItem *item, bool shortDescription) const
{
  return QString();
}

QString PHPLanguageSupport::formatClassName(const QString &name) const
{
  return name;
}

QString PHPLanguageSupport::unformatClassName(const QString &name) const
{
  return name;
}

void PHPLanguageSupport::documentLoaded( const KUrl &url )
{
/*
    if ( isPHPLanguageDocument( url ) )
        m_backgroundParser->addDocument( url );
*/
}

void PHPLanguageSupport::documentClosed( const KUrl &url )
{
/*
    if ( isPHPLanguageDocument( url ) )
        m_backgroundParser->removeDocument( url );
*/
}

bool PHPLanguageSupport::isPHPLanguageDocument( const KUrl &url )
{
    KMimeType::Ptr mimetype = KMimeType::findByURL( url );
    foreach ( QString mime, m_mimetypes )
        if ( mimetype->is( mime ) )
            return true;
    return false;
}

#include "phplanguagesupport.moc"
