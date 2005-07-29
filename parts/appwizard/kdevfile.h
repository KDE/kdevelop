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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

/**
@file kdevfile.h
File abstraction layer.
*/
#ifndef KDEVFILE_H
#define KDEVFILE_H

#include <kmimetype.h>

/**
File abstraction.
*/
class KDevFile
{
public:
    /**Styles of comment-formats a source file can have.*/
    enum CommentingStyle
    {
        NoCommenting,   //!< file seems to be unable to contain comments
        CPPStyle,       //!< C Style: <pre>/* comment */</pre>
        CStyle,         //!< C++ Style: <pre>// comment</pre>
        AdaStyle,       //!< Ada Style: <pre>-- comment --</pre>
        PascalStyle,    //!< Pascal Style: <pre>{  comment  }</pre>
        BashStyle,      //!< Shell Style: <pre>#  comment</pre>
        XMLStyle        //!< XML Style: <pre><!-- comment --></pre>
    };

public:
    /**@return A commenting style for a given mime type.
    @param mimeType A string which defines a mime type (like "text/x-c+++src").*/
    static CommentingStyle commentingStyleFromMimeType(const QString& mimeType);
    
    /**@return A commenting style for a given mime type.
    @param mimeType The mime type.*/
    static CommentingStyle commentingStyleFromMimeType( KMimeType::Ptr mimeType );
};

#endif
