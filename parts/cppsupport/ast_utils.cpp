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
#include <qstringlist.h>

#include <klocale.h>
#include <kdebug.h>
#include <kapplication.h>

#include <ktexteditor/editinterface.h>

AST* findNodeAt( AST* node, int line, int column )
{
    // kdDebug(9007) << "findNodeAt(" << node << ")" << endl;

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
    if( !ast || !editIface || !parent || ast->nodeType() == NodeType_Generic )
	return;
    
    int startLine, startColumn;
    int endLine, endColumn;
    ast->getStartPosition( &startLine, &startColumn );
    ast->getEndPosition( &endLine, &endColumn );
    
    QString nodeDescription = ast->text();
    if( !nodeDescription ){
	nodeDescription = ast->nodeType() == NodeType_TranslationUnit ? QString::fromLatin1("") : editIface->textLine( startLine ).simplifyWhiteSpace();
    }
	
    if( nodeDescription.isEmpty() )
	nodeDescription += i18n( " [%1]" ).arg( nodeTypeToString(ast->nodeType()) );
    
    QListViewItem* item = new QListViewItem( parent, nodeDescription,
					     QString::number(startLine), QString::number(startColumn),
					     QString::number(endLine), QString::number(endColumn) );
					     
    if( ast->nodeType() == NodeType_FunctionDefinition )
	   return;
    
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

void scopeOfNode( AST* ast, QStringList& scope )
{
    if( !ast )
	return;
    
    if( ast->parent() )
	scopeOfNode( ast->parent(), scope );
    
    QString s;
    switch( ast->nodeType() )
    {
    case NodeType_ClassSpecifier:
        if( ((ClassSpecifierAST*)ast)->name() ){
	    s = ((ClassSpecifierAST*)ast)->name()->text();
	    s = s.isEmpty() ? QString::fromLatin1("<unnamed>") : s;
	    scope.push_back( s );
	}
	break;

    case NodeType_Namespace:
	s = ((NamespaceAST*)ast)->namespaceName();
	s = s.isEmpty() ? QString::fromLatin1("<unnamed>") : s;
	scope.push_back( s );
	break;

    case NodeType_FunctionDefinition:
        if( ((FunctionDefinitionAST*)ast)->nestedName() ){
	    s = ((FunctionDefinitionAST*)ast)->nestedName()->text();
	    if( s )
	       scope.push_back( s );
	}
	break;

    default:
	break;
    }
}



