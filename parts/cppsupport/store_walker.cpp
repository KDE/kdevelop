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

#include <kdebug.h>
#include <qfileinfo.h>
#include <qdir.h>

StoreWalker::StoreWalker( const QString& fileName, CodeModel* store )
    : m_store( store ), m_anon( 0 )
{
    m_fileName = QDir( fileName ).canonicalPath();

    //kdDebug(9007) << "StoreWalker::StoreWalker(" << m_fileName << ")" << endl;
}

StoreWalker::~StoreWalker()
{
}

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
    m_anon = 0;
    m_imports.clear();

    m_imports << QStringList();
    TreeParser::parseTranslationUnit( ast );
    m_imports.pop_back();

    m_store->addFile( m_file );
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
    if( !m_currentClass.isEmpty() ){
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
    ClassDom klass = findContainer( scopeStr, 0, true );

    FunctionDom method = m_store->create<FunctionModel>();
    method->setName( id );

    parseFunctionArguments( d, method );

    if( klass || m_currentClass.top() ){
	method->setConstant( d->constant() != 0 );
	method->setAccess( m_currentAccess );
	method->setStatic( isStatic );
	method->setVirtual( isVirtual );
    }

    QString text = typeOfDeclaration( typeSpec, d );
    if( !text.isEmpty() )
	method->setResultType( text );

    method->setFileName( m_fileName );
    method->setStartPosition( startLine, startColumn );
    method->setEndPosition( endLine, endColumn );

    method->setImplementedInFile( m_fileName );
    method->setImplementationStartPosition( startLine, startColumn );
    method->setImplementationEndPosition( endLine, endColumn );

    if( m_inSignals )
        method->setSignal( true );

    if( m_inSlots )
        method->setSlot( true );

    if( m_currentClass.top() )
	m_currentClass.top()->addFunction( method );
    else if( klass ){
	FunctionList lst;
	findFunctions( toString(method), klass, lst );
	if( lst.size() ){
	    FunctionDom f = lst[ 0 ];
	    f->setImplementedInFile( m_fileName );
	    f->setImplementationStartPosition( startLine, startColumn );
	    f->setImplementationEndPosition( endLine, endColumn );
	}
    } else if( scopeStr.isEmpty() )
	m_file->addFunction( method );

    /// FIXME: add function definition
#if 0
    else if( m_currentNamespace.top() )
	m_currentNamespace.top()->addFunction( method );
    else
	m_file->addFunction( method );
#endif
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

    QString kind = ast->classKey()->text();
    if( kind == "class" )
	m_currentAccess = CodeModelItem::Private;
    else
	m_currentAccess = CodeModelItem::Public;
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

    QString scopeStr = scopeOfName( ast->name() );
    ClassDom cl = findContainer( scopeStr, 0, true );

    ClassDom klass = m_store->create<ClassModel>();
    klass->setStartPosition( startLine, startColumn );
    klass->setEndPosition( endLine, endColumn );
    klass->setFileName( m_fileName );

    klass->setName( className );

    if( cl )
	cl->addClass( klass );
    else if( m_currentClass.top() )
	m_currentClass.top()->addClass( klass );
    else if( m_currentNamespace.top() )
	m_currentNamespace.top()->addClass( klass );
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
#if 0
    QPtrList<EnumeratorAST> l = ast->enumeratorList();
    QPtrListIterator<EnumeratorAST> it( l );
    while( it.current() ){
	VariableDom attr = findOrInsertAttribute( it.current(), m_currentContainer, it.current()->id()->text() );
	attr->setAccess( m_currentAccess );
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
#endif
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
    ClassDom cl = findContainer( scopeStr, 0, true ); 	//FIXME: ROBE!!

    if( !scopeStr.isEmpty() && cl && !cl->isNamespace() )
        return;

    VariableDom attr = m_store->create<VariableModel>();
    attr->setName( id );
    attr->setFileName( m_fileName );

    if( m_currentClass.top() )
	m_currentClass.top()->addVariable( attr );
    else if( cl )        // FIXME: ROBE!!!
	cl->addVariable( attr );
    else if( m_currentNamespace.top() )
	m_currentNamespace.top()->addVariable( attr );
    else
	m_file->addVariable( attr );

    attr->setAccess( m_currentAccess );

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

    attr->setStartPosition( startLine, startColumn );
    attr->setEndPosition( endLine, endColumn );
    attr->setStatic( true );
}

void StoreWalker::parseAccessDeclaration( AccessDeclarationAST * access )
{
    QPtrList<AST> l = access->accessList();

    QString accessStr = l.at( 0 )->text();
    if( accessStr == "public" )
	m_currentAccess = CodeModelItem::Public;
    else if( accessStr == "protected" )
	m_currentAccess = CodeModelItem::Protected;
    else if( accessStr == "private" )
	m_currentAccess = CodeModelItem::Private;
    else if( accessStr == "signals" )
	m_currentAccess = CodeModelItem::Protected;
    else
	m_currentAccess = CodeModelItem::Public;

    m_inSlots = l.count() > 1 ? l.at( 1 )->text() == "slots" : false;
    m_inSignals = l.count() > 1 ? l.at( 0 )->text() == "signals" : false;
}

NamespaceDom StoreWalker::findOrInsertNamespace( NamespaceAST* ast, const QString & name )
{
    //kdDebug(9007) << "-----------------> findOrInsert" << name << " found!!" << endl;

    if( m_currentNamespace.top() && m_currentNamespace.top()->hasNamespace(name) )
	return m_currentNamespace.top()->namespaceByName( name );

    if( m_file->hasNamespace(name) )
        return m_file->namespaceByName( name );

    int startLine, startColumn;
    int endLine, endColumn;
    ast->getStartPosition( &startLine, &startColumn );
    ast->getEndPosition( &endLine, &endColumn );

    NamespaceDom ns = m_store->create<NamespaceModel>();
    ns->setName( name );
    ns->setStartPosition( startLine, startColumn );
    ns->setEndPosition( endLine, endColumn );

    if( m_currentNamespace.top() )
	m_currentNamespace.top()->addNamespace( ns );
    else
	m_file->addNamespace( ns );

    return ns;
}

VariableDom StoreWalker::findOrInsertAttribute( AST* ast, ClassDom scope, const QString & name )
{
    if( !scope->hasVariable(name) ){
	VariableDom attr = m_store->create<VariableModel>();
	attr->setName( name );
	attr->setAccess( m_currentAccess );

	int startLine, startColumn;
	int endLine, endColumn;

	ast->getStartPosition( &startLine, &startColumn );
	ast->getEndPosition( &endLine, &endColumn );

	attr->setStartPosition( startLine, startColumn );
	attr->setEndPosition( endLine, endColumn );
	attr->setFileName( m_fileName );

	scope->addVariable( attr );
	return attr;
    }

    return scope->variableByName( name );
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

    if( m_inSignals )
        method->setSignal( true );

    if( m_inSlots )
        method->setSlot( true );

    if( m_currentClass.top() ){

	QString text = typeOfDeclaration( typeSpec, d );
	if( !text.isEmpty() )
	    method->setResultType( text );
    } else {
	QString text = typeOfDeclaration( typeSpec, d );
	if( !text.isEmpty() )
	    method->setResultType( text );
    }
    method->setConstant( d->constant() != 0 );

    QString key = m_currentScope.join(".") + " " + toString( method );
    if( m_implementationMap.contains(key) ){
	//kdDebug(9007) << "---------> found an implementation for: " << key << endl;
	ImplementationInfo impl = m_implementationMap[ key ];
	method->setImplementedInFile( impl.fileName );
	method->setImplementationStartPosition( impl.startLine, impl.startColumn );
	method->setImplementationEndPosition( impl.endLine, impl.endColumn );
    } else {
	//kdDebug(9007) << "---------> no implementation for: " << key << endl;
    }

    if( m_currentClass.top() )
	m_currentClass.top()->addFunction( method );
    else if( m_currentNamespace.top() )
	m_currentNamespace.top()->addFunction( method );
    else
	m_file->addFunction( method );
}

void StoreWalker::parseFunctionArguments( DeclaratorAST* declarator, FunctionDom method )
{
    ParameterDeclarationClauseAST* clause = declarator->parameterDeclarationClause();

    if( clause && clause->parameterDeclarationList() ){
        ParameterDeclarationListAST* params = clause->parameterDeclarationList();
	QPtrList<ParameterDeclarationAST> l( params->parameterList() );
	QPtrListIterator<ParameterDeclarationAST> it( l );
	while( it.current() ){
	    ParameterDeclarationAST* param = it.current();
	    ++it;

	    ArgumentDom arg = m_store->create<ArgumentModel>();

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

void StoreWalker::parseBaseClause( BaseClauseAST * baseClause, ClassDom klass )
{
    QPtrList<BaseSpecifierAST> l = baseClause->baseSpecifierList();
    QPtrListIterator<BaseSpecifierAST> it( l );
    while( it.current() ){
	BaseSpecifierAST* baseSpecifier = it.current();

	QString baseName;
	if( baseSpecifier->name() )
	    baseName = baseSpecifier->name()->text();

	klass->addBaseClass( baseName );

	++it;
    }
}

ClassDom StoreWalker::findContainer( const QString& name, NamespaceDom container, bool includeImports )
{
    if( name.isEmpty() )
	return model_cast<ClassDom>( container );

    if( !container ){
	return findContainer( name, m_store->globalNamespace(), includeImports );
    }

    QStringList path = QStringList::split( ".", name );
    QStringList::Iterator it = path.begin();
    while( it != path.end() ){
        QString s = *it;
        ++it;

	if( !container->hasNamespace(s) )
	    break;

        NamespaceDom scope = container->namespaceByName( s );

        path.remove( s );
        container = scope;
    }

    if( path.size() == 0 )
        return model_cast<ClassDom>( container );

    QString className = path.join( "." );

    ClassDom c = model_cast<ClassDom>( container );
    while( c && path.size() ){
	QString s = path.front();
	path.pop_front();

	if( !c->hasClass(s) ){
	    c = 0;
	    break;
	}

	ClassList classList = c->classByName( s );
	c = classList[ 0 ];
	for( ClassList::Iterator cit=classList.begin(); cit!=classList.end(); ++cit ){
	    if( QFileInfo( (*cit)->fileName() ).dirPath(true) == QFileInfo( m_fileName ).dirPath(true) )
		c = *cit;
	}
   }

    if( !c && includeImports ){

        QStringList imports;
        QValueList<QStringList>::Iterator lIt = m_imports.begin();
        while( lIt != m_imports.end() ){
           imports += (*lIt );
           ++lIt;
        }

        QStringList::Iterator impIt = imports.begin();
        while( impIt != imports.end() ){
            ClassDom kl = findContainer( (*impIt) + "." + name, container, false );
            if( kl )
                return kl;
            ++impIt;
        }
    }

    return c;
}

QString StoreWalker::scopeOfName( NameAST* id )
{
    QStringList scope = m_currentScope;
    if( id && id->classOrNamespaceNameList().count() ){
        if( id->isGlobal() )
	    scope.clear();
	QPtrList<ClassOrNamespaceNameAST> l = id->classOrNamespaceNameList();
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

QString StoreWalker::scopeOfDeclarator( DeclaratorAST* d )
{
    return scopeOfName( d->declaratorId() );
}

QString StoreWalker::toString( const FunctionDom& fun )
{
    QString text = fun->name();
    text += "(";
    const ArgumentList args = fun->argumentList();
    ArgumentList::ConstIterator it = args.begin();
    while( it != args.end() ){
	const ArgumentDom& arg = *it;
	++it;

	text += arg->type();

	if( it != args.end() )
	    text += ",";
    }
    text += ")";
    if( fun->isConstant() )
	text += " const";
    if( fun->isAbstract() )
	text += "=0";

    return text;
}

void StoreWalker::findFunctions( const QString & proto, const NamespaceDom& ns, FunctionList & lst )
{
    //findFunctions( proto, ns->namespaceList(), lst );
    //findFunctions( proto, ns->classList(), lst );
    findFunctions( proto, ns->functionList(), lst );
}

void StoreWalker::findFunctions( const QString & proto, const NamespaceList& namespaceList, FunctionList & lst )
{
    for( NamespaceList::ConstIterator it=namespaceList.begin(); it!=namespaceList.end(); ++it )
	findFunctions( proto, *it, lst );
}

void StoreWalker::findFunctions( const QString & proto, const ClassList& classList, FunctionList & lst )
{
    for( ClassList::ConstIterator it=classList.begin(); it!=classList.end(); ++it )
	findFunctions( proto, *it, lst );
}

void StoreWalker::findFunctions( const QString & proto, const FunctionList& functionList, FunctionList & lst )
{
    for( FunctionList::ConstIterator it=functionList.begin(); it!=functionList.end(); ++it )
	findFunctions( proto, *it, lst );
}

void StoreWalker::findFunctions( const QString & proto, const ClassDom& klass, FunctionList & lst )
{
    findFunctions( proto, klass->classList(), lst );
    findFunctions( proto, klass->functionList(), lst );
}

void StoreWalker::findFunctions( const QString & proto, const FunctionDom& fun, FunctionList & lst )
{
    if( toString(fun) == proto )
	lst << fun;
}

void StoreWalker::findFunctions( const QString & proto, FunctionList & lst )
{
    FileList fileList = m_store->fileList();
    for( FileList::Iterator it=fileList.begin(); it!=fileList.end(); ++it )
	findFunctions( proto, model_cast<NamespaceDom>(*it), lst );
}

void StoreWalker::buildImplementationMap( FileDom file, QStringList& scope, QMap< QString, ImplementationInfo > & map )
{
    buildImplementationMap( file->namespaceList(), scope, map );
    buildImplementationMap( file->classList(), scope, map );
    buildImplementationMap( file->functionList(), scope, map );
    buildImplementationMap( file->variableList(), scope, map );
}

void StoreWalker::buildImplementationMap( NamespaceDom ns, QStringList& scope, QMap< QString, ImplementationInfo > & map )
{
    scope.push_back( ns->name() );

    buildImplementationMap( ns->namespaceList(), scope, map );
    buildImplementationMap( ns->classList(), scope, map );
    buildImplementationMap( ns->functionList(), scope, map );
    buildImplementationMap( ns->variableList(), scope, map );

    scope.pop_back();
}

void StoreWalker::buildImplementationMap( ClassDom klass, QStringList& scope, QMap< QString, ImplementationInfo > & map )
{
    scope.push_back( klass->name() );

    buildImplementationMap( klass->classList(), scope, map );
    buildImplementationMap( klass->functionList(), scope, map );
    buildImplementationMap( klass->variableList(), scope, map );

    scope.pop_back();
}

void StoreWalker::buildImplementationMap( FunctionDom fun, QStringList& scope, QMap< QString, ImplementationInfo > & map )
{
    if( fun->hasImplementation() && fun->implementedInFile() != m_fileName ){
	ImplementationInfo impl;
	impl.fileName = fun->implementedInFile();
	fun->getImplementationStartPosition( &impl.startLine, &impl.startColumn );
	fun->getImplementationEndPosition( &impl.endLine, &impl.endColumn );
	map.insert( scope.join(".") + " " + toString(fun), impl );
    }
}

void StoreWalker::buildImplementationMap( VariableDom var, QStringList& scope, QMap< QString, ImplementationInfo > & map )
{
}

void StoreWalker::buildImplementationMap( const NamespaceList & namespaceList, QStringList& scope, QMap< QString, ImplementationInfo > & map )
{
    for( NamespaceList::ConstIterator it=namespaceList.begin(); it!=namespaceList.end(); ++it )
	buildImplementationMap( *it, scope, map );
}

void StoreWalker::buildImplementationMap( const ClassList & classList, QStringList& scope, QMap< QString, ImplementationInfo > & map )
{
    for( ClassList::ConstIterator it=classList.begin(); it!=classList.end(); ++it )
	buildImplementationMap( *it, scope, map );
}

void StoreWalker::buildImplementationMap( const FunctionList & functionList, QStringList& scope, QMap< QString, ImplementationInfo > & map )
{
    for( FunctionList::ConstIterator it=functionList.begin(); it!=functionList.end(); ++it )
	buildImplementationMap( *it, scope, map );
}

void StoreWalker::buildImplementationMap( const VariableList & variableList, QStringList& scope, QMap< QString, ImplementationInfo > & map )
{
}

void StoreWalker::buildImplementationMap( FileDom file )
{
    //kdDebug(9007) << "------------> StoreWalker::buildImplementationMap()" << endl;
    QStringList scope;
    m_implementationMap.clear();
    buildImplementationMap( file, scope, m_implementationMap );
}
