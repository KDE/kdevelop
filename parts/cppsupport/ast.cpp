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

#include "ast.h"
#include <kdebug.h>

QString nodeTypeToString( NodeType type )
{
    switch( type )
    {
    case NodeType_Generic:
	return "Generic";
    case NodeType_TemplateArgumentList:
	return "TemplateArgumentList";
    case NodeType_ClassOrNamespaceName:
	return "ClassOrNamespaceName";
    case NodeType_NestedNameSpecifier:
	return "NestedNameSpecifier";
    case NodeType_Name:
	return "Name";
    case NodeType_Declaration:
	return "Declaration";
    case NodeType_TypeSpecifier:
	return "TypeSpecifier";
    case NodeType_BaseSpecifier:
	return "BaseSpecifier";
    case NodeType_BaseClause:
	return "BaseClause";
    case NodeType_ClassSpecifier:
	return "ClassSpecifier";
    case NodeType_Enumerator:
	return "Enumerator";
    case NodeType_EnumSpecifier:
	return "EnumSpecifier";
    case NodeType_ElaboratedTypeSpecifier:
	return "ElaboratedTypeSpecifier";
    case NodeType_LinkageBody:
	return "LinkageBody";
    case NodeType_LinkageSpecification:
	return "LinkageSpecification";
    case NodeType_Namespace:
	return "Namespace";
    case NodeType_NamespaceAlias:
	return "NamespaceAlias";
    case NodeType_Using:
	return "Using";
    case NodeType_UsingDirective:
	return "UsingDirective";
    case NodeType_InitDeclaratorList:
	return "InitDeclaratorList";
    case NodeType_Typedef:
	return "Typedef";
    case NodeType_Declarator:
	return "Declarator";
    case NodeType_InitDeclarator:
	return "InitDeclarator";
    case NodeType_TemplateDeclaration:
	return "TemplateDeclaration";
    case NodeType_SimpleDeclaration:
	return "SimpleDeclaration";
    case NodeType_Statement:
	return "Statement";
    case NodeType_TranslationUnit:
	return "TranslationUnit";
    case NodeType_Custom:	
	return "Custom";
    }
    
    return QString::null;
}


// ------------------------------------------------------------------------
AST::AST()
    : m_nodeType( NodeType_Generic ), m_parent( 0 ),
      m_startLine( 0 ), m_startColumn( 0 ),
      m_endLine( 0 ), m_endColumn( 0 )
{
    m_children.setAutoDelete( false );
}

AST::~AST()
{
    if( m_parent )
	m_parent->removeChild( this );
}

void AST::setStartPosition( int line, int col )
{
   m_startLine = line;
   m_startColumn = col;
}

void AST::getStartPosition( int* line, int* col ) const
{
    if( line )
        *line = m_startLine;

    if( col )
        * col = m_startColumn;
}

void AST::setEndPosition( int line, int col )
{
   m_endLine = line;
   m_endColumn = col;
}

void AST::getEndPosition( int* line, int* col ) const
{
    if( line )
        *line = m_endLine;

    if( col )
        * col = m_endColumn;
}

void AST::setParent( AST* parent )
{
    if( m_parent )
	m_parent->removeChild( this );

    m_parent = parent;
    if( m_parent )
	m_parent->appendChild( this );
}

void AST::appendChild( AST* child )
{
    m_children.append( child );
}

void AST::removeChild( AST* child )
{
    m_children.remove( child );
}

// ------------------------------------------------------------------------
NameAST::NameAST()
    : m_global( false )
{
}

NameAST::~NameAST()
{
}

bool NameAST::isGlobal() const
{
    return m_global;
}

void NameAST::setGlobal( bool b )
{
    m_global = b;
}

void NameAST::setNestedName( NestedNameSpecifierAST::Node& nestedName )
{
    m_nestedName = nestedName;
    if( m_nestedName.get() ) m_nestedName->setParent( this );
}

NestedNameSpecifierAST* NameAST::nestedName()
{
    return m_nestedName.get();
}

void NameAST::setUnqualifedName( AST::Node& unqualifiedName )
{
    m_unqualifiedName = unqualifiedName;
    if( m_unqualifiedName.get() ) m_unqualifiedName->setParent( this );
}

AST* NameAST::unqualifiedName()
{
    return m_unqualifiedName.get();
}

// ------------------------------------------------------------------------
DeclarationAST::DeclarationAST()
{
}

DeclarationAST::~DeclarationAST()
{
}

// ------------------------------------------------------------------------
LinkageBodyAST::LinkageBodyAST()
{
    m_declarations.setAutoDelete( true );
}

LinkageBodyAST::~LinkageBodyAST()
{
}

void LinkageBodyAST::addDeclaration( DeclarationAST::Node& ast )
{
    if( !ast.get() )
        return;

    ast->setParent( this );
    m_declarations.append( ast.release() );
}

// ------------------------------------------------------------------------
LinkageSpecificationAST::LinkageSpecificationAST()
{
}

LinkageSpecificationAST::~LinkageSpecificationAST()
{
}

QString LinkageSpecificationAST::externType() const
{
    return m_externType;
}

void LinkageSpecificationAST::setExternType( const QString& type )
{
    m_externType = type;
}

LinkageBodyAST* LinkageSpecificationAST::linkageBody()
{
    return m_linkageBody.get();
}

void LinkageSpecificationAST::setLinkageBody( LinkageBodyAST::Node& linkageBody )
{
    m_linkageBody = linkageBody;
    if( m_linkageBody.get() ) m_linkageBody->setParent( this );
}

DeclarationAST* LinkageSpecificationAST::declaration()
{
    return m_declaration.get();
}

void LinkageSpecificationAST::setDeclaration( DeclarationAST::Node& decl )
{
    m_declaration = decl;
    if( m_declaration.get() ) m_declaration->setParent( this );
}

// ------------------------------------------------------------------------
TranslationUnitAST::TranslationUnitAST()
{
    m_declarations.setAutoDelete( true );
}

TranslationUnitAST::~TranslationUnitAST()
{
}

void TranslationUnitAST::addDeclaration( DeclarationAST::Node& ast )
{
    if( !ast.get() )
        return;

    ast->setParent( this );
    m_declarations.append( ast.release() );
}

// ------------------------------------------------------------------------
NamespaceAST::NamespaceAST()
{
}

NamespaceAST::~NamespaceAST()
{
}

QString NamespaceAST::namespaceName() const
{
    return m_namespaceName;
}

void NamespaceAST::setNamespaceName( const QString& name )
{
    m_namespaceName = name;
}

LinkageBodyAST* NamespaceAST::linkageBody()
{
    return m_linkageBody.get();
}

void NamespaceAST::setLinkageBody( LinkageBodyAST::Node& linkageBody )
{
    m_linkageBody = linkageBody;
    if( m_linkageBody.get() ) m_linkageBody->setParent( this );
}


// ------------------------------------------------------------------------
NamespaceAliasAST::NamespaceAliasAST()
{
}

NamespaceAliasAST::~NamespaceAliasAST()
{
}

QString NamespaceAliasAST::namespaceName() const
{
    return m_namespaceName;
}

void NamespaceAliasAST::setNamespaceName( const QString& name )
{
    m_namespaceName = name;
}

NameAST* NamespaceAliasAST::aliasName()
{
    return m_aliasName.get();
}

void NamespaceAliasAST::setAliasName( NameAST::Node& name )
{
    m_aliasName = name;
    if( m_aliasName.get() ) m_aliasName->setParent( this );
}

// ------------------------------------------------------------------------
UsingAST::UsingAST()
    : m_typename( false )
{
}

UsingAST::~UsingAST()
{
}

bool UsingAST::isTypename() const
{
    return m_typename;
}

void UsingAST::setTypename( bool b )
{
    m_typename = b;
}

NameAST* UsingAST::name()
{
    return m_name.get();
}

void UsingAST::setName( NameAST::Node& name )
{
    m_name = name;
    if( m_name.get() ) m_name->setParent( this );
}

// ------------------------------------------------------------------------
UsingDirectiveAST::UsingDirectiveAST()
{
}

UsingDirectiveAST::~UsingDirectiveAST()
{
}

NameAST* UsingDirectiveAST::name()
{
    return m_name.get();
}

void UsingDirectiveAST::setName( NameAST::Node& name )
{
    m_name = name;
    if( m_name.get() ) m_name->setParent( this );
}

TypedefAST::TypedefAST()
{
}

TypedefAST::~TypedefAST()
{
}

TypeSpecifierAST* TypedefAST::typeSpec()
{
    return m_typeSpec.get();
}

void TypedefAST::setTypeSpec( TypeSpecifierAST::Node& typeSpec )
{
    m_typeSpec = typeSpec;
    if( m_typeSpec.get() ) m_typeSpec->setParent( this );
}

InitDeclaratorListAST* TypedefAST::initDeclaratorList()
{
    return m_initDeclaratorList.get();
}

void TypedefAST::setInitDeclaratorList( InitDeclaratorListAST::Node& initDeclaratorList )
{
    m_initDeclaratorList = initDeclaratorList;
    if( m_initDeclaratorList.get() ) m_initDeclaratorList->setParent( this );
}

// ------------------------------------------------------------------------
TemplateArgumentListAST::TemplateArgumentListAST()
{
}

TemplateArgumentListAST::~TemplateArgumentListAST()
{
}

void TemplateArgumentListAST::addArgument( AST::Node& arg )
{
    if( !arg.get() )
        return;

    arg->setParent( this );
    m_arguments.append( arg.release() );
}


// ------------------------------------------------------------------------
TemplateDeclarationAST::TemplateDeclarationAST()
    : m_export( false )
{
}

TemplateDeclarationAST::~TemplateDeclarationAST()
{
}

bool TemplateDeclarationAST::isExport() const
{
    return m_export;
}

void TemplateDeclarationAST::setExport( bool b )
{
    m_export = b;
}

AST* TemplateDeclarationAST::templateParameterList()
{
    return m_templateParameterList.get();
}

void TemplateDeclarationAST::setTemplateParameterList( AST::Node& templateParameterList )
{
    m_templateParameterList = templateParameterList;
    if( m_templateParameterList.get() ) m_templateParameterList->setParent( this );
}

DeclarationAST* TemplateDeclarationAST::declaration()
{
    return m_declaration.get();
}

void TemplateDeclarationAST::setDeclaration( DeclarationAST::Node& declaration )
{
    m_declaration = declaration;
    if( m_declaration.get() ) m_declaration->setParent( this );
}

// ------------------------------------------------------------------------
ClassOrNamespaceNameAST::ClassOrNamespaceNameAST()
{
}

ClassOrNamespaceNameAST::~ClassOrNamespaceNameAST()
{
}

AST* ClassOrNamespaceNameAST::name()
{
    return m_name.get();
}

void ClassOrNamespaceNameAST::setName( AST::Node& name )
{
    m_name = name;
    if( m_name.get() ) m_name->setParent( this );
}

TemplateArgumentListAST* ClassOrNamespaceNameAST::templateArgumentList()
{
    return m_templateArgumentList.get();
}

void ClassOrNamespaceNameAST::setTemplateArgumentList( TemplateArgumentListAST::Node& templateArgumentList )
{
    m_templateArgumentList = templateArgumentList;
    if( m_templateArgumentList.get() ) m_templateArgumentList->setParent( this );
}

// ------------------------------------------------------------------------
NestedNameSpecifierAST::NestedNameSpecifierAST()
{
    m_classOrNamespaceNameList.setAutoDelete( true );
}

NestedNameSpecifierAST::~NestedNameSpecifierAST()
{
}

void NestedNameSpecifierAST::addClassOrNamespaceName( ClassOrNamespaceNameAST::Node& classOrNamespaceName )
{
    if( !classOrNamespaceName.get() )
        return;

    classOrNamespaceName->setParent( this );
    m_classOrNamespaceNameList.append( classOrNamespaceName.release() );
}


// ------------------------------------------------------------------------
TypeSpecifierAST::TypeSpecifierAST()
{
}

TypeSpecifierAST::~TypeSpecifierAST()
{
}


// ------------------------------------------------------------------------
ClassSpecifierAST::ClassSpecifierAST()
{
    m_declarations.setAutoDelete( true );
}

ClassSpecifierAST::~ClassSpecifierAST()
{
}

AST* ClassSpecifierAST::classKey()
{
    return m_classKey.get();
}

void ClassSpecifierAST::setClassKey( AST::Node& classKey )
{
    m_classKey = classKey;
    if( m_classKey.get() ) m_classKey->setParent( this );
}

void ClassSpecifierAST::addDeclaration( DeclarationAST::Node& declaration )
{
    if( !declaration.get() )
        return;

    declaration->setParent( this );
    m_declarations.append( declaration.release() );
}

// ------------------------------------------------------------------------
EnumSpecifierAST::EnumSpecifierAST()
{
    m_enumerators.setAutoDelete( true );
}

EnumSpecifierAST::~EnumSpecifierAST()
{
}

void EnumSpecifierAST::addEnumerator( EnumeratorAST::Node& enumerator )
{
    if( !enumerator.get() )
        return;

    enumerator->setParent( this );
    m_enumerators.append( enumerator.release() );
}


// ------------------------------------------------------------------------
ElaboratedTypeSpecifierAST::ElaboratedTypeSpecifierAST()
{
}

ElaboratedTypeSpecifierAST::~ElaboratedTypeSpecifierAST()
{
}

AST* ElaboratedTypeSpecifierAST::kind()
{
    return m_kind.get();
}

void ElaboratedTypeSpecifierAST::setKind( AST::Node& kind )
{
    m_kind = kind;
    if( m_kind.get() ) m_kind->setParent( this );
}

// ------------------------------------------------------------------------
StatementAST::StatementAST()
{
}

StatementAST::~StatementAST()
{
}

// ------------------------------------------------------------------------
EnumeratorAST::EnumeratorAST()
{
}

EnumeratorAST::~EnumeratorAST()
{
}

AST* EnumeratorAST::id()
{
    return m_id.get();
}

void EnumeratorAST::setId( AST::Node& id )
{
    m_id = id;
    if( m_id.get() ) m_id->setParent( this );
}

AST* EnumeratorAST::expr()
{
    return m_expr.get();
}

void EnumeratorAST::setExpr( AST::Node& expr )
{
    m_expr = expr;
    if( m_expr.get() ) m_expr->setParent( this );
}

// ------------------------------------------------------------------------
BaseClauseAST::BaseClauseAST()
{
    m_baseSpecifiers.setAutoDelete( true );
}

BaseClauseAST::~BaseClauseAST()
{
}

void BaseClauseAST::addBaseSpecifier( BaseSpecifierAST::Node& baseSpecifier )
{
    if( !baseSpecifier.get() )
        return;

    baseSpecifier->setParent( this );
    m_baseSpecifiers.append( baseSpecifier.release() );
}

// ------------------------------------------------------------------------
BaseSpecifierAST::BaseSpecifierAST()
{
}

BaseSpecifierAST::~BaseSpecifierAST()
{
}

// ------------------------------------------------------------------------
SimpleDeclarationAST::SimpleDeclarationAST()
{
}

SimpleDeclarationAST::~SimpleDeclarationAST()
{
}

TypeSpecifierAST* SimpleDeclarationAST::typeSpec()
{
    return m_typeSpec.get();
}

void SimpleDeclarationAST::setTypeSpec( TypeSpecifierAST::Node& typeSpec )
{
    m_typeSpec = typeSpec;
    if( m_typeSpec.get() ) m_typeSpec->setParent( this );
}

InitDeclaratorListAST* SimpleDeclarationAST::initDeclaratorList()
{
    return m_initDeclaratorList.get();
}

void SimpleDeclarationAST::setInitDeclaratorList( InitDeclaratorListAST::Node& initDeclaratorList )
{
    m_initDeclaratorList = initDeclaratorList;
    if( m_initDeclaratorList.get() ) m_initDeclaratorList->setParent( this );
}

// ------------------------------------------------------------------------
InitDeclaratorListAST::InitDeclaratorListAST()
{
}

InitDeclaratorListAST::~InitDeclaratorListAST()
{
}

// ------------------------------------------------------------------------
DeclaratorAST::DeclaratorAST()
    : m_isConstMethod( false )
{
    m_ptrOpList.setAutoDelete( true );
    m_arrayDimensionList.setAutoDelete( true );
}

DeclaratorAST::~DeclaratorAST()
{
}

DeclaratorAST* DeclaratorAST::subDeclarator()
{
    return m_subDeclarator.get();
}

void DeclaratorAST::setSubDeclarator( DeclaratorAST::Node& subDeclarator )
{
    m_subDeclarator = subDeclarator;
    if( m_subDeclarator.get() ) m_subDeclarator->setParent( this );
}

NameAST* DeclaratorAST::declaratorId()
{
    return m_declaratorId.get();
}

void DeclaratorAST::setDeclaratorId( NameAST::Node& declaratorId )
{
    m_declaratorId = declaratorId;
    if( m_declaratorId.get() ) m_declaratorId->setParent( this );
}

AST* DeclaratorAST::bitfieldInitialization()
{
    return m_bitfieldInitialization.get();
}

void DeclaratorAST::setBitfieldInitialization( AST::Node& bitfieldInitialization )
{
    m_bitfieldInitialization = bitfieldInitialization;
    if( m_bitfieldInitialization.get() ) m_bitfieldInitialization->setParent( this );
}

QPtrList<AST> DeclaratorAST::arrayDimensionList()
{
    return m_arrayDimensionList;
}

void DeclaratorAST::addArrayDimension( AST::Node& arrayDimension )
{
    if( !arrayDimension.get() )
        return;

    arrayDimension->setParent( this );
    m_arrayDimensionList.append( arrayDimension.release() );
}

AST* DeclaratorAST::parameterDeclarationClause()
{
    return m_parameterDeclarationClause.get();
}

void DeclaratorAST::setParameterDeclarationClause( AST::Node& parameterDeclarationClause )
{
    m_parameterDeclarationClause = parameterDeclarationClause;
    if( m_parameterDeclarationClause.get() ) m_parameterDeclarationClause->setParent( this );
}

bool DeclaratorAST::isConstMethod()
{
    return m_isConstMethod;
}

void DeclaratorAST::setIsConstMethod( bool isConstMethod )
{
    m_isConstMethod = isConstMethod;
}

AST* DeclaratorAST::exceptionSpecification()
{
    return m_exceptionSpecification.get();
}

void DeclaratorAST::setExceptionSpecification( AST::Node& exceptionSpecification )
{
    m_exceptionSpecification = exceptionSpecification;
    if( m_exceptionSpecification.get() ) m_exceptionSpecification->setParent( this );
}

QPtrList<AST> DeclaratorAST::ptrOpList()
{
    return m_ptrOpList;
}

void DeclaratorAST::addPtrOp( AST::Node& ptrOp )
{
    if( !ptrOp.get() )
        return;

    ptrOp->setParent( this );
    m_ptrOpList.append( ptrOp.release() );
}


// --------------------------------------------------------------------------
InitDeclaratorAST::InitDeclaratorAST()
{
}

InitDeclaratorAST::~InitDeclaratorAST()
{
}

DeclaratorAST* InitDeclaratorAST::declarator()
{
    return m_declarator.get();
}

void InitDeclaratorAST::setDeclarator( DeclaratorAST::Node& declarator )
{
    m_declarator = declarator;
    if( m_declarator.get() ) m_declarator->setParent( this );
}

AST* InitDeclaratorAST::initializer()
{
    return m_initializer.get();
}

void InitDeclaratorAST::setInitializer( AST::Node& initializer )
{
    m_initializer = initializer;
    if( m_initializer.get() ) m_initializer->setParent( this );
}
