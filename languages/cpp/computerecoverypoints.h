/***************************************************************************
   begin                : Sat Jul 21 2001
   copyright            : (C) 2001 by Victor R�er
   email                : victor_roeder@gmx.de
   copyright            : (C) 2002,2003 by Roberto Raggi
   email                : roberto@kdevelop.org
   copyright            : (C) 2005 by Adam Treat
   email                : manyoso@yahoo.com
   copyright            : (C) 2006 by David Nolden
   email                : david.nolden.kdevelop@art-master.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __COMPUTERECOVERYPOINTS_H__
#define __COMPUTERECOVERYPOINTS_H__

#include <qvaluestack.h>

#include "simpletype.h"
#include "tree_parser.h"

const QString hardCodedAliases = "std=_GLIBCXX_STD;";

static QString toSimpleName( NameAST* name )
{
  if ( !name )
    return QString::null;
  
  QString s;
  QPtrList<ClassOrNamespaceNameAST> l = name->classOrNamespaceNameList();
  QPtrListIterator<ClassOrNamespaceNameAST> nameIt( l );
  while ( nameIt.current() )
  {
    if ( nameIt.current() ->name() )
    {
      s += nameIt.current() ->name() ->text() + "::";
    }
    ++nameIt;
  }
  
  if ( name->unqualifiedName() && name->unqualifiedName() ->name() )
    s += name->unqualifiedName() ->name() ->text();
  
  return s;
}


struct RecoveryPoint
{
  int kind;
  QStringList scope;
  QValueList<QStringList> imports;
  
  int startLine, startColumn;
  int endLine, endColumn;
  
  RecoveryPoint()
    : kind( 0 ), startLine( 0 ), startColumn( 0 ),
    endLine( 0 ), endColumn( 0 )
  {}
  
    ///Registers the recovery-points imports into the given namespace
  void registerImports( SimpleType ns ) {
    SimpleTypeNamespace* n = dynamic_cast<SimpleTypeNamespace*>( &(*ns) );
    if( !n ) {
      kdDebug( 9007 ) << "the global namespace was not resolved correctly " << endl;
    } else {
            ///put the imports into the global namespace
      for( QValueList<QStringList>::iterator it = imports.begin(); it != imports.end(); ++it ) {
        kdDebug( 9007 ) << "inserting import " << *it << " into the global scole" << endl;
        n->addAliasMap( "", (*it).join("::") );
      }
      n->addAliases( hardCodedAliases );
    }
  }
  
private:
  RecoveryPoint( const RecoveryPoint& source );
  void operator = ( const RecoveryPoint& source );
};


// namespace?
class ComputeRecoveryPoints: public TreeParser
{
public:
 ComputeRecoveryPoints( QPtrList<RecoveryPoint>& points )
   : recoveryPoints( points )
 {}

 virtual void parseTranslationUnit( TranslationUnitAST* ast )
 {
  QValueList<QStringList> dummy;

  m_imports.push( dummy );
  TreeParser::parseTranslationUnit( ast );
  m_imports.pop();

  kdDebug( 9007 ) << "found " << recoveryPoints.count() << " recovery points" << endl;
 }

 virtual void parseUsingDirective( UsingDirectiveAST* ast )
 {
  if ( !ast->name() )
   return ;

  QStringList type = CppCodeCompletion::typeName( ast->name() ->text() ).scope();
  m_imports.top().push_back( type );
 }

 virtual void parseNamespace( NamespaceAST* ast )
 {
  //insertRecoveryPoint( ast );
  m_currentScope.push_back( ast->namespaceName() ->text() );

  m_imports.push( m_imports.top() ); // dup
  m_imports.top().push_back( m_currentScope );

  TreeParser::parseNamespace( ast );

  m_imports.pop();
  m_currentScope.pop_back();
 }

 void parseTemplateDeclaration( TemplateDeclarationAST* ast )
 {
  if ( ast->declaration() )
   parseDeclaration( ast->declaration() );
  
  TreeParser::parseTemplateDeclaration( ast );
 }
 
 
 virtual void parseSimpleDeclaration( SimpleDeclarationAST* ast )
 {
  TypeSpecifierAST * typeSpec = ast->typeSpec();
  //InitDeclaratorListAST* declarators = ast->initDeclaratorList();

  if ( typeSpec )
   parseTypeSpecifier( typeSpec );

  //insertRecoveryPoint( ast );
  TreeParser::parseSimpleDeclaration( ast );
 }

 virtual void parseFunctionDefinition( FunctionDefinitionAST* ast )
 {
  m_imports.push( m_imports.top() ); // dup
  insertRecoveryPoint( ast );
  m_imports.pop();
 }

 virtual void parseClassSpecifier( ClassSpecifierAST* ast )
 {
  insertRecoveryPoint( ast );
  m_currentScope.push_back( toSimpleName( ast->name() ) );
  TreeParser::parseClassSpecifier( ast );
  m_currentScope.pop_back();
 }

 void insertRecoveryPoint( AST* ast )
 {
  if ( !ast )
   return ;

  RecoveryPoint* pt = new RecoveryPoint();
  pt->kind = ast->nodeType();
  pt->scope = m_currentScope;
  ast->getStartPosition( &pt->startLine, &pt->startColumn );
  ast->getEndPosition( &pt->endLine, &pt->endColumn );
  pt->imports = m_imports.top();

  recoveryPoints.append( pt );
 }

private:
 QPtrList<RecoveryPoint>& recoveryPoints;
 QValueStack< QValueList<QStringList> > m_imports;
 QStringList m_currentScope;
};

#endif 
// kate: indent-mode csands; tab-width 4;
