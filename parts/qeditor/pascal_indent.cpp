/***************************************************************************
 *   Copyright (C) 2003 Alexander Dymo                                     *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "pascal_indent.h"
#include "qeditor.h"
#include <kdebug.h>

PascalIndent::PascalIndent( QEditor* ed )
    : QEditorIndenter( ed ),
    rxCompoundStmt("^\\s*(begin|for|try|while|case|repeat|if|else|var|const|type)\\b.*")
{
}

PascalIndent::~PascalIndent()
{
}

int PascalIndent::indentForLine( int line )
{
    if( line == 0 )
        return 0;

    int prevLine = QMAX( 0, previousNonBlankLine( line ) );
    int sw = 4;

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

