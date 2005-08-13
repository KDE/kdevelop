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

#include <qstringlist.h>
#include <qregexp.h>

#include <klocale.h>
#include <kdebug.h>
#include <kapplication.h>

#include <ktexteditor/editinterface.h>

AST* findNodeAt( AST* node, int line, int column )
{
	// kdDebug(9007) << "findNodeAt(" << node << ")" << endl;
	
	if ( !node )
		return 0;
	
	int startLine, startColumn;
	int endLine, endColumn;
	
	node->getStartPosition( &startLine, &startColumn );
	node->getEndPosition( &endLine, &endColumn );
	
	if ( ( line > startLine || ( line == startLine && column >= startColumn ) ) &&
	     ( line < endLine || ( line == endLine && column < endColumn ) ) )
	{
		
		QPtrList<AST> children = node->children();
		QPtrListIterator<AST> it( children );
		while ( it.current() )
		{
			AST * a = it.current();
			++it;
			
			AST* r = findNodeAt( a, line, column );
			if ( r )
				return r;
		}
		
		return node;
	}
	
	return 0;
}

void scopeOfNode( AST* ast, QStringList& scope )
{
	if ( !ast )
		return ;
	
	if ( ast->parent() )
		scopeOfNode( ast->parent(), scope );
	
	QString s;
	switch ( ast->nodeType() )
	{
	case NodeType_ClassSpecifier:
		if ( ( ( ClassSpecifierAST* ) ast ) ->name() )
		{
			s = ( ( ClassSpecifierAST* ) ast ) ->name() ->text();
			s = s.isEmpty() ? QString::fromLatin1( "<unnamed>" ) : s;
			scope.push_back( s );
		}
		break;
		
	case NodeType_Namespace:
		{
			AST* namespaceName = ( ( NamespaceAST* ) ast ) ->namespaceName();
			s = namespaceName ? namespaceName->text() : QString::fromLatin1( "<unnamed>" );
			scope.push_back( s );
		}
		break;
		
	case NodeType_FunctionDefinition:
		{
			FunctionDefinitionAST* funDef = static_cast<FunctionDefinitionAST*>( ast );
			DeclaratorAST* d = funDef->initDeclarator() ->declarator();
			
			// hotfix for bug #68726
			if ( !d->declaratorId() )
				break;
			
			QPtrList<ClassOrNamespaceNameAST> l = d->declaratorId() ->classOrNamespaceNameList();
			QPtrListIterator<ClassOrNamespaceNameAST> nameIt( l );
			while ( nameIt.current() )
			{
				AST * name = nameIt.current() ->name();
				scope.push_back( name->text() );
				
				++nameIt;
			}
		}
		break;
		
	default:
		break;
	}
}


QString typeSpecToString( TypeSpecifierAST* typeSpec )   /// @todo remove
{
	if ( !typeSpec )
		return QString::null;
	
	return typeSpec->text().replace( QRegExp( " :: " ), "::" );
}

QString declaratorToString( DeclaratorAST* declarator, const QString& scope, bool skipPtrOp )
{
	if ( !declarator )
		return QString::null;
	
	QString text;
	
	if ( !skipPtrOp )
	{
		QPtrList<AST> ptrOpList = declarator->ptrOpList();
		for ( QPtrListIterator<AST> it( ptrOpList ); it.current(); ++it )
		{
			text += it.current() ->text();
		}
		text += " ";
	}
	
	text += scope;
	
	if ( declarator->subDeclarator() )
		text += QString::fromLatin1( "(" ) + declaratorToString( declarator->subDeclarator() ) + QString::fromLatin1( ")" );
	
	if ( declarator->declaratorId() )
		text += declarator->declaratorId() ->text();
	
	QPtrList<AST> arrays = declarator->arrayDimensionList();
	QPtrListIterator<AST> it( arrays );
	while ( it.current() )
	{
		text += "[]";
		++it;
	}
	
	if ( declarator->parameterDeclarationClause() )
	{
		text += "( ";
		
		ParameterDeclarationListAST* l = declarator->parameterDeclarationClause() ->parameterDeclarationList();
		if ( l != 0 )
		{
			QPtrList<ParameterDeclarationAST> params = l->parameterList();
			QPtrListIterator<ParameterDeclarationAST> it( params );
			
			while ( it.current() )
			{
				QString type = typeSpecToString( it.current() ->typeSpec() );
				text += type;
				if ( !type.isEmpty() )
					text += " ";
				text += declaratorToString( it.current() ->declarator() );
				
				++it;
				
				if ( it.current() )
					text += ", ";
			}
		}
		
		text += " )";
		
		if ( declarator->constant() != 0 )
			text += " const";
	}

	return text.replace( QRegExp( " :: " ), "::" ).simplifyWhiteSpace();
}
//kate: indent-mode csands; tab-width 4; space-indent off;
