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
#include "ast_utils.h"

#include <classstore.h>
#include <parsedclasscontainer.h>
#include <parsedscopecontainer.h>
#include <kdebug.h>
#include <qfileinfo.h>

StoreWalker::StoreWalker( const QString& fileName, ClassStore* store )
    : m_fileName( fileName ), m_store( store ), m_currentContainer( 0 ), m_currentClass( 0 ), m_anon( 0 )
{
}

StoreWalker::~StoreWalker()
{
}

void StoreWalker::parseTranslationUnit( TranslationUnitAST* ast )
{
    m_currentScope.clear();    
    m_currentContainer = m_store->globalScope();
    m_currentClass = 0;
    m_currentAccess = PIE_PUBLIC;
    m_inSlots = false;
    m_inSignals = false;
    m_anon = 0;

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
    ParsedScopeContainer* scope = dynamic_cast<ParsedScopeContainer*>( m_currentContainer );
    if( !scope ){
        kdDebug(9007) << "!!!!!!!!!!!!!!!!!!!!!!!!!! **error** !!!!!!!!!!!!!!!!!!!!" << endl;
	return;
    }

    QString nsName;
    if( !ast->namespaceName() ){
	QFileInfo fileInfo( m_fileName );
	QString shortFileName = fileInfo.baseName();

	nsName.sprintf( "(%s_%d)", shortFileName.latin1(), m_anon++ );
    } else {
	nsName = ast->namespaceName()->text();
    }

    int startLine, startColumn;
    int endLine, endColumn;
    ast->getStartPosition( &startLine, &startColumn );
    ast->getEndPosition( &endLine, &endColumn );

    ParsedScopeContainer* ns = findOrInsertScopeContainer( scope, nsName );
    ns->setDeclaredOnLine( startLine );
    ns->setDeclaredInFile( m_fileName );
    ns->setDeclarationEndsOnLine( endLine );

    ns->setDefinedOnLine( startLine );
    ns->setDefinedInFile( m_fileName );
    
    ParsedClassContainer* old_scope = m_currentContainer;
    m_currentContainer = ns;
    m_currentScope.push_back( nsName );

    TreeParser::parseNamespace( ast );

    m_currentScope.pop_back();
    m_currentContainer = old_scope;
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
    TypeSpecifierAST* typeSpec = ast->typeSpec();
    GroupAST* funSpec = ast->functionSpecifier();
    GroupAST* storageSpec = ast->storageSpecifier();

    if( !ast->initDeclarator() )
	return;

    DeclaratorAST* d = ast->initDeclarator()->declarator();

    if( !d->declaratorId() )
	return;

    bool isFriend = false;
    bool isVirtual = false;
    bool isStatic = false;
    bool isInline = false;

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
    ast->getStartPosition( &startLine, &startColumn );
    ast->getEndPosition( &endLine, &endColumn );

    QString id = d->declaratorId()->unqualifiedName()->text();
    NestedNameSpecifierAST* nestedName = d->declaratorId()->nestedName();

    QStringList scope = m_currentScope;
    if( nestedName ){
        if( d->declaratorId()->isGlobal() )
	    scope.clear();
	QPtrList<ClassOrNamespaceNameAST> l = nestedName->classOrNamespaceNameList();
	QPtrListIterator<ClassOrNamespaceNameAST> it( l );
	while( it.current() ){
	    scope << it.current()->text();
	    ++it;
	}
    }

    ParsedClassContainer* cl = getClassByName( m_store, scope.join(".") );
    if( cl == 0 )
        cl = m_store->getScopeByName( scope.join(".") );
    if( cl == 0 )
        cl = m_currentContainer;

    ParsedMethod* method = new ParsedMethod();
    method->setName( id );
    method->setIsConst( d->constant() != 0 );

    parseFunctionArguments( d, method );

    if( cl )
	method->setAccess( m_currentAccess );
    if( isStatic )
	method->setIsStatic( isStatic );
    if( cl && isVirtual )
	method->setIsVirtual( isVirtual );

    bool isDestructor = id.startsWith("~");
    method->setIsDestructor( isDestructor );

    bool isConstructor = cl && typeSpec == 0 && id == cl->name();
    method->setIsConstructor( isConstructor );

    if( cl && isConstructor )
	method->setType( cl->name() + "*" );
    else if( cl && isDestructor )
	method->setType( "void" );
    else {
	QString text = typeOfDeclaration( typeSpec, d );
	if( !text.isEmpty() )
	    method->setType( text );
    }

    ParsedMethod* m = cl->getMethod( method );
    bool isStored = m != 0;

    if( m != 0 )
    {
	delete( method );
	method = m;
    }

    method->setDefinedOnLine( startLine );
    method->setDefinedInFile( m_fileName );

    ParsedClass* klass = dynamic_cast<ParsedClass*>( cl );

    if( !isStored && klass ){
	if( m_inSlots )
	    klass->addSlot( method );
	else if( m_inSignals )
	    klass->addSignal( method );
	else
	    klass->addMethod( method );
    } else if( !isStored )
	m_currentContainer->addMethod( method );

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
	className.sprintf( "(%s_%d)", shortFileName.latin1(), m_anon++ );
    } else {
	className = ast->name()->text();
    }
    
    ParsedClass* klass = new ParsedClass();
    klass->setDeclaredOnLine( startLine );
    klass->setDeclaredInFile( m_fileName );
    klass->setDefinedOnLine( startLine );
    klass->setDefinedInFile( m_fileName );
    
    klass->setName( className );
    klass->setDeclaredInScope( m_currentScope.join(".") );

    bool inStore = false;
    if( kind == "class" )
	inStore = m_store->hasClass( klass->path() );
    else 
	inStore = m_store->hasStruct( klass->path() );

    if( inStore ){
	ParsedClass* parsedClassRef = getClassByName( m_store, klass->path() );
	parsedClassRef->setDeclaredOnLine( klass->declaredOnLine() );
	parsedClassRef->setDeclaredInFile( klass->declaredInFile() );
	parsedClassRef->setDeclaredInScope( klass->declaredInScope() );
	delete klass;
	klass = parsedClassRef;
    }

    if( kind == "class" ){
	m_currentContainer->addClass( klass );

	if( !inStore )
	    m_store->addClass( klass );
    } else {
	m_currentContainer->addStruct( klass );

	if( !inStore )
	    m_store->addStruct( klass );
    }

    if( ast->baseClause() )
        parseBaseClause( ast->baseClause(), klass );

    m_currentScope.push_back( className );

    ParsedClass* oldClass = m_currentClass;
    ParsedClassContainer* oldContainer = m_currentContainer;

    m_currentContainer = klass;
    m_currentClass = klass;
    TreeParser::parseClassSpecifier( ast );
    m_currentContainer = oldContainer;
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
	ParsedAttribute* attr = findOrInsertAttribute( m_currentContainer, it.current()->id()->text() );
	attr->setType( "int" );
	attr->setIsStatic( true );

	int startLine, startColumn;
	int endLine, endColumn;
	it.current()->getStartPosition( &startLine, &startColumn );
	it.current()->getEndPosition( &endLine, &endColumn );

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

    DeclaratorAST* t = d;
    while( t && t->subDeclarator() )
	t = t->subDeclarator();

    QString id;
    if( t && t->declaratorId() && t->declaratorId()->unqualifiedName() )
	id = t->declaratorId()->unqualifiedName()->text();

    QStringList scope = m_currentScope;
    if( d->declaratorId() && d->declaratorId()->nestedName() ){
        if( d->declaratorId()->isGlobal() )
	    scope.clear();
        NestedNameSpecifierAST* nestedName = d->declaratorId()->nestedName();
	QPtrList<ClassOrNamespaceNameAST> l = nestedName->classOrNamespaceNameList();
	QPtrListIterator<ClassOrNamespaceNameAST> it( l );
	while( it.current() ){
	    scope << it.current()->text();
	    ++it;
	}
    }

    ParsedClassContainer* cl = getClassByName( m_store, scope.join(".") );
    if( cl == 0 )
        cl = m_store->getScopeByName( scope.join(".") );
    if( cl == 0 )
        cl = m_currentContainer;

    ParsedAttribute* attr = findOrInsertAttribute( cl, id );

    bool isFriend = false;
    bool isVirtual = false;
    bool isStatic = false;
    bool isInline = false;
    bool isInitialized = decl->initializer() != 0;

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

    attr->setDeclaredOnLine( startLine );
    attr->setDeclaredInFile( m_fileName );
    attr->setDeclarationEndsOnLine( endLine );
    attr->setDefinedOnLine( startLine );
    attr->setDefinedInFile( m_fileName );
    if( isStatic )
	attr->setIsStatic( true );
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
    ParsedScopeContainer* ns = scope->getScopeByName( name );
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
    ParsedAttribute* attr = scope->getAttributeByName( name );
    if( !attr ){
	attr = new ParsedAttribute();
	attr->setName( name );

	scope->addAttribute( attr );
    }

    return attr;
}

void StoreWalker::parseFunctionDeclaration(  GroupAST* funSpec, GroupAST* storageSpec,
					     TypeSpecifierAST * typeSpec, InitDeclaratorAST * decl )
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
    parseFunctionArguments( d, method );

    if( m_currentClass ){
	bool isDestructor = id.startsWith("~");
	method->setIsDestructor( isDestructor );

	bool isConstructor = typeSpec == 0 && id == m_currentClass->name();
	method->setIsConstructor( isConstructor );

	if( m_currentClass && isConstructor )
	    method->setType( m_currentClass->name() + "*" );
	else if( m_currentClass && isDestructor )
	    method->setType( "void" );
	else
	    method->setType( typeOfDeclaration(typeSpec, d) );
    } else {
	QString text = typeOfDeclaration( typeSpec, d );
	if( !text.isEmpty() )
	    method->setType( text );
    }
    method->setIsConst( d->constant() != 0 );

    ParsedMethod* m = m_currentContainer->getMethod( method );
    if( m != 0 ){
	method->setDefinedInFile( m->definedInFile() );
	method->setDefinedOnLine( m->definedOnLine() );
	method->setDefinitionEndsOnLine( m->definitionEndsOnLine() );
	m_currentContainer->removeMethod( m );
    }

    if( m_currentClass ){
	if( m_inSlots )
	    m_currentClass->addSlot( method );
	else if( m_inSignals )
	    m_currentClass->addSignal( method );
	else
	    m_currentClass->addMethod( method );
    } else
	m_currentContainer->addMethod( method );
}

void StoreWalker::parseFunctionArguments( DeclaratorAST* declarator, ParsedMethod* method )
{
    ParameterDeclarationClauseAST* clause = declarator->parameterDeclarationClause();

    if( clause && clause->parameterDeclarationList() ){
        ParameterDeclarationListAST* params = clause->parameterDeclarationList();
	QPtrList<ParameterDeclarationAST> l( params->parameterList() );
	QPtrListIterator<ParameterDeclarationAST> it( l );
	while( it.current() ){
	    ParameterDeclarationAST* param = it.current();
	    ++it;

	    ParsedArgument* arg = new ParsedArgument();
	    
	    if( param->declarator() ){
		QString text = declaratorToString(param->declarator(), QString::null, true ); 
		if( !text.isEmpty() )
		    arg->setName( text );
	    }

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

void StoreWalker::parseBaseClause( BaseClauseAST * baseClause, ParsedClass * klass )
{
    klass->parents.clear();
    QPtrList<BaseSpecifierAST> l = baseClause->baseSpecifierList();
    QPtrListIterator<BaseSpecifierAST> it( l );
    while( it.current() ){
	BaseSpecifierAST* baseSpecifier = it.current();

	QString access;
	if( baseSpecifier->access() )
	    access = baseSpecifier->access()->text();
	bool isVirtual = baseSpecifier->isVirtual() != 0;

	QString baseName;
	if( baseSpecifier->name() )
	    baseName = baseSpecifier->name()->text();

	ParsedParent* parent = new ParsedParent();
	parent->setName( baseName );
	if( access == "public" )
	    parent->setAccess( PIE_PUBLIC );
	else if( access == "protected" )
	    parent->setAccess( PIE_PROTECTED );
	else
	    parent->setAccess( PIE_PRIVATE );

	klass->addParent( parent );
	++it;
    }
}

ParsedClass * StoreWalker::getClassByName( ParsedClassContainer * container, const QString & name )
{
    ParsedClass* c = container->getClassByName( name );
    if( !c )
	c = container->getStructByName( name );
    return c;
}

ParsedClass * StoreWalker::getClassByName( ClassStore * container, const QString & name )
{
    ParsedClass* c = container->getClassByName( name );
    if( !c )
	c = container->getStructByName( name );
    return c;
}

