/***************************************************************************
 *   Copyright (C) 2003 by Roberto Raggi                                   *
 *   roberto@kdevelop.org                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "store_walker.h"
#include <classstore.h>
#include <parsedscopecontainer.h>
#include <kdebug.h>
#include <qfileinfo.h>

StoreWalker::StoreWalker( const QString& fileName, ClassStore* store )
    : m_fileName( fileName ), m_store( store ), m_currentScopeContainer( 0 ), m_currentClass( 0 )
{
}

StoreWalker::~StoreWalker()
{
}

void StoreWalker::parseTranslationUnit( TranslationUnitAST* ast )
{
    m_currentScope.clear();    
    m_currentScopeContainer = m_store->globalScope();
    m_currentClass = 0;
    m_currentAccess = PIE_PUBLIC;
    m_inSlots = false;
    m_inSignals = false;

    m_store->removeWithReferences( m_fileName );
    TreeParser::parseTranslationUnit( ast );
}

void StoreWalker::parseDeclaration( DeclarationAST* ast )
{
    TreeParser::parseDeclaration( ast );
}

void StoreWalker::parseLinkageSpecification( LinkageSpecificationAST* ast )
{
    TreeParser::parseLinkageSpecification( ast );
}

void StoreWalker::parseNamespace( NamespaceAST* ast )
{
    QString nsName;
    if( !ast->namespaceName() ){
	QFileInfo fileInfo( m_fileName );
	QString shortFileName = fileInfo.baseName();
	
	nsName = QString::fromLatin1("(") + shortFileName + QString::fromLatin1(")");
    } else {
	nsName = ast->namespaceName()->text();
    }

    int startLine, startColumn;
    int endLine, endColumn;
    ast->getStartPosition( &startLine, &startColumn );
    ast->getEndPosition( &endLine, &endColumn );
    
    ParsedScopeContainer* ns = findOrInsertScopeContainer( m_currentScopeContainer, nsName );
    ns->setDeclaredOnLine( startLine );
    ns->setDeclaredInFile( m_fileName );
    ns->setDeclarationEndsOnLine( endLine );
    
    ParsedScopeContainer* old_scope = m_currentScopeContainer;
    m_currentScopeContainer = ns;
    m_currentScope.push_back( nsName );
    
    TreeParser::parseNamespace( ast );
    
    m_currentScope.pop_back();
    m_currentScopeContainer = old_scope;
}

void StoreWalker::parseNamespaceAlias( NamespaceAliasAST* ast )
{
    TreeParser::parseNamespaceAlias( ast );
}

void StoreWalker::parseUsing( UsingAST* ast )
{
    TreeParser::parseUsing( ast );
}

void StoreWalker::parseUsingDirective( UsingDirectiveAST* ast )
{
    TreeParser::parseUsingDirective( ast );
}

void StoreWalker::parseTypedef( TypedefAST* ast )
{
    TreeParser::parseTypedef( ast );
}

void StoreWalker::parseTemplateDeclaration( TemplateDeclarationAST* ast )
{
    if( ast->declaration() )
	parseDeclaration( ast->declaration() );
    
    TreeParser::parseTemplateDeclaration( ast );
}

void StoreWalker::parseSimpleDeclaration( SimpleDeclarationAST* ast )
{
    TypeSpecifierAST* typeSpec = ast->typeSpec();
    InitDeclaratorListAST* declarators = ast->initDeclaratorList();

    if( typeSpec )
	parseTypeSpecifier( typeSpec );

    if( declarators ){
	QPtrList<InitDeclaratorAST> l = declarators->initDeclaratorList();

	QPtrListIterator<InitDeclaratorAST> it( l );
	while( it.current() ){
	    parseDeclaration(  ast->functionSpecifier(), ast->storageSpecifier(), typeSpec, it.current() );
	    ++it;
	}
    }

    TreeParser::parseSimpleDeclaration( ast );
}

void StoreWalker::parseFunctionDefinition( FunctionDefinitionAST* ast )
{
    TreeParser::parseFunctionDefinition( ast );
}

void StoreWalker::parseLinkageBody( LinkageBodyAST* ast )
{
    TreeParser::parseLinkageBody( ast );
}

void StoreWalker::parseTypeSpecifier( TypeSpecifierAST* ast )
{
    TreeParser::parseTypeSpecifier( ast );
}

void StoreWalker::parseClassSpecifier( ClassSpecifierAST* ast )
{
    int startLine, startColumn;
    int endLine, endColumn;
    ast->getStartPosition( &startLine, &startColumn );
    ast->getEndPosition( &endLine, &endColumn );
    
    
    PIAccess oldAccess = m_currentAccess;
    bool oldInSlots = m_inSlots;
    bool oldInSignals = m_inSignals;
 
    QString kind = ast->classKey()->text();
    if( kind == "class" )
	m_currentAccess = PIE_PRIVATE;
    else 
	m_currentAccess = PIE_PUBLIC;
    m_inSlots = false;
    m_inSignals = false;
    
    QString className;
    if( !ast->name() ){
	QFileInfo fileInfo( m_fileName );
	QString shortFileName = fileInfo.baseName();
	
	className = QString::fromLatin1("(") + shortFileName + QString::fromLatin1(")");
    } else {
	className = ast->name()->text();
    }
    
    ParsedClass* klass = new ParsedClass();
    klass->setDeclaredOnLine( startLine );
    klass->setDeclaredInFile( m_fileName );
    klass->setName( className );
    klass->setDeclaredInScope( m_currentScope.join(".") );
    
    bool inStore = m_store->hasClass( klass->path() );
    if( inStore ){
	ParsedClass* parsedClassRef = m_store->getClassByName( klass->path() );
	parsedClassRef->setDeclaredOnLine( klass->declaredOnLine() );
	parsedClassRef->setDeclaredInFile( klass->declaredInFile() );
	parsedClassRef->setDeclaredInScope( klass->declaredInScope() );
	delete klass;
	klass = parsedClassRef;	
    }
    
    if( m_currentClass )
	m_currentClass->addClass( klass );
    else
	m_currentScopeContainer->addClass( klass );
    
    if( !inStore )
	m_store->addClass( klass );
     
    m_currentScope.push_back( className );
 
    ParsedClass* oldClass = m_currentClass;
    m_currentClass = klass;
    TreeParser::parseClassSpecifier( ast );
    m_currentClass = oldClass;
    
    m_currentScope.pop_back();
    
    m_currentAccess = oldAccess;
    m_inSlots = oldInSlots;
    m_inSignals = oldInSignals;
}

void StoreWalker::parseEnumSpecifier( EnumSpecifierAST* ast )
{
    QPtrList<EnumeratorAST> l = ast->enumeratorList();
    QPtrListIterator<EnumeratorAST> it( l );
    while( it.current() ){
	ParsedClassContainer* c = m_currentClass ? (ParsedClassContainer*) m_currentClass : (ParsedClassContainer*) m_currentScopeContainer;
	ParsedAttribute* attr = findOrInsertAttribute( c, it.current()->id()->text() );
	
	int startLine, startColumn;
	int endLine, endColumn;
	ast->getStartPosition( &startLine, &startColumn );
	ast->getEndPosition( &endLine, &endColumn );
    
	attr->setDeclaredOnLine( startLine );
	attr->setDefinedOnLine( startLine );
	attr->setDeclaredInFile( m_fileName );
	attr->setDefinedInFile( m_fileName );
	attr->setDeclarationEndsOnLine( endLine );
	
	++it;
    }
    
    TreeParser::parseEnumSpecifier( ast );
}

void StoreWalker::parseElaboratedTypeSpecifier( ElaboratedTypeSpecifierAST* ast )
{
    TreeParser::parseElaboratedTypeSpecifier( ast );
}

void StoreWalker::parseTypeDeclaratation( TypeSpecifierAST* typeSpec )
{
    parseTypeSpecifier( typeSpec );
}

void StoreWalker::parseDeclaration( GroupAST* funSpec, GroupAST* storageSpec, TypeSpecifierAST* typeSpec, InitDeclaratorAST* decl )
{
    DeclaratorAST* d = decl->declarator();

    if( !d )
	return;

    if( !d->subDeclarator() && d->parameterDeclarationClause() )
	return parseFunctionDeclaration( funSpec, storageSpec, typeSpec, decl );

    QString id;
    
    DeclaratorAST* t = d;
    while( t && t->subDeclarator() )
	t = t->subDeclarator();
    id = t->declaratorId()->text();
    
    ParsedAttribute* attr = findOrInsertAttribute( (m_currentClass ?
						    (ParsedClassContainer*) m_currentClass :
						    (ParsedClassContainer*) m_currentScopeContainer), id );
    int startLine, startColumn;
    int endLine, endColumn;
    decl->getStartPosition( &startLine, &startColumn );
    decl->getEndPosition( &endLine, &endColumn );

    attr->setDeclaredOnLine( startLine );
    attr->setDeclaredInFile( m_fileName );
    attr->setDeclarationEndsOnLine( endLine );
}

void StoreWalker::parseAccessDeclaration( AccessDeclarationAST * access )
{
    QPtrList<AST> l = access->accessList();

    QString accessStr = l.at( 0 )->text();
    if( accessStr == "public" )
	m_currentAccess = PIE_PUBLIC;
    else if( accessStr == "protected" )
	m_currentAccess = PIE_PROTECTED;
    else if( accessStr == "private" )
	m_currentAccess = PIE_PRIVATE;
    else if( accessStr == "signals" )
	m_currentAccess = PIE_PROTECTED;
    else
	m_currentAccess = PIE_PUBLIC;

    m_inSlots = l.count() > 1 ? l.at( 1 )->text() == "slots" : false;
    m_inSignals = l.count() > 1 ? l.at( 0 )->text() == "signals" : false;
}

ParsedScopeContainer * StoreWalker::findOrInsertScopeContainer( ParsedScopeContainer* scope, const QString & name )
{
    ParsedScopeContainer* ns = m_currentScopeContainer->getScopeByName( name );
    if( !ns ){
	ns = new ParsedScopeContainer();
	ns->setName( name );

	scope->addScope( ns );
	m_store->addScope( ns );
    }

    return ns;
}

ParsedAttribute * StoreWalker::findOrInsertAttribute( ParsedClassContainer * scope, const QString & name )
{
    ParsedAttribute* attr = m_currentScopeContainer->getAttributeByName( name );
    if( !attr ){
	attr = new ParsedAttribute();
	attr->setName( name );

	scope->addAttribute( attr );
    }

    return attr;
}

void StoreWalker::parseFunctionDeclaration(  GroupAST* funSpec, GroupAST* storageSpec, TypeSpecifierAST * typeSpec, InitDeclaratorAST * decl )
{
    bool isFriend = false;
    bool isVirtual = false;
    bool isStatic = false;
    bool isInline = false;
    bool isPure = decl->initializer() != 0;

    if( funSpec ){
	QPtrList<AST> l = funSpec->nodeList();
	QPtrListIterator<AST> it( l );
	while( it.current() ){
	    QString text = it.current()->text();
	    if( text == "virtual" ) isVirtual = true;
	    else if( text == "inline" ) isInline = true;
	    ++it;
	}
    }

    if( storageSpec ){
	QPtrList<AST> l = storageSpec->nodeList();
	QPtrListIterator<AST> it( l );
	while( it.current() ){
	    QString text = it.current()->text();
	    if( text == "friend" ) isFriend = true;
	    else if( text == "static" ) isStatic = true;
	    ++it;
	}
    }

    int startLine, startColumn;
    int endLine, endColumn;
    decl->getStartPosition( &startLine, &startColumn );
    decl->getEndPosition( &endLine, &endColumn );

    DeclaratorAST* d = decl->declarator();
    QString id = d->declaratorId()->unqualifiedName()->text();

    ParsedMethod* method = new ParsedMethod();
    method->setName( id );

    method->setDefinedOnLine( startLine );
    method->setDefinedInFile( m_fileName );
    method->setDeclaredOnLine( startLine );
    method->setDeclaredInFile( m_fileName );
    method->setAccess( m_currentAccess );
    method->setIsStatic( isStatic );
    method->setIsVirtual( isVirtual );
    method->setIsPure( isPure );
    method->setType( typeOfDeclaration(typeSpec, d) );
    if( m_currentClass ){
	method->setIsDestructor( id.startsWith("~") );
	method->setIsConstructor( typeSpec == 0 && id == m_currentClass->name() );
    }
    method->setIsConst( d->constant() != 0 );
 
    ParsedClassContainer* c = m_currentClass ? (ParsedClassContainer*) m_currentClass : (ParsedClassContainer*) m_currentScopeContainer;
    ParsedMethod* m = c->getMethod( method );
    if( m != 0 ){
	method->setDefinedInFile( m->definedInFile() );
	method->setDefinedOnLine( m->definedOnLine() );
	method->setDefinitionEndsOnLine( m->definitionEndsOnLine() );
	c->removeMethod( m );	
    }
    
    parseFunctionArguments( d, method );

    if( m_currentClass ){
	if( m_inSlots )
	    m_currentClass->addSlot( method );
	else if( m_inSignals )
	    m_currentClass->addSignal( method );
	else
	    m_currentClass->addMethod( method );
    } else
	m_currentScopeContainer->addMethod( method );
}

void StoreWalker::parseFunctionArguments( DeclaratorAST* declarator, ParsedMethod* method )
{
    ParameterDeclarationClauseAST* clause = declarator->parameterDeclarationClause();
    ParameterDeclarationListAST* params = clause->parameterDeclarationList();
    
    if( params ){
	QPtrList<ParameterDeclarationAST> l( params->parameterList() );
	QPtrListIterator<ParameterDeclarationAST> it( l );
	while( it.current() ){
	    ParameterDeclarationAST* param = it.current();
	    ++it;
	    
	    ParsedArgument* arg = new ParsedArgument();
	    if( param->declarator()->declaratorId() )
		arg->setName( param->declarator()->declaratorId()->text() );
	    
	    arg->setType( typeOfDeclaration(param->typeSpec(), param->declarator()) );
	    method->addArgument( arg );
	}
    }
}

QString StoreWalker::typeOfDeclaration( TypeSpecifierAST* typeSpec, DeclaratorAST* declarator )
{
    if( !typeSpec || !declarator )
        return QString::null;

    QString text;
    
    if( typeSpec->cvQualify() ){
	QPtrList<AST> l = typeSpec->cvQualify()->nodeList();
	QPtrListIterator<AST> it( l );
	while( it.current() ){
	    text += it.current()->text();
	    ++it;
	    
	    text += " ";
	}
    }
    
    text += typeSpec->text();
    
    if( typeSpec->cv2Qualify() ){
	text += " ";
	QPtrList<AST> l = typeSpec->cv2Qualify()->nodeList();
	QPtrListIterator<AST> it( l );
	while( it.current() ){
	    text += it.current()->text();
	    ++it;
	    
	    text += " ";
	}
    }
    
    text = text.simplifyWhiteSpace();
    
    QPtrList<AST> ptrOpList = declarator->ptrOpList();
    for( QPtrListIterator<AST> it(ptrOpList); it.current(); ++it ){
	text += it.current()->text();
	++it;
    }
    
    return text;
}

