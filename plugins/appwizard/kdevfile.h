/***************************************************************************
 *   Copyright (C) 2004-2005 by Sascha Cunz                                *
 *   sascha@kdevelop.org                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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

// kate: indent-width 4; replace-tabs on; tab-width 4; space-indent on;
