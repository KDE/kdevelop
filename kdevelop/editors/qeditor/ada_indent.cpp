/*
 * Copyright (C) 2003 Oliver Kellogg
 * okellogg@users.sourceforge.net
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include "ada_indent.h"
#include "qeditor.h"
#include <kdebug.h>

AdaIndent::AdaIndent( QEditor* ed )
    : QEditorIndenter( ed ),
    rxCompoundStmt("^\\s*(begin|for|declare|while|case|loop|if|else|subtype|type)\\b.*")
{
}

AdaIndent::~AdaIndent()
{
}

int AdaIndent::indentForLine( int line )
{
    if( line == 0 )
        return 0;

    int prevLine = QMAX( 0, previousNonBlankLine( line ) );
    int sw = 3;

    QString lineText = editor()->text( line );
    QString prevLineText = editor()->text( prevLine );

    int lineInd = indentation( lineText );
    int prevLineInd = indentation( prevLineText );

    kdDebug() << "lineText=" << lineText << "  prevLineText=" << prevLineText << " indent prev=" << lineInd << endl;

    if (rxCompoundStmt.exactMatch(prevLineText))
    {
        kdDebug() << "exact match for compound statement match" << endl;
        return prevLineInd + sw;
    }
    else
        return prevLineInd;
}

