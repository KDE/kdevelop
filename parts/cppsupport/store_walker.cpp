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
    : m_fileName( fileName ), m_store( store ),
      m_currentContainer( 0 ), m_currentClass( 0 ),
      m_currentScopeContainer( 0 ), m_anon( 0 )
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
    m_imports.clear();

    m_store->removeWithReferences( m_fileName );
    m_imports << QStringList();
    m_currentScopeContainer = m_store->globalScope();
    TreeParser::parseTranslationUnit( ast );
    m_imports.pop_back();
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

	nsName.sprintf( "(%s_%d)", shortFileName.local8Bit().data(), m_anon++ );
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

    //ns->setDefinedOnLine( startLine );
    //ns->setDefinitionEndsOnLine( endLine );
    //ns->setDefinedInFile( m_fileName );

    ParsedClassContainer* old_container = m_currentContainer;
    ParsedScopeContainer* old_scope = m_currentScopeContainer;
    m_currentContainer = ns;
    m_currentScopeContainer = ns;
    m_currentScope.push_back( nsName );

    TreeParser::parseNamespace( ast );

    m_currentScope.pop_back();
    m_currentScopeContainer = old_scope;
    m_currentContainer = old_container;
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
    QString name = ast->name()->unqualifiedName()->text();
    m_imports.back().push_back( name );
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

    QString id = d->declaratorId()->unqualifiedName()->text().stripWhiteSpace();

    QString scopeStr = scopeOfDeclarator( d );
    ParsedClassContainer* cl = findContainer( scopeStr, 0, true );
    if( cl == 0 && scopeStr )
        return;   // class not found!!!!!


    if( !cl )
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

#if 0
    if( cl && isConstructor )
	method->setType( cl->name() + "*" );
    else if( cl && isDestructor )
	method->setType( "void" );
    else
#endif
    {
	QString text = typeOfDeclaration( typeSpec, d );
	if( !text.isEmpty() )
	    method->setType( text );
    }

    ParsedClass* klass = dynamic_cast<ParsedClass*>( cl );

    ParsedMethod* m = klass ? klass->getMethod( method ) : cl->getMethod( method );

    bool isStored = m != 0;
    if( m != 0 ){
	delete( method );
	method = m;
    } else {
	method->setDeclaredInFile( m_fileName );
	method->setDeclaredOnLine( startLine );
	method->setDeclarationEndsOnLine( endLine );
    }

    method->setDefinedOnLine( startLine );
    method->setDefinedInFile( m_fileName );
    method->setDefinitionEndsOnLine( endLine );

    if( !isStored && klass ){
	if( m_inSlots )
	    klass->addSlot( method );
	else if( m_inSignals )
	    klass->addSignal( method );
	else
	    klass->addMethod( method );
    } else if( !isStored )
	cl->addMethod( method );

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
	className.sprintf( "(%s_%d)", shortFileName.local8Bit().data(), m_anon++ );
    } else {
	className = ast->name()->unqualifiedName()->text().stripWhiteSpace();
    }

    ParsedClass* klass = new ParsedClass();
    klass->setDeclaredOnLine( startLine );
    klass->setDeclarationEndsOnLine( endLine );
    klass->setDeclaredInFile( m_fileName );
    klass->setDefinedOnLine( startLine );
    klass->setDefinedInFile( m_fileName );
    klass->setDefinitionEndsOnLine( endLine );

    klass->setName( className );
    klass->setDeclaredInScope( m_currentScope.join(".") );

    bool innerClass = m_currentClass != 0;

    if ( kind == "class" ) {
        if (innerClass)
	  m_currentClass->addClass( klass );
	else{
 	  m_store->addClass( klass );
          currentScope()->addClass( klass );
        }
    }
    else {
        if (innerClass)
          m_currentClass->addStruct( klass );
	else{
	  m_store->addStruct( klass );
          currentScope()->addStruct( klass );
        }

    }

    if ( ast->baseClause() )
        parseBaseClause( ast->baseClause(), klass );

    m_currentScope.push_back( className );

    ParsedClass* oldClass = m_currentClass;
    ParsedClassContainer* oldContainer = m_currentContainer;

    m_currentContainer = klass;
    m_currentClass = klass;
    m_imports.push_back( QStringList() );

    TreeParser::parseClassSpecifier( ast );

    m_imports.pop_back();
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
        attr->setDefinitionEndsOnLine( endLine );

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

    QString scopeStr = scopeOfDeclarator( d );
    ParsedClassContainer* cl = findContainer( scopeStr, 0, true );
    if( cl == 0 )
        cl = m_currentContainer;

    ParsedAttribute* attr = findOrInsertAttribute( cl, id );

    QString text = typeOfDeclaration( typeSpec, d );
    if( !text.isEmpty() )
	attr->setType( text );


    bool isFriend = false;
    //bool isVirtual = false;
    bool isStatic = false;
    //bool isInline = false;
    //bool isInitialized = decl->initializer() != 0;

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
    attr->setDefinitionEndsOnLine( endLine );
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
    method->setDeclarationEndsOnLine( endLine );
    method->setDefinitionEndsOnLine( endLine );
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

	QString text = typeOfDeclaration( typeSpec, d );
	if( !text.isEmpty() )
	    method->setType( text );
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

	    QString tp = typeOfDeclaration( param->typeSpec(), param->declarator() );
	    if( !tp.isEmpty() )
		arg->setType( tp );
	    method->addArgument( arg );
	}
    }
}

QString StoreWalker::typeOfDeclaration( TypeSpecifierAST* typeSpec, DeclaratorAST* declarator )
{
    if( !typeSpec || !declarator )
        return QString::null;

    QString text;

    text += typeSpec->text();

    QPtrList<AST> ptrOpList = declarator->ptrOpList();
    for( QPtrListIterator<AST> it(ptrOpList); it.current(); ++it ){
	text += it.current()->text();
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
	Q_UNUSED( isVirtual );

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

ParsedClassContainer* StoreWalker::findContainer( const QString& name, ParsedScopeContainer* container, bool includeImports )
{
    if( !container )
        container = m_store->globalScope();

    QStringList path = QStringList::split( ".", name );
    QStringList::Iterator it = path.begin();
    while( it != path.end() ){
        QString s = *it;
        ++it;

        ParsedScopeContainer* scope = container->getScopeByName( s );
        if( !scope )
            break;

        path.remove( s );
        container = scope;
    }

    if( path.size() == 0 )
        return container;

    QString className = path.join( "." );

    ParsedClass* klass = 0;
    ParsedClassContainer* c = container;
    while( c && path.size() ){
	QString s = path.front();
	path.pop_front();
	klass = c->getClassByName( s );
	if( !klass )
	    klass = c->getStructByName( s );
	if( !klass )
	    break;

	c = klass;
    }

    if( !klass && includeImports ){

        QStringList imports;
        QValueList<QStringList>::Iterator lIt = m_imports.begin();
        while( lIt != m_imports.end() ){
           imports += (*lIt );
           ++lIt;
        }

        QStringList::Iterator impIt = imports.begin();
        while( impIt != imports.end() ){
            ParsedClassContainer* kl = findContainer( (*impIt) + "." + name, container, false );
            if( kl )
                return kl;
            ++impIt;
        }
    }

    return klass;
}

ParsedScopeContainer* StoreWalker::currentScope()
{
    ParsedScopeContainer* scope = m_currentScopeContainer;
    if( !scope )
        scope = m_store->globalScope();
    return scope;
}

QString StoreWalker::scopeOfDeclarator( DeclaratorAST* d )
{
    QStringList scope = m_currentScope;
    if( d && d->declaratorId() && d->declaratorId()->classOrNamespaceNameList().count() ){
        if( d->declaratorId()->isGlobal() )
	    scope.clear();
	QPtrList<ClassOrNamespaceNameAST> l = d->declaratorId()->classOrNamespaceNameList();
	QPtrListIterator<ClassOrNamespaceNameAST> it( l );
	while( it.current() ){
	    if( it.current()->name() ){
	       scope << it.current()->name()->text();
	    }
	    ++it;
	}
    }

    return scope.join( "." );
}
