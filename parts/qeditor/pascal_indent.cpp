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
#include "paragdata.h"
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
    const int sw = 4;

    QString lineText = editor()->text( line );
    QString prevLineText = editor()->text( prevLine );

    int lineInd = indentation( lineText );
    int prevLineInd = indentation( prevLineText );

    int extraInd = 0;
    
    ParagData* data = (ParagData*) editor()->document()->paragAt( prevLine )->extraData();
    if( data ){
	QValueList<Symbol> symbolList = data->symbolList();
	QValueList<Symbol>::Iterator it = symbolList.begin();
	while( it != symbolList.end() ){
	    const Symbol& sym = *it;
	    ++it;
	    
	    if ( sym.type() == Symbol::Left )
		extraInd += 4;
	    else if( sym.type() == Symbol::Right )
		extraInd -= 4;
	}
    }
    
    kdDebug() << "lineText=" << lineText << "  prevLineText=" << prevLineText << " indent prev=" << lineInd << endl;
    kdDebug() << "extraInd is " << extraInd << endl;

    if (rxCompoundStmt.exactMatch(prevLineText))
    {
        kdDebug() << "exact match for compound statement match" << endl;
        return QMAX( prevLineInd + sw + extraInd, 0 );
    }
    else
        return QMAX( prevLineInd + extraInd, 0 );
}

