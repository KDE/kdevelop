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

#include "tag_creator.h"
#include "catalog.h"
#include "ast_utils.h"
#include "cpp_tags.h"
#include "doxydoc.h"

#include <kdebug.h>
#include <qfileinfo.h>
#include <qregexp.h>

DoxyDoc* TagCreator::m_documentation = new DoxyDoc( QStringList() );

TagCreator::TagCreator( const QString& fileName, Catalog* c )
: m_catalog( c ), m_fileName( fileName ), m_anon( 0 )
{
}

TagCreator::~TagCreator()
{
}

void TagCreator::destroyDocumentation()
{
	delete m_documentation;
}

void TagCreator::setDocumentationDirectories( const QStringList& str )
{
	if ( m_documentation )
		delete m_documentation;
	m_documentation = new DoxyDoc( str );
}

void TagCreator::parseDeclaration( DeclarationAST* ast )
{
	if( ast->nodeType() == NodeType_AccessDeclaration ||
	    m_currentAccess.isEmpty() ||
	    m_currentAccess.contains("private")	||	///In order to correctly evaluate templates, the private members are necessary too
	    m_currentAccess.contains("public") ||
	    m_currentAccess.contains("protected") ||
	    m_currentAccess.contains("signals") )
	{
		TreeParser::parseDeclaration( ast );
	}
}

void TagCreator::parseTranslationUnit( TranslationUnitAST* ast )
{
	m_currentScope.clear();
	m_currentAccess = QString::null;
	m_inSlots = false;
	m_inSignals = false;
	m_anon = 0;
	m_imports.clear();
	m_inClass = false;
	
	m_imports << QStringList();
	TreeParser::parseTranslationUnit( ast );
	m_imports.pop_back();
}

void TagCreator::parseNamespace( NamespaceAST* ast )
{
	QString nsName;
	if( !ast->namespaceName() || ast->namespaceName()->text().isEmpty() )
	{
        // anonymous namespace
	}
	else
		nsName = ast->namespaceName()->text();
	
	Tag tag;
	tag.setKind( Tag::Kind_Namespace );
	tag.setFileName( m_fileName );
	tag.setName( nsName );
	tag.setScope( m_currentScope );
	if( !ast->comment().isEmpty() )
		tag.setComment( ast->comment() );
	
	int line, col;
	ast->getStartPosition( &line, &col );
	tag.setStartPosition( line, col );
	
	ast->getEndPosition( &line, &col );
	tag.setEndPosition( line, col );
	
	m_catalog->addItem( tag );
	
	m_currentScope.push_back( nsName );
	TreeParser::parseNamespace( ast );
	m_currentScope.pop_back();
}

void TagCreator::parseElaboratedTypeSpecifier( ElaboratedTypeSpecifierAST* ast )
{
	TreeParser::parseElaboratedTypeSpecifier( ast );
}

void TagCreator::parseUsingDirective( UsingDirectiveAST* ast )
{
	QString name;
	if( ast->name() )
		name = ast->name()->text();
	
	if( !name.isNull() ){
		Tag tag;
		tag.setKind( Tag::Kind_UsingDirective );
		tag.setFileName( m_fileName );
		tag.setName( name );
		tag.setScope( m_currentScope );
		
		int line, col;
		ast->getStartPosition( &line, &col );
		tag.setStartPosition( line, col );
		
		ast->getEndPosition( &line, &col );
		tag.setEndPosition( line, col );
		
		m_catalog->addItem( tag );
	}
	
	m_imports.back().push_back( name );
}

void TagCreator::parseTypedef( TypedefAST* ast )
{
	TypeSpecifierAST* typeSpec = ast->typeSpec();
	InitDeclaratorListAST* declarators = ast->initDeclaratorList();
	
	if( typeSpec && declarators ){
		QString typeId;
		
		if( typeSpec->name() )
			typeId = typeSpec->name()->text();
		
		QPtrList<InitDeclaratorAST> l( declarators->initDeclaratorList() );
		QPtrListIterator<InitDeclaratorAST> it( l );
		
		InitDeclaratorAST* initDecl = 0;
		while( 0 != (initDecl = it.current()) )
		{
			
			QString type, id;
			if( initDecl->declarator() )
			{
				type = typeOfDeclaration( typeSpec, initDecl->declarator() );
				
				DeclaratorAST* d = initDecl->declarator();
				while( d->subDeclarator() )
					d = d->subDeclarator();
				
				if( d->declaratorId() )
					id = d->declaratorId()->text();
			}
			
			Tag tag;
			if( !ast->comment().isEmpty() )
				tag.setComment( ast->comment() );
			
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
			
			++it;
		}
		
	}
}

void TagCreator::parseTemplateDeclaration( TemplateDeclarationAST* ast )
{
	m_currentTemplateDeclarator.push( ast );
	if ( ast->declaration() )
		parseDeclaration( ast->declaration() );
	
	
	
	TreeParser::parseTemplateDeclaration( ast );
	
	m_currentTemplateDeclarator.pop();
}


void TagCreator::parseSimpleDeclaration( SimpleDeclarationAST* ast )
{
	CommentPusher push( *this, ast->comment() );
	
	TypeSpecifierAST* typeSpec = ast->typeSpec();
	InitDeclaratorListAST* declarators = ast->initDeclaratorList();
	
	if( typeSpec )
		parseTypeSpecifier( typeSpec );
	
	if( declarators )
	{
		QPtrList<InitDeclaratorAST> l = declarators->initDeclaratorList();
		
		QPtrListIterator<InitDeclaratorAST> it( l );
		while( it.current() )
		{
			parseMyDeclaration(  ast->functionSpecifier(), ast->storageSpecifier(), typeSpec, it.current() );
			++it;
		}
	}
}

void TagCreator::parseFunctionDefinition( FunctionDefinitionAST* ast )
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
		QPtrList<AST> l = funSpec->nodeList();
		QPtrListIterator<AST> it( l );
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
		QPtrList<AST> l = storageSpec->nodeList();
		QPtrListIterator<AST> it( l );
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
	
	
	QString id = d->declaratorId() ->unqualifiedName() ->text().stripWhiteSpace();
	QString scopeStr = scopeOfDeclarator( d );
	
	Tag tag;
	if( !comment() )
		tag.setComment( ast->comment() );
	
	CppFunction<Tag> tagBuilder( tag );
	tag.setKind( Tag::Kind_Function );
	
	tag.setFileName( m_fileName );
	tag.setName( id );
	tag.setScope( QStringList::split( ".", scopeStr ) );
	if( !ast->comment().isEmpty() )
		tag.setComment( ast->comment() );
	
	int line, col;
	ast->getStartPosition( &line, &col );
	tag.setStartPosition( line, col );
	
	ast->getEndPosition( &line, &col );
	tag.setEndPosition( line, col );
	
	tagBuilder.setType( typeOfDeclaration( typeSpec, d ) );
	
	parseFunctionArguments( tag, d );
	checkTemplateDeclarator( tag );
	
	QString arguments = tag.attribute( "a" ).toStringList().join( "," );
tag.setAttribute( "description", m_documentation->functionDescription( scopeStr.replace( QRegExp( "." ), ":" ), id, typeOfDeclaration( typeSpec, d ), arguments ) );
	
	tagBuilder.setAccess( TagUtils::stringToAccess( m_currentAccess ) );
	
	tagBuilder.setFriend( isFriend );
	tagBuilder.setVirtual( isVirtual );
	tagBuilder.setStatic( isStatic );
	tagBuilder.setInline( isInline );
	tagBuilder.setPure( false );
	tagBuilder.setConst( d->constant() != 0 );
	tagBuilder.setSignal( m_inSignals );
	tagBuilder.setSlot( m_inSlots );
	
	m_catalog->addItem( tag );
	
	if ( !m_currentAccess.isEmpty() )
	{
		tag.setKind( Tag::Kind_FunctionDeclaration );
		m_catalog->addItem( tag );
	}
}

void TagCreator::parseLinkageBody( LinkageBodyAST* ast )
{
	QPtrList<DeclarationAST> l = ast->declarationList();
	QPtrListIterator<DeclarationAST> it( l );
	while ( it.current() )
	{
		parseDeclaration( it.current() );
		++it;
	}
}

void TagCreator::checkTemplateDeclarator( Tag& tag ) {
	if( !m_currentTemplateDeclarator.empty() && m_currentTemplateDeclarator.top() != 0) {
		TemplateDeclarationAST* a = m_currentTemplateDeclarator.top();
		
		m_currentTemplateDeclarator.pop();
		m_currentTemplateDeclarator.push(0);
		
		///the template-declarator belongs to exactly this declaration
		takeTemplateParams( tag, a );
	}
}


void TagCreator::takeTemplateParams( Tag& target, TemplateDeclarationAST* ast) {
	TemplateParameterListAST* pl = ast->templateParameterList();
	if( pl ) {
		QPtrList<TemplateParameterAST> list = pl->templateParameterList();
		
		TemplateParameterAST* curr = list.first();
		while( curr != 0 ) {
			QString a, b;
			if( curr->typeParameter() ) {
				if( curr->typeParameter()->name() )
					a = curr->typeParameter()->name()->text();
				if( curr->typeParameter()->typeId() ) 
					b =  curr->typeParameter()->typeId()->text();
			}
			
			target.addTemplateParam( a, b );
			curr = list.next();
		}
	}
}


void TagCreator::parseClassSpecifier( ClassSpecifierAST* ast )
{
	int startLine, startColumn;
	int endLine, endColumn;
	ast->getStartPosition( &startLine, &startColumn );
	ast->getEndPosition( &endLine, &endColumn );
	
	QString oldAccess = m_currentAccess;
	bool oldInSlots = m_inSlots;
	bool oldInSignals = m_inSignals;
	
	QString kind = ast->classKey() ->text();
	if ( kind == "class" )
		m_currentAccess = "private";
	else
		m_currentAccess = "public";
	m_inSlots = false;
	m_inSignals = false;
	
	QString className;
	if ( !ast->name() )
	{
		//QFileInfo fileInfo( m_fileName );
		//QString shortFileName = fileInfo.baseName();
		//className.sprintf( "(%s_%d)", shortFileName.local8Bit(), m_anon++ );
	}
	else
	{
		className = ast->name() ->text();
	}
	
	Tag tag;
	if( !ast->comment().isEmpty() )
		tag.setComment( ast->comment() );
	
	tag.setKind( Tag::Kind_Class );
	
	tag.setFileName( m_fileName );
	tag.setName( className );
	tag.setScope( m_currentScope );
	
	int line, col;
	ast->getStartPosition( &line, &col );
	tag.setStartPosition( line, col );
	
	ast->getEndPosition( &line, &col );
	tag.setEndPosition( line, col );
	
	
	checkTemplateDeclarator( tag );
	
	
	m_catalog->addItem( tag );
	
	if ( ast->baseClause() )
		parseBaseClause( tag.path(), ast->baseClause() );
	
	m_currentScope.push_back( className );
	int oldInClass = m_inClass;
	m_inClass = true;
	TreeParser::parseClassSpecifier( ast );
	m_currentScope.pop_back();
	m_inClass = oldInClass;
	
	m_currentAccess = oldAccess;
	m_inSlots = oldInSlots;
	m_inSignals = oldInSignals;
}

void TagCreator::parseEnumSpecifier( EnumSpecifierAST* ast )
{
	Tag tag;
	if( !ast->comment().isEmpty() )
		tag.setComment( ast->comment() );
	
	tag.setKind( Tag::Kind_Enum );
	
	tag.setFileName( m_fileName );
	if ( ast->name() )
		tag.setName( ast->name() ->text() );
	tag.setScope( m_currentScope );
	
	int line, col;
	ast->getStartPosition( &line, &col );
	tag.setStartPosition( line, col );
	
	ast->getEndPosition( &line, &col );
	tag.setEndPosition( line, col );
	
	m_catalog->addItem( tag );
	
	QPtrList<EnumeratorAST> l = ast->enumeratorList();
	QPtrListIterator<EnumeratorAST> it( l );
	while ( it.current() )
	{
		QString name = it.current() ->id() ->text();
		
		Tag tag;
		tag.setKind( Tag::Kind_Enumerator );
		tag.setComment( it.current()->comment() );
		
		tag.setFileName( m_fileName );
		tag.setName( name );
		tag.setScope( m_currentScope );

		if( ast->name() ) {
			tag.setAttribute( "enum", ast->name()->text() );
		} else {
			tag.setAttribute( "enum", "const int" );
		}
		
		int line, col;
		it.current() ->getStartPosition( &line, &col );
		tag.setStartPosition( line, col );
		
		it.current() ->getEndPosition( &line, &col );
		tag.setEndPosition( line, col );
		
		m_catalog->addItem( tag );
		
		++it;
	}
	
	TreeParser::parseEnumSpecifier( ast );
}

void TagCreator::parseMyDeclaration( GroupAST* funSpec, GroupAST* storageSpec, TypeSpecifierAST* typeSpec, InitDeclaratorAST* decl )
{
	DeclaratorAST * d = decl->declarator();
	
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
	
	QString scopeStr = scopeOfDeclarator( d );
	
	QString type = typeOfDeclaration( typeSpec, d );
	
	
	bool isFriend = false;
	//bool isVirtual = false;
	bool isStatic = false;
	//bool isInline = false;
	//bool isInitialized = decl->initializer() != 0;
	
	if ( storageSpec )
	{
		QPtrList<AST> l = storageSpec->nodeList();
		QPtrListIterator<AST> it( l );
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
	
	Tag tag;
	CppVariable<Tag> tagBuilder( tag );
	
	tag.setKind( Tag::Kind_Variable );
	tag.setFileName( m_fileName );
	tag.setName( id );
	tag.setScope( QStringList::split( ".", scopeStr ) );
	if( !comment().isEmpty() )
		tag.setComment( comment() );
	
	int line, col;
	decl->getStartPosition( &line, &col );
	tag.setStartPosition( line, col );
	
	decl->getEndPosition( &line, &col );
	tag.setEndPosition( line, col );
	
	tagBuilder.setType( type );
	tagBuilder.setFriend( isFriend );
	tagBuilder.setStatic( isStatic );
	tagBuilder.setAccess( TagUtils::stringToAccess( m_currentAccess ) );
	
	m_catalog->addItem( tag );
}

void TagCreator::parseAccessDeclaration( AccessDeclarationAST * access )
{
	QPtrList<AST> l = access->accessList();
	
	m_currentAccess = l.at( 0 )->text();
	if( m_currentAccess == "signals" )
		m_currentAccess = "protected";
	
	m_inSlots = l.count() > 1 ? l.at( 1 )->text() == "slots" : false;
	m_inSignals = l.count() >= 1 ? l.at( 0 )->text() == "signals" : false;
}

void TagCreator::parseFunctionDeclaration( GroupAST* funSpec, GroupAST* storageSpec,
                                           TypeSpecifierAST * typeSpec, InitDeclaratorAST * decl )
{
	bool isFriend = false;
	bool isVirtual = false;
	bool isStatic = false;
	bool isInline = false;
	bool isPure = decl->initializer() != 0;

	if ( funSpec )
	{
		QPtrList<AST> l = funSpec->nodeList();
		QPtrListIterator<AST> it( l );
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
		QPtrList<AST> l = storageSpec->nodeList();
		QPtrListIterator<AST> it( l );
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

	QString type = typeOfDeclaration( typeSpec, d );

	Tag tag;
	CppFunction<Tag> tagBuilder( tag );

	if( !comment().isEmpty() )
		tag.setComment( comment() );
	tag.setKind( Tag::Kind_FunctionDeclaration );
	tag.setFileName( m_fileName );
	tag.setName( id );
	tag.setScope( m_currentScope );

	int line, col;
	decl->getStartPosition( &line, &col );
	tag.setStartPosition( line, col );

	decl->getEndPosition( &line, &col );
	tag.setEndPosition( line, col );

	tagBuilder.setType( type );
	tagBuilder.setFriend( isFriend );
	tagBuilder.setVirtual( isVirtual );
	tagBuilder.setStatic( isStatic );
	tagBuilder.setInline( isInline );
	tagBuilder.setPure( isPure );
	tagBuilder.setConst( d->constant() != 0 );
	tagBuilder.setSignal( m_inSignals );
	tagBuilder.setSlot( m_inSlots );

	parseFunctionArguments( tag, d );
	checkTemplateDeclarator( tag );
	
	QString arguments = tag.attribute( "a" ).toStringList().join( "," );
	QString scopeStr = m_currentScope.join( "::" );
	tag.setAttribute( "description", m_documentation->functionDescription( scopeStr, id, type, arguments ) );

	m_catalog->addItem( tag );
}

void TagCreator::parseFunctionArguments( Tag& tag, DeclaratorAST* declarator )
{
	ParameterDeclarationClauseAST* clause = declarator->parameterDeclarationClause();
	
	QStringList types;
	QStringList args;
	if( clause && clause->parameterDeclarationList() ){
		ParameterDeclarationListAST* params = clause->parameterDeclarationList();
		QPtrList<ParameterDeclarationAST> l( params->parameterList() );
		QPtrListIterator<ParameterDeclarationAST> it( l );
		
		while( it.current() ){
			ParameterDeclarationAST* param = it.current();
			++it;
			
			QString name;
			if( param->declarator() ){
				name = declaratorToString(param->declarator(), QString::null, true );
			}
			
			QString type = typeOfDeclaration( param->typeSpec(), param->declarator() );
			
			types << type;
			args << name;
		}
		
		if( clause->ellipsis() ){
			types << "...";
			args << "";
		}
		
	}
	
	CppFunction<Tag> tagBuilder( tag );
	
	tagBuilder.setArguments( types );
	tagBuilder.setArgumentNames( args );
}

QString TagCreator::typeOfDeclaration( TypeSpecifierAST* typeSpec, DeclaratorAST* declarator )
{
	if( !typeSpec || !declarator )
		return QString::null;
	
	QString text;
	
	text += typeSpec->text();
	text = text.simplifyWhiteSpace();
	
	QPtrList<AST> ptrOpList = declarator->ptrOpList();
	for( QPtrListIterator<AST> it(ptrOpList); it.current(); ++it )
		text += it.current()->text();
	
	return text;
}

void TagCreator::parseBaseClause( const QString& className, BaseClauseAST * baseClause )
{
	QPtrList<BaseSpecifierAST> l = baseClause->baseSpecifierList();
	QPtrListIterator<BaseSpecifierAST> it( l );
	while ( it.current() )
	{
		BaseSpecifierAST * baseSpecifier = it.current();
		
		QString access;
		if ( baseSpecifier->access() )
			access = baseSpecifier->access() ->text();
		bool isVirtual = baseSpecifier->isVirtual() != 0;
		
		QString baseName;
		if( baseSpecifier->name() == 0 ) return; ///Workaround for some bug elsewhere
		
		QPtrList<ClassOrNamespaceNameAST> l = baseSpecifier->name() ->classOrNamespaceNameList();
		QPtrListIterator<ClassOrNamespaceNameAST> nameIt( l );
		while ( nameIt.current() )
		{
			if ( nameIt.current() ->name() )
			{
				baseName += nameIt.current() ->name() ->text() + "::";
			}
			++nameIt;
		}
		
		if ( baseSpecifier->name() ->unqualifiedName() && baseSpecifier->name() ->unqualifiedName() ->name() )
			baseName += baseSpecifier->name() ->text(); ///I changed this because I need the template-information. I hope it has no bad side-effects.
			//baseSpecifier->name() ->unqualifiedName() ->name() ->text();
		
		Tag tag;
		CppBaseClass<Tag> tagBuilder( tag );
		
		tag.setKind( Tag::Kind_Base_class );
		tag.setFileName( m_fileName );
		tag.setName( className );
		tag.setScope( m_currentScope );
		
		tagBuilder.setBaseClass( baseName );
		tagBuilder.setVirtual( isVirtual );
		tagBuilder.setAccess( TagUtils::stringToAccess( access ) );
		
		m_catalog->addItem( tag );
		
		int line, col;
		baseClause->getStartPosition( &line, &col );
		tag.setStartPosition( line, col );
		
		baseClause->getEndPosition( &line, &col );
		tag.setEndPosition( line, col );
		
		++it;
	}
}

QString TagCreator::scopeOfDeclarator( DeclaratorAST* d )
{
	QStringList scope = m_currentScope;
	if ( d && d->declaratorId() && d->declaratorId() ->classOrNamespaceNameList().count() )
	{
		if ( d->declaratorId() ->isGlobal() )
			scope.clear();
		QPtrList<ClassOrNamespaceNameAST> l = d->declaratorId() ->classOrNamespaceNameList();
		QPtrListIterator<ClassOrNamespaceNameAST> it( l );
		while ( it.current() )
		{
			if ( it.current() ->name() )
				scope << it.current() ->name() ->text();
			
			++it;
		}
	}
	
	return scope.join( "." );
}

int TagUtils::stringToAccess( const QString & access )
{
	QStringList l = QStringList()
		<< "public" << "protected" << "private"
		<< "public slots" << "protected slots" << "private slots"
		<< "signals";
	
	int idx = l.findIndex( access );
	return idx == -1 ? 0 : idx+1;
}

QString TagUtils::accessToString( int id )
{
	if( id == 0 ) return "unknown";
	
	QStringList l = QStringList()
		<< "public" << "protected" << "private"
		<< "public slots" << "protected slots" << "private slots"
		<< "signals";
	
	if( l.at(id-1) != l.end() )
		return l[ id-1 ];
	
	return QString::null;
}

//kate: indent-mode csands; tab-width 4; space-indent off;
