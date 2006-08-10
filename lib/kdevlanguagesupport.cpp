/* This file is part of the KDE project
   Copyright (C) 2001 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2001-2002 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (C) 2002-2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2002 F@lk Brettschneider <falkbr@kdevelop.org>
   Copyright (C) 2003 Alexander Dymo <adymo@kdevelop.org>
   Copyright (C) 2003 Amilcar do Carmo Lucas <amilcar@ida.ing.tu-bs.de>

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
#include "kdevlanguagesupport.h"

#include <kdevast.h>
#include <kdevdocument.h>

KDevLanguageSupport::KDevLanguageSupport(KInstance *instance,
                                         QObject *parent)
    : KDevPlugin(instance, parent)
{
}

KDevLanguageSupport::~KDevLanguageSupport()
{
}

bool KDevLanguageSupport::supportsDocument( KDevDocument* file )
{
    KMimeType::Ptr mimetype = file->mimeType();
    QStringList mimeList = mimeTypes();
    QStringList::const_iterator it = mimeList.begin();
    for ( ; it != mimeList.end(); it++ )
        if ( mimetype->is( *it ) )
            return true;
    return false;
}

bool KDevLanguageSupport::supportsDocument( const KUrl &url )
{
    KMimeType::Ptr mimetype = KMimeType::findByURL( url );
    QStringList mimeList = mimeTypes();
    QStringList::const_iterator it = mimeList.begin();
    for ( ; it != mimeList.end(); it++ )
        if ( mimetype->is( *it ) )
            return true;
    return false;
}

void KDevLanguageSupport::read( KDevAST * ast, std::ifstream &in )
{
    //Do Nothing
}

void KDevLanguageSupport::write( KDevAST * ast, std::ofstream &out )
{
    //Do Nothing
}

#include "kdevlanguagesupport.moc"
