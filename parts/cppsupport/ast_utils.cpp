/***************************************************************************
 *   Copyright (C) 2002 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ast_utils.h"
#include "ast.h"
#include <kdebug.h>

AST* findNodeAt( AST* node, int line, int column )
{
    kdDebug(9007) << "findNodeAt(" << node << ")" << endl;

    if( !node )
	return 0;

    int startLine, startColumn;
    int endLine, endColumn;

    node->getStartPosition( &startLine, &startColumn );
    node->getEndPosition( &endLine, &endColumn );

    if( (line > startLine || (line == startLine && column >= startColumn)) &&
        (line < endLine || (line == endLine && column <= endColumn)) ){

        QPtrList<AST> children = node->children();
	QPtrListIterator<AST> it( children );
	while( it.current() ){
	    AST* a = it.current();
	    ++it;

	    AST* r = findNodeAt( a, line, column );
	    if( r )
		return r;
	}

	return node;
    }

    return 0;
}
