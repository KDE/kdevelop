/* This file is part of the KDE project
   Copyright (C) 2004 Sascha Cunz <mail@sacu.de>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "kdevfile.h"

static struct MimeTypeMapping
{
	const char* szMimeType;
	KDevFile::CommentingStyle commentStyle;
} mimeTypeMapping[] = {
{	"text/x-c++hdr",            KDevFile::CPPStyle           },
{	"text/x-c++src",            KDevFile::CPPStyle           },
{	"text/x-chdr",              KDevFile::CStyle             },
{	"text/x-csrc",              KDevFile::CStyle             },
{	"text/adasrc",              KDevFile::AdaStyle           },
{	"text/x-pascal",            KDevFile::PascalStyle        },
{	"text/x-perl",              KDevFile::BashStyle          },
{	"text/xml",                 KDevFile::XMLStyle           },
{	"text/x-xslt",              KDevFile::XMLStyle           },
{	"text/x-xslfo",             KDevFile::XMLStyle           },
{	"text/html",                KDevFile::XMLStyle           },
{	"text/docbook",             KDevFile::XMLStyle           },
{	"application/x-php",        KDevFile::CPPStyle           },
{	"application/x-shellscript",KDevFile::BashStyle          },
{	NULL,                       KDevFile::NoCommenting       }
};

KDevFile::CommentingStyle KDevFile::commentingStyleFromMimeType( const QString& mimeType )
{
	int idx = 0;
	while( mimeTypeMapping[idx].szMimeType )
	{
		if( mimeType == mimeTypeMapping[idx].szMimeType )
			return mimeTypeMapping[idx].commentStyle;
		idx++;
	}
	return KDevFile::NoCommenting;
}

KDevFile::CommentingStyle KDevFile::commentingStyleFromMimeType( KMimeType::Ptr mimeType )
{
	return commentingStyleFromMimeType( mimeType->name() );
}

