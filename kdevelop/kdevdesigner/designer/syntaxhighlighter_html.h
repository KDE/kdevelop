/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef SYNTAXHIGHLIGHTER_HTML_H
#define SYNTAXHIGHLIGHTER_HTML_H

#include <./private/qrichtext_p.h>

class SyntaxHighlighter_HTML : public QTextPreProcessor
{
public:

    enum HTML {
	Standard = 1,
	Keyword,
	Attribute,
	AttribValue
    };

    SyntaxHighlighter_HTML();
    virtual ~SyntaxHighlighter_HTML();
    void process( QTextDocument *doc, QTextParagraph *string, int start, bool invalidate = TRUE );
    QTextFormat *format( int id );

private:
    void addFormat( int id, QTextFormat *f );

    QTextFormat *lastFormat;
    int lastFormatId;
    QIntDict<QTextFormat> formats;

};

#endif
