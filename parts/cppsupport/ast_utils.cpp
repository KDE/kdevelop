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
#include <qlistview.h>
#include <klocale.h>
#include <kdebug.h>
#include <ktexteditor/editinterface.h>

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

void buildView( AST* ast, KTextEditor::EditInterface* editIface, QListViewItem* parent )
{
    if( !ast || !editIface || !parent )
	return;
    
    int startLine, startColumn;
    int endLine, endColumn;
    ast->getStartPosition( &startLine, &startColumn );
    ast->getEndPosition( &endLine, &endColumn );
    
    QString nodeDescription = ast->nodeType() == NodeType_TranslationUnit ? QString::fromLatin1("TranslationUnit") : editIface->textLine( startLine );
    QListViewItem* item = new QListViewItem( parent, nodeDescription.simplifyWhiteSpace(),
					     QString::number(startLine), QString::number(startColumn),
					     QString::number(endLine), QString::number(endColumn) );
    
    QPtrList<AST> children = ast->children();
    if( children.count() ){
	item->setExpandable( true );
	item->setOpen( true );
    } else
	return;
    
    QPtrListIterator<AST> it( children );
    while( it.current() ){
	buildView( it.current(), editIface, item );
	++it;
    }
}

