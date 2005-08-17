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
#include "urlutil.h"

#include <kdebug.h>
#include <qfileinfo.h>
#include <qdir.h>
//Added by qt3to4:
#include <Q3PtrList>

StoreWalker::StoreWalker( const QString& fileName, CodeModel* store )
: m_store( store ), m_anon( 0 )
{
	m_fileName = URLUtil::canonicalPath( fileName );
	
	//kdDebug(9007) << "StoreWalker::StoreWalker(" << m_fileName << ")" << endl;
}

StoreWalker::~StoreWalker()
{}

void StoreWalker::parseTranslationUnit( TranslationUnitAST* ast )
{
	m_file = m_store->create<FileModel>();
	m_file->setName( m_fileName ); /// @todo ?!?
	
	m_currentScope.clear();
	m_currentNamespace.clear();
	m_currentClass.clear();
	
	m_currentAccess = CodeModelItem::Public;
	m_inSlots = false;
	m_inSignals = false;
	m_inStorageSpec = false;
	m_inTypedef = false;
	m_currentDeclarator = 0;
	m_anon = 0;
	m_imports.clear();
	
	m_imports << QStringList();
	TreeParser::parseTranslationUnit( ast );
	m_imports.pop_back();
}

void StoreWalker::parseDeclaration( DeclarationAST* ast )
{
	TreeParser::parseDeclaration( ast );
}

void StoreWalker::parseLinkageSpecification( LinkageSpecificationAST* ast )
{
	int inStorageSpec = m_inStorageSpec;
	m_inStorageSpec = true;
	TreeParser::parseLinkageSpecification( ast );
	m_inStorageSpec = inStorageSpec;
}

void StoreWalker::parseNamespace( NamespaceAST* ast )
{
	if ( !m_currentClass.isEmpty() )
	{
		kdDebug( 9007 ) << "!!!!!!!!!!!!!!!!!!!!!!!!!! **error** !!!!!!!!!!!!!!!!!!!!" << endl;
		return ;
	}
	
	QString nsName;
	if ( !ast->namespaceName() || ast->namespaceName()->text().isEmpty() )
	{
		QFileInfo fileInfo( m_fileName );
		QString shortFileName = fileInfo.baseName();
		
		nsName.sprintf( "(%s_%d)", shortFileName.local8Bit().data(), m_anon++ );
	}
	else
	{
		nsName = ast->namespaceName() ->text();
	}
	
	NamespaceDom ns = findOrInsertNamespace( ast, nsName );
	
	m_currentScope.push_back( nsName );
	m_currentNamespace.push( ns );
	
	TreeParser::parseNamespace( ast );
	
	m_currentNamespace.pop();
	m_currentScope.pop_back();
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
}

void StoreWalker::parseTypedef( TypedefAST* ast )
{
#if 0
	DeclaratorAST * oldDeclarator = m_currentDeclarator;
	
	if ( ast && ast->initDeclaratorList() && ast->initDeclaratorList() ->initDeclaratorList().count() > 0 )
	{
		Q3PtrList<InitDeclaratorAST> lst( ast->initDeclaratorList() ->initDeclaratorList() );
		m_currentDeclarator = lst.at( 0 ) ->declarator();
	}
	
	m_inTypedef = true;
	
	TreeParser::parseTypedef( ast );
	
	m_inTypedef = false;
	m_currentDeclarator = oldDeclarator;
#else
	
	TypeSpecifierAST* typeSpec = ast->typeSpec();
	InitDeclaratorListAST* declarators = ast->initDeclaratorList();
	
	if ( typeSpec && declarators )
	{
		QString typeId;
		
		if ( typeSpec->name() )
			typeId = typeSpec->name() ->text();
		
		Q3PtrList<InitDeclaratorAST> l( declarators->initDeclaratorList() );
		Q3PtrListIterator<InitDeclaratorAST> it( l );
		
		InitDeclaratorAST* initDecl = 0;
		while ( 0 != ( initDecl = it.current() ) )
		{
			
			QString type, id;
			if ( initDecl->declarator() )
			{
				type = typeOfDeclaration( typeSpec, initDecl->declarator() );
				
				DeclaratorAST* d = initDecl->declarator();
				while ( d->subDeclarator() )
				{
					d = d->subDeclarator();
				}
				
				if ( d->declaratorId() )
					id = d->declaratorId() ->text();
			}
			
			TypeAliasDom typeAlias = m_store->create<TypeAliasModel>();
			typeAlias->setFileName( m_fileName );
			typeAlias->setName( id );
			typeAlias->setType( type );
			
			int line, col;
			initDecl->getStartPosition( &line, &col );
			typeAlias->setStartPosition( line, col );
			
			initDecl->getEndPosition( &line, &col );
			typeAlias->setEndPosition( line, col );
			
			if ( m_currentClass.top() )
				m_currentClass.top() ->addTypeAlias( typeAlias );
			else if ( m_currentNamespace.top() )
				m_currentNamespace.top() ->addTypeAlias( typeAlias );
			else
				m_file->addTypeAlias( typeAlias );
			
#if 0
			
			Tag tag;
			tag.setKind( Tag::Kind_Typedef );
			tag.setFileName( m_fileName );
			tag.setName( id );
			tag.setScope( m_currentScope );
			tag.setAttribute( "t", type );
			int line, col;
			initDecl->getStartPosition( &line, &col );
			
			tag.setStartPosition( line, col );
			
			initDecl->getEndPosition( &line, &col );
			tag.setEndPosition( line, col );
			
			m_catalog->addItem( tag );
#endif
			
			++it;
		}
		
	}
#endif
}

void StoreWalker::parseTemplateDeclaration( TemplateDeclarationAST* ast )
{
	if ( ast->declaration() )
		parseDeclaration( ast->declaration() );
	
	TreeParser::parseTemplateDeclaration( ast );
}

void StoreWalker::parseSimpleDeclaration( SimpleDeclarationAST* ast )
{
	TypeSpecifierAST * typeSpec = ast->typeSpec();
	InitDeclaratorListAST* declarators = ast->initDeclaratorList();
	
	if ( typeSpec )
		parseTypeSpecifier( typeSpec );
	
	if ( declarators )
	{
		Q3PtrList<InitDeclaratorAST> l = declarators->initDeclaratorList();
		
		Q3PtrListIterator<InitDeclaratorAST> it( l );
		while ( it.current() )
		{
			parseDeclaration( ast->functionSpecifier(), ast->storageSpecifier(),
			                  typeSpec, it.current() );
			++it;
		}
	}
}

void StoreWalker::parseFunctionDefinition( FunctionDefinitionAST* ast )
{
	TypeSpecifierAST * typeSpec = ast->typeSpec();
	GroupAST* funSpec = ast->functionSpecifier();
	GroupAST* storageSpec = ast->storageSpecifier();
	
	if ( !ast->initDeclarator() )
		return ;
	
	DeclaratorAST* d = ast->initDeclarator() ->declarator();
	
	if ( !d->declaratorId() )
		return ;
	
	bool isFriend = false;
	bool isVirtual = false;
	bool isStatic = false;
	bool isInline = false;
	
	if ( funSpec )
	{
		Q3PtrList<AST> l = funSpec->nodeList();
		Q3PtrListIterator<AST> it( l );
		while ( it.current() )
		{
			QString text = it.current() ->text();
			if ( text == "virtual" )
				isVirtual = true;
			else if ( text == "inline" )
				isInline = true;
			++it;
		}
	}
	
	if ( storageSpec )
	{
		Q3PtrList<AST> l = storageSpec->nodeList();
		Q3PtrListIterator<AST> it( l );
		while ( it.current() )
		{
			QString text = it.current() ->text();
			if ( text == "friend" )
				isFriend = true;
			else if ( text == "static" )
				isStatic = true;
			++it;
		}
	}
	
	int startLine, startColumn;
	int endLine, endColumn;
	ast->getStartPosition( &startLine, &startColumn );
	ast->getEndPosition( &endLine, &endColumn );
	
	QString id = d->declaratorId() ->unqualifiedName() ->text().stripWhiteSpace();
	
	QStringList scope = scopeOfDeclarator( d, m_currentScope );
	
	FunctionDefinitionDom method = m_store->create<FunctionDefinitionModel>();
	method->setScope( scope );
	method->setName( id );
	
	parseFunctionArguments( d, model_cast<FunctionDom>( method ) );
	
	QString text = typeOfDeclaration( typeSpec, d );
	if ( !text.isEmpty() )
		method->setResultType( text );
	
	method->setFileName( m_fileName );
	method->setStartPosition( startLine, startColumn );
	method->setEndPosition( endLine, endColumn );
	
	if ( m_inSignals )
		method->setSignal( true );
	
	if ( m_inSlots )
		method->setSlot( true );
	
	if ( m_currentClass.top() || ( method->name() == "main" && scope.isEmpty() ) )
	{
		method->setConstant( d->constant() != 0 );
		method->setAccess( m_currentAccess );
		method->setStatic( isStatic );
		method->setVirtual( isVirtual );
		
		if ( m_currentClass.top() )
			m_currentClass.top() ->addFunction( model_cast<FunctionDom>( method ) );
		else
			m_file->addFunction( model_cast<FunctionDom>( method ) );
	}
	
	if ( m_currentClass.top() )
		m_currentClass.top() ->addFunctionDefinition( method );
	else if ( m_currentNamespace.top() )
		m_currentNamespace.top() ->addFunctionDefinition( method );
	else
		m_file->addFunctionDefinition( method );
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
	
	int oldAccess = m_currentAccess;
	bool oldInSlots = m_inSlots;
	bool oldInSignals = m_inSignals;
	
	QString kind = ast->classKey() ->text();
	if ( kind == "class" )
		m_currentAccess = CodeModelItem::Private;
	else
		m_currentAccess = CodeModelItem::Public;
	m_inSlots = false;
	m_inSignals = false;
	
	QString className;
	if ( !ast->name() && m_currentDeclarator && m_currentDeclarator->declaratorId() )
	{
		className = m_currentDeclarator->declaratorId() ->text().stripWhiteSpace();
	}
	else if ( !ast->name() )
	{
		QFileInfo fileInfo( m_fileName );
		QString shortFileName = fileInfo.baseName();
		className.sprintf( "(%s_%d)", shortFileName.local8Bit().data(), m_anon++ );
	}
	else
	{
		className = ast->name() ->unqualifiedName() ->text().stripWhiteSpace();
	}
	
	if ( !scopeOfName( ast->name(), QStringList() ).isEmpty() )
	{
		kdDebug( 9007 ) << "skip private class declarations" << endl;
		return ;
	}
	
	ClassDom klass = m_store->create<ClassModel>();
	klass->setStartPosition( startLine, startColumn );
	klass->setEndPosition( endLine, endColumn );
	klass->setFileName( m_fileName );
	
	klass->setName( className );
	
	klass->setScope( m_currentScope );
	
	if ( m_currentClass.top() )
		m_currentClass.top() ->addClass( klass );
	else if ( m_currentNamespace.top() )
		m_currentNamespace.top() ->addClass( klass );
	else
		m_file->addClass( klass );
	
	if ( ast->baseClause() )
		parseBaseClause( ast->baseClause(), klass );
	
	m_currentScope.push_back( className );
	m_currentClass.push( klass );
	
	m_imports.push_back( QStringList() );
	
	TreeParser::parseClassSpecifier( ast );
	
	m_imports.pop_back();
	m_currentClass.pop();
	
	m_currentScope.pop_back();
	
	m_currentAccess = oldAccess;
	m_inSlots = oldInSlots;
	m_inSignals = oldInSignals;
}

void StoreWalker::parseEnumSpecifier( EnumSpecifierAST* ast )
{
	Q3PtrList<EnumeratorAST> l = ast->enumeratorList();
	Q3PtrListIterator<EnumeratorAST> it( l );
	while ( it.current() )
	{
		VariableDom attr = m_store->create<VariableModel>();
		attr->setName( it.current() ->id() ->text() );
		attr->setFileName( m_fileName );
		attr->setAccess( m_currentAccess );
		attr->setType( "int" );
		attr->setStatic( true );
		
		int startLine, startColumn;
		int endLine, endColumn;
		it.current() ->getStartPosition( &startLine, &startColumn );
		attr->setStartPosition( startLine, startColumn );
		
		it.current() ->getEndPosition( &endLine, &endColumn );
		attr->setEndPosition( endLine, endColumn );
		
		if ( m_currentClass.top() )
			m_currentClass.top() ->addVariable( attr );
		else if ( m_currentNamespace.top() )
			m_currentNamespace.top() ->addVariable( attr );
		else
			m_file->addVariable( attr );
		
		++it;
	}
}

void StoreWalker::parseElaboratedTypeSpecifier( ElaboratedTypeSpecifierAST* ast )
{
	TreeParser::parseElaboratedTypeSpecifier( ast );
}

void StoreWalker::parseTypeDeclaratation( TypeSpecifierAST* typeSpec )
{
	parseTypeSpecifier( typeSpec );
}

void StoreWalker::parseDeclaration( GroupAST* funSpec, GroupAST* storageSpec, 
                                    TypeSpecifierAST* typeSpec, InitDeclaratorAST* decl )
{
	if ( m_inStorageSpec )
		return ;
	
	DeclaratorAST* d = decl->declarator();
	
	if ( !d )
		return ;
	
	if ( !d->subDeclarator() && d->parameterDeclarationClause() )
		return parseFunctionDeclaration( funSpec, storageSpec, typeSpec, decl );
	
	DeclaratorAST* t = d;
	while ( t && t->subDeclarator() )
		t = t->subDeclarator();
	
	QString id;
	if ( t && t->declaratorId() && t->declaratorId() ->unqualifiedName() )
		id = t->declaratorId() ->unqualifiedName() ->text();
	
	if ( !scopeOfDeclarator( d, QStringList() ).isEmpty() )
	{
		kdDebug( 9007 ) << "skip declaration" << endl;
		return ;
	}
	
	VariableDom attr = m_store->create<VariableModel>();
	attr->setName( id );
	attr->setFileName( m_fileName );
	
	if ( m_currentClass.top() )
		m_currentClass.top() ->addVariable( attr );
	else if ( m_currentNamespace.top() )
		m_currentNamespace.top() ->addVariable( attr );
	else
		m_file->addVariable( attr );
	
	attr->setAccess( m_currentAccess );
	
	QString text = typeOfDeclaration( typeSpec, d );
	if ( !text.isEmpty() )
		attr->setType( text );
	
	bool isFriend = false;
	//bool isVirtual = false;
	bool isStatic = false;
	//bool isInline = false;
	//bool isInitialized = decl->initializer() != 0;
	
	if ( storageSpec )
	{
		Q3PtrList<AST> l = storageSpec->nodeList();
		Q3PtrListIterator<AST> it( l );
		while ( it.current() )
		{
			QString text = it.current() ->text();
			if ( text == "friend" )
				isFriend = true;
			else if ( text == "static" )
				isStatic = true;
			++it;
		}
	}
	
	int startLine, startColumn;
	int endLine, endColumn;
	decl->getStartPosition( &startLine, &startColumn );
	decl->getEndPosition( &endLine, &endColumn );
	
	attr->setStartPosition( startLine, startColumn );
	attr->setEndPosition( endLine, endColumn );
	attr->setStatic( isStatic );
}

void StoreWalker::parseAccessDeclaration( AccessDeclarationAST * access )
{
	Q3PtrList<AST> l = access->accessList();
	
	QString accessStr = l.at( 0 ) ->text();
	if ( accessStr == "public" )
		m_currentAccess = CodeModelItem::Public;
	else if ( accessStr == "protected" )
		m_currentAccess = CodeModelItem::Protected;
	else if ( accessStr == "private" )
		m_currentAccess = CodeModelItem::Private;
	else if ( accessStr == "signals" )
		m_currentAccess = CodeModelItem::Protected;
	else
		m_currentAccess = CodeModelItem::Public;
	
	m_inSlots = l.count() > 1 ? l.at( 1 ) ->text() == "slots" : false;
	m_inSignals = l.count() >= 1 ? l.at( 0 ) ->text() == "signals" : false;
}

NamespaceDom StoreWalker::findOrInsertNamespace( NamespaceAST* ast, const QString & name )
{
	if ( m_currentNamespace.top() && m_currentNamespace.top() ->hasNamespace( name ) )
		return m_currentNamespace.top() ->namespaceByName( name );
	
	if ( m_file->hasNamespace( name ) )
		return m_file->namespaceByName( name );
	
	int startLine, startColumn;
	int endLine, endColumn;
	ast->getStartPosition( &startLine, &startColumn );
	ast->getEndPosition( &endLine, &endColumn );
	
	NamespaceDom ns = m_store->create<NamespaceModel>();
	ns->setFileName( m_fileName );
	ns->setName( name );
	ns->setStartPosition( startLine, startColumn );
	ns->setEndPosition( endLine, endColumn );
	
	ns->setScope( m_currentScope );
	
	if ( m_currentNamespace.top() )
		m_currentNamespace.top() ->addNamespace( ns );
	else
		m_file->addNamespace( ns );
	
	return ns;
}

void StoreWalker::parseFunctionDeclaration( GroupAST* funSpec, GroupAST* storageSpec,
                                            TypeSpecifierAST * typeSpec, InitDeclaratorAST * decl )
{
	bool isFriend = false;
	bool isVirtual = false;
	bool isStatic = false;
	bool isInline = false;
	bool isPure = decl->initializer() != 0;
	
	if ( funSpec )
	{
		Q3PtrList<AST> l = funSpec->nodeList();
		Q3PtrListIterator<AST> it( l );
		while ( it.current() )
		{
			QString text = it.current() ->text();
			if ( text == "virtual" )
				isVirtual = true;
			else if ( text == "inline" )
				isInline = true;
			++it;
		}
	}
	
	if ( storageSpec )
	{
		Q3PtrList<AST> l = storageSpec->nodeList();
		Q3PtrListIterator<AST> it( l );
		while ( it.current() )
		{
			QString text = it.current() ->text();
			if ( text == "friend" )
				isFriend = true;
			else if ( text == "static" )
				isStatic = true;
			++it;
		}
	}
	
	int startLine, startColumn;
	int endLine, endColumn;
	decl->getStartPosition( &startLine, &startColumn );
	decl->getEndPosition( &endLine, &endColumn );
	
	DeclaratorAST* d = decl->declarator();
	QString id = d->declaratorId() ->unqualifiedName() ->text();
	
	FunctionDom method = m_store->create<FunctionModel>();
	method->setName( id );
	
	method->setFileName( m_fileName );
	method->setStartPosition( startLine, startColumn );
	method->setEndPosition( endLine, endColumn );
	method->setAccess( m_currentAccess );
	method->setStatic( isStatic );
	method->setVirtual( isVirtual );
	method->setAbstract( isPure );
	parseFunctionArguments( d, method );
	
	if ( m_inSignals )
		method->setSignal( true );
	
	if ( m_inSlots )
		method->setSlot( true );
	
	QString text = typeOfDeclaration( typeSpec, d );
	if ( !text.isEmpty() )
		method->setResultType( text );
	
	method->setConstant( d->constant() != 0 );
	method->setScope( scopeOfDeclarator( d, m_currentScope ) );
	
	if ( m_currentClass.top() )
		m_currentClass.top() ->addFunction( method );
	else if ( m_currentNamespace.top() )
		m_currentNamespace.top() ->addFunction( method );
	else
		m_file->addFunction( method );
}

void StoreWalker::parseFunctionArguments( DeclaratorAST* declarator, FunctionDom method )
{
	ParameterDeclarationClauseAST * clause = declarator->parameterDeclarationClause();
	
	if ( clause && clause->parameterDeclarationList() )
	{
		ParameterDeclarationListAST * params = clause->parameterDeclarationList();
		Q3PtrList<ParameterDeclarationAST> l( params->parameterList() );
		Q3PtrListIterator<ParameterDeclarationAST> it( l );
		while ( it.current() )
		{
			ParameterDeclarationAST * param = it.current();
			++it;
			
			ArgumentDom arg = m_store->create<ArgumentModel>();
			
			if ( param->declarator() )
			{
				QString text = declaratorToString( param->declarator(), QString::null, true );
				if ( !text.isEmpty() )
					arg->setName( text );
			}
			
			QString tp = typeOfDeclaration( param->typeSpec(), param->declarator() );
			if ( !tp.isEmpty() )
				arg->setType( tp );
			
			method->addArgument( arg );
		}
	}
}

QString StoreWalker::typeOfDeclaration( TypeSpecifierAST* typeSpec, DeclaratorAST* declarator )
{
	if ( !typeSpec || !declarator )
		return QString::null;
	
	QString text;
	
	text += typeSpec->text();
	
	Q3PtrList<AST> ptrOpList = declarator->ptrOpList();
	for ( Q3PtrListIterator<AST> it( ptrOpList ); it.current(); ++it )
	{
		text += it.current() ->text();
	}
	
	return text;
}

void StoreWalker::parseBaseClause( BaseClauseAST * baseClause, ClassDom klass )
{
	Q3PtrList<BaseSpecifierAST> l = baseClause->baseSpecifierList();
	Q3PtrListIterator<BaseSpecifierAST> it( l );
	while ( it.current() )
	{
		BaseSpecifierAST * baseSpecifier = it.current();
		
		QString baseName;
		if ( baseSpecifier->name() )
			baseName = baseSpecifier->name() ->text();
		
		klass->addBaseClass( baseName );
		
		++it;
	}
}

QStringList StoreWalker::scopeOfName( NameAST* id, const QStringList& startScope )
{
	QStringList scope = startScope;
	if ( id && id->classOrNamespaceNameList().count() )
	{
		if ( id->isGlobal() )
			scope.clear();
		Q3PtrList<ClassOrNamespaceNameAST> l = id->classOrNamespaceNameList();
		Q3PtrListIterator<ClassOrNamespaceNameAST> it( l );
		while ( it.current() )
		{
			if ( it.current() ->name() )
			{
				scope << it.current() ->name() ->text();
			}
			++it;
		}
	}
	
	return scope;
}

QStringList StoreWalker::scopeOfDeclarator( DeclaratorAST* d, const QStringList& startScope )
{
	return scopeOfName( d->declaratorId(), startScope );
}

//kate: indent-mode csands; tab-width 4; space-indent off;
