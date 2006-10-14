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
#include <driver.h>

StoreWalker::StoreWalker( const QString& fileName, CodeModel* store )
: m_store( store ), m_anon( 0 )
{
	m_fileName = URLUtil::canonicalPath( fileName );
    m_hashedFileName = HashedString( m_fileName );
	
	//kdDebug(9007) << "StoreWalker::StoreWalker(" << m_fileName << ")" << endl;
}

StoreWalker::~StoreWalker()
{}


void StoreWalker::parseTranslationUnit( const ParsedFile& ast )
{
	m_file = m_store->create<FileModel>();
	m_file->setName( m_fileName ); /// @todo ?!?

	m_currentScope.clear();
	m_currentNamespace.clear();
	m_currentClass.clear();

	ParsedFilePointer p = new ParsedFile( ast );
	p->setTranslationUnit( 0 ); //Necessary so the memory is not bloated after the first parse
	m_file->setParseResult( p.data() ); ///@todo beautify
	
	m_currentAccess = CodeModelItem::Public;
	m_inSlots = false;
	m_inSignals = false;
	m_inStorageSpec = false;
	m_inTypedef = false;
	m_currentDeclarator = 0;
	m_anon = 0;
	m_imports.clear();
	
	m_imports << QPair<QMap<QString, ClassDom>, QStringList>(QMap<QString, ClassDom>(), QStringList());
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
	
	int startLine, startColumn;
	int endLine, endColumn;
	ast->getStartPosition( &startLine, &startColumn );
	ast->getEndPosition( &endLine, &endColumn );

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
  QString nsName; 
  QString aliasName;
  
  if( !ast->namespaceName() || ast->namespaceName()->text().isEmpty() )
  {
        // anonymous namespace
  }
  else
    nsName = ast->namespaceName()->text();
  
  if( ast->aliasName() )
    aliasName = ast->aliasName()->text();

  
  if( !nsName.isNull() ) {
    NamespaceAliasModel m;
    m.setName( nsName );
    m.setAliasName( aliasName );
    m.setFileName( m_hashedFileName );
    if( m_currentNamespace.empty() )
      m_file->addNamespaceAlias( m );
    else
      m_currentNamespace.top() ->addNamespaceAlias( m );
  }
  
  TreeParser::parseNamespaceAlias( ast );
}

void StoreWalker::parseUsing( UsingAST* ast )
{
  TreeParser::parseUsing( ast );
}

void StoreWalker::parseUsingDirective( UsingDirectiveAST* ast )
{
  QString name;
  if( ast->name() )
    name = ast->name()->text();
  
  if( !name.isNull() ) {
    NamespaceImportModel m;
    m.setName( name );
    m.setFileName( m_hashedFileName );
    if( m_currentNamespace.empty() )
      m_file->addNamespaceImport( m );
    else
      m_currentNamespace.top() ->addNamespaceImport( m );
  }
  
  m_imports.back().second.push_back( name );
}

void StoreWalker::parseTypedef( TypedefAST* ast )
{
#if 0
	DeclaratorAST * oldDeclarator = m_currentDeclarator;
	
	if ( ast && ast->initDeclaratorList() && ast->initDeclaratorList() ->initDeclaratorList().count() > 0 )
	{
		QPtrList<InitDeclaratorAST> lst( ast->initDeclaratorList() ->initDeclaratorList() );
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
		
		QPtrList<InitDeclaratorAST> l( declarators->initDeclaratorList() );
		QPtrListIterator<InitDeclaratorAST> it( l );
		
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
			typeAlias->setComment( ast->comment() );
			
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
	m_currentTemplateDeclarator.push( ast );
	if ( ast->declaration() )
		parseDeclaration( ast->declaration() );
	
	
	
	TreeParser::parseTemplateDeclaration( ast );
	
	m_currentTemplateDeclarator.pop();
}

void StoreWalker::parseSimpleDeclaration( SimpleDeclarationAST* ast )
{
	TypeSpecifierAST * typeSpec = ast->typeSpec();
	InitDeclaratorListAST* declarators = ast->initDeclaratorList();
	CommentPusher push( *this, ast->comment() );
	
	if ( typeSpec )
		parseTypeSpecifier( typeSpec );
	
	if ( declarators )
	{
		QPtrList<InitDeclaratorAST> l = declarators->initDeclaratorList();
		
		QPtrListIterator<InitDeclaratorAST> it( l );
		while ( it.current() )
		{
			parseDeclaration( ast->functionSpecifier(), ast->storageSpecifier(),
			                  typeSpec, it.current() );
			++it;
		}
	}
}


QStringList StoreWalker::findScope( const QStringList& scope ) {
	ClassDom d = findClassFromScope( scope );
	
	if( d ) {
		QStringList ret = d->scope();
		ret << d->name();
		return ret;
	}
	
	return scope;
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
	ast->getStartPosition( &startLine, &startColumn );
	ast->getEndPosition( &endLine, &endColumn );
	
	QString id = d->declaratorId() ->unqualifiedName() ->text().stripWhiteSpace();
	
	QStringList scope = scopeOfDeclarator( d, m_currentScope );
	ClassDom c = findClassFromScope( scope );
	if( c ){
		scope = c->scope();
		scope << c->name();
	}
	
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
	if( !ast->comment().isEmpty() )
		method->setComment( ast->comment() );
	
	checkTemplateDeclarator( & (*method) );
	
	if ( m_inSignals )
		method->setSignal( true );
	
	if ( m_inSlots )
		method->setSlot( true );
	
	if( c && c->isClass() )
		method->setConstant( d->constant() != 0 );
	else if ( m_currentClass.top() || ( method->name() == "main" && scope.isEmpty() ) )
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

void StoreWalker::takeTemplateParams( TemplateModelItem& target, TemplateDeclarationAST* ast) {
	TemplateParameterListAST* pl = ast->templateParameterList();
	if( pl ) {
		QPtrList<TemplateParameterAST> list = pl->templateParameterList();
		
		TemplateParameterAST* curr = list.first();
		while( curr != 0 ) {
			QString a, b;
			if( curr->typeParameter() && curr->typeParameter()->name() ) {
				a = curr->typeParameter()->name()->text();
				if( curr->typeParameter()->typeId() ) 
					b =  curr->typeParameter()->typeId()->text();
			}
			
			target.addTemplateParam( a, b );
			CodeModelItem* cmi = dynamic_cast<CodeModelItem*>(&target);
			QString nm = "0";
			if(cmi) nm = cmi->name();
		kdDebug() << "item " << nm << " taking template-parameters " << a << ", default=" << b << "\n";
			curr = list.next();
		}
	}
}

void StoreWalker::checkTemplateDeclarator( TemplateModelItem* item ) {
	if( !m_currentTemplateDeclarator.empty() && m_currentTemplateDeclarator.top() != 0) {
		TemplateDeclarationAST* a = m_currentTemplateDeclarator.top();
		
		m_currentTemplateDeclarator.pop();
		m_currentTemplateDeclarator.push(0);
		
		takeTemplateParams( *item, a );
	}
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
	
	ClassDom klass = m_store->create<ClassModel>();
	klass->setStartPosition( startLine, startColumn );
	klass->setEndPosition( endLine, endColumn );
	klass->setFileName( m_fileName );
	
	int i = className.find( '<' );
	if( i != -1 ) {
		klass->setSpecializationDeclaration( className.mid( i ) );
		className = className.left( i );
	}
	
	klass->setName( className );
	klass->setComment( ast->comment() );
	
	checkTemplateDeclarator( &(*klass) );
	
	bool embed = !scopeOfName( ast->name(), QStringList() ).isEmpty();
	
	QStringList oldScope;
	
	
	if( embed ) {
		ClassDom embedderClass = findClassFromScope( m_currentScope + scopeOfName( ast->name(), QStringList() ));
		
		if(embedderClass) {
			if(embedderClass->fileName() != klass->fileName()) {
				///since we are creating a link between both files, put them into the same parsing-group
				FileDom dm = embedderClass->file();
				if( dm ) {
					m_file->setGroupId( m_store->mergeGroups( dm->groupId(), m_file->groupId() ) );
				}else{
					kdDebug() << "file " << embedderClass->fileName() << " missing in store \n";
				}
			}
				
			oldScope = m_currentScope;
			m_currentScope = embedderClass->scope();
			m_currentScope.push_back( embedderClass->name() );
			m_currentClass.push( embedderClass );
			
			//m_file->addClass( klass );//experiment
		}else{
			kdDebug( 9007 ) << "could not find embedding class " << QStringList(m_currentScope + scopeOfName( ast->name(), QStringList() )).join("::") << " for " << className << endl;
			embed = false;
		}
	}
	
	if ( m_currentClass.top() )
		m_currentClass.top() ->addClass( klass );
	else if ( m_currentNamespace.top() )
		m_currentNamespace.top() ->addClass( klass );
	else
		m_file->addClass( klass );
	
	klass->setScope( m_currentScope );
	
	
	if ( ast->baseClause() )
		parseBaseClause( ast->baseClause(), klass );
	
	m_currentScope.push_back( className );
	m_currentClass.push( klass );
	
	//m_imports.push_back( QStringList() );
	
	TreeParser::parseClassSpecifier( ast );
	

	//m_imports.pop_back();
	m_currentClass.pop();
	
	m_currentScope.pop_back();
	
	if( embed ) {
		m_currentScope = oldScope;
		m_currentClass.pop();
	}
	
	m_currentAccess = oldAccess;
	m_inSlots = oldInSlots;
	m_inSignals = oldInSignals;
}

void StoreWalker::parseEnumSpecifier( EnumSpecifierAST* ast )
{
	if( ast->name() ) {
	TypeAliasDom typeAlias = m_store->create<TypeAliasModel>();
	typeAlias->setFileName( m_fileName );
	typeAlias->setName( ast->name()->text() );
	typeAlias->setType( "const int" );
	typeAlias->setComment( ast->comment() );
	
	int line, col;
	ast->getStartPosition( &line, &col );
	typeAlias->setStartPosition( line, col );
	
	ast->getEndPosition( &line, &col );
	typeAlias->setEndPosition( line, col );
	
	if ( m_currentClass.top() )
		m_currentClass.top() ->addTypeAlias( typeAlias );
	else if ( m_currentNamespace.top() )
		m_currentNamespace.top() ->addTypeAlias( typeAlias );
	else
		m_file->addTypeAlias( typeAlias );
	}

	QPtrList<EnumeratorAST> l = ast->enumeratorList();
	QPtrListIterator<EnumeratorAST> it( l );
	while ( it.current() )
	{
		VariableDom attr = m_store->create<VariableModel>();
		attr->setName( it.current() ->id() ->text() );
		attr->setFileName( m_fileName );
		attr->setAccess( m_currentAccess );
		
		if( !ast->name() ) {
			attr->setType( "const int" );
		} else {
			attr->setType( ast->name()->text() );
		}

		attr->setEnumeratorVariable( true );

		attr->setComment( (*it)->comment() );
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
		kdDebug( 9007 ) << "skip declaration of " << QStringList(scopeOfDeclarator( d, QStringList() )).join("::") << "::" << id << endl;
		return ;
	}
	
	VariableDom attr = m_store->create<VariableModel>();
	attr->setName( id );
	attr->setFileName( m_fileName );
	attr->setComment( comment() );
	
	
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
	
	attr->setStartPosition( startLine, startColumn );
	attr->setEndPosition( endLine, endColumn );
	attr->setStatic( isStatic );
}

void StoreWalker::parseAccessDeclaration( AccessDeclarationAST * access )
{
	QPtrList<AST> l = access->accessList();
	
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
	ns->setComment( ast->comment() );
	
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
	
	FunctionDom method = m_store->create<FunctionModel>();
	method->setName( id );
	
	method->setComment( comment() );
	method->setFileName( m_fileName );
	method->setStartPosition( startLine, startColumn );
	method->setEndPosition( endLine, endColumn );
	method->setAccess( m_currentAccess );
	method->setStatic( isStatic );
	method->setVirtual( isVirtual );
	method->setAbstract( isPure );
	parseFunctionArguments( d, method );
	
	checkTemplateDeclarator( & (*method) );
		
	
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
		QPtrList<ParameterDeclarationAST> l( params->parameterList() );
		QPtrListIterator<ParameterDeclarationAST> it( l );
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
	
	QPtrList<AST> ptrOpList = declarator->ptrOpList();
	for ( QPtrListIterator<AST> it( ptrOpList ); it.current(); ++it )
	{
		text += it.current() ->text();
	}
	
	return text;
}

void StoreWalker::parseBaseClause( BaseClauseAST * baseClause, ClassDom klass )
{
	QPtrList<BaseSpecifierAST> l = baseClause->baseSpecifierList();
	QPtrListIterator<BaseSpecifierAST> it( l );
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
		QPtrList<ClassOrNamespaceNameAST> l = id->classOrNamespaceNameList();
		QPtrListIterator<ClassOrNamespaceNameAST> it( l );
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


ClassDom StoreWalker::findClassFromScope( const QStringList& scope ) 
{
	QString scopeText = scope.join("::");
	if( !m_imports.isEmpty() ) {
		QMapIterator<QString, ClassDom> it = m_imports.back().first.find( scopeText );
		if( it != m_imports.back().first.end() ) {
			return *it;
		}
	}
	
	ClassDom c = classFromScope( scope );
	if( c ) {
		if( !m_imports.isEmpty() ) m_imports.back().first[ scopeText ] = c;
		return c;
	}
	
	if(!m_imports.isEmpty() && !m_imports.back().second.isEmpty()) {
			///try the same using one of the imports(performance-wise this is not good, but simple)
		
		QStringList::iterator it = m_imports.back().second.begin();
		while(it != m_imports.back().second.end()) {
			QStringList scp = QStringList::split("::", *it) + m_currentScope + scope;
			c = classFromScope( scp );
			if( c ) {
				if( !m_imports.isEmpty() ) m_imports.back().first[ scopeText ] = c;
				return c;
			}
			++it;
		}
	}
	return c;
}

ClassDom StoreWalker::classFromScope(const QStringList& scope) {
	if(scope.isEmpty())return ClassDom(0);
	
	NamespaceDom glob = m_store->globalNamespace();
	if( !glob ) return ClassDom();
	
	ClassModel* curr =  glob ;
	
	QStringList::const_iterator mit = scope.begin();
	
	while(curr->isNamespace() && mit != scope.end() && ((NamespaceModel*)curr)->hasNamespace( *mit )) {
		curr = &(*( ((NamespaceModel*)curr)->namespaceByName( *mit ) ));
		++mit;
	}
	
	while((curr->isNamespace() || curr->isClass()) && mit != scope.end() && curr->hasClass( *mit )) {
		ClassList cl = curr->classByName( *mit );
		curr = &(**cl.begin() );
		++mit;
	}
	
	if(mit == scope.end()) {
		return curr;
	}
	
	return ClassDom(0);
}


QStringList StoreWalker::scopeOfDeclarator( DeclaratorAST* d, const QStringList& startScope )
{
	return scopeOfName( d->declaratorId(), startScope );
}

//kate: indent-mode csands; tab-width 4; space-indent off;
