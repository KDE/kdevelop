/*
 *  Copyright (C) 2002 Roberto Raggi (roberto@kdevelop.org)
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 */

#include "python_indent.h"
#include "qeditor.h"
#include <kdebug.h>

PythonIndent::PythonIndent( QEditor* ed )
    : QEditorIndenter( ed ),
    rxLineEndedWithAColon( "^[^#]*:\\s*(#.*)?$" ),
    rxStopExecutionStmt( "^\\s*(break|continue|raise|return|pass)\\b.*" ),
    rxHeaderKeyword( "^\\s*(elif|else|except|finaly)\\b.*" ),
    rxOneLinerStmt( "^\\s*(for|if|try)\\b.*" )
{
}

PythonIndent::~PythonIndent()
{
}

int PythonIndent::indentForLine( int line )
{
    if( line == 0 )
	return 0;
    
    int prevLine = QMAX( 0, previousNonBlankLine( line ) );
    int sw = 4;
    
    QString lineText = editor()->text( line );
    QString prevLineText = editor()->text( prevLine );
    
    int lineInd = indentation( lineText );
    int prevLineInd = indentation( prevLineText );
    
    if( rxLineEndedWithAColon.exactMatch(prevLineText) ){
	return prevLineInd + sw;
    
    } else if( rxStopExecutionStmt.exactMatch(prevLineText) ){
	return prevLineInd - sw;
	
    } else if( rxHeaderKeyword.exactMatch(lineText) ){
	
	if( rxOneLinerStmt.exactMatch(prevLineText) )
	    return prevLineInd;
	
	return prevLineInd - sw;
	
    } else    
	return prevLineInd;
}

