/* This file is part of the KDE project
   Copyright (C) 2004 Sascha Cunz <sascha@sacu.de>

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

/* Yet to come file abstraction layer */

#include <kmimetype.h>

class KDevFile
{
public:
	/**
	 * Styles of comment-formats a source file can take
	 */
	enum CommentingStyle
	{
		NoCommenting,	//!< file seems unable to contain comments
		CPPStyle,	//!< C++ Style:		/* comment */
		CStyle,		//!< (old) C Style:	// comment
		AdaStyle,	//!< ADA-Style:		-- comment --
		PascalStyle,	//!< Pascal-Style:	{  comment  }
		BashStyle,	//!< Shell-Style:	#  comment
		XMLStyle	//!< XML-Style:		<!-- comment -->
	};
	
public:
	/**
	 * return a commenting style for a given mime type.
	 */
	static CommentingStyle commentingStyleFromMimeType( const QString& mimeType );
	
	/**
	 * return a commenting style for a given mime type.
	 */
	static CommentingStyle commentingStyleFromMimeType( KMimeType::Ptr mimeType );
};
