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
    case NodeType_IfStatement:
	return "IfStatement";
    case NodeType_WhileStatement:
	return "WhileStatement";
    case NodeType_DoStatement:
	return "DoStatement";
    case NodeType_ForStatement:
	return "ForStatement";
    case NodeType_SwitchStatement:
	return "SwitchStatement";
    case NodeType_DeclarationStatement:
	return "DeclarationStatement";
    case NodeType_StatementList:
	return "StatementList";
    case NodeType_TranslationUnit:
	return "TranslationUnit";
    case NodeType_FunctionDefinition:
	return "FunctionDefinition";
    case NodeType_ExpressionStatement:
	return "ExpressionStatement";
    case NodeType_ParameterDeclaration:
	return "ParameterDeclaration";
    case NodeType_ParameterDeclarationList:
	return "ParameterDeclarationList";
    case NodeType_ParameterDeclarationClause:
	return "ParameterDeclarationClause";
    case NodeType_Group:
	return "Group";
    case NodeType_AccessDeclaration:
	return "AccessDeclaration";
    case NodeType_UnqualifiedName:
	return "UnqualifiedName";
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

void NameAST::setGlobal( bool b )
{
    m_global = b;
}

void NameAST::setNestedName( NestedNameSpecifierAST::Node& nestedName )
{
    m_nestedName = nestedName;
    if( m_nestedName.get() ) m_nestedName->setParent( this );
}

void NameAST::setUnqualifedName( AST::Node& unqualifiedName )
{
    m_unqualifiedName = unqualifiedName;
    if( m_unqualifiedName.get() ) m_unqualifiedName->setParent( this );
}

// ------------------------------------------------------------------------
DeclarationAST::DeclarationAST()
{
}

// ------------------------------------------------------------------------
LinkageBodyAST::LinkageBodyAST()
{
    m_declarationList.setAutoDelete( true );
}

void LinkageBodyAST::addDeclaration( DeclarationAST::Node& ast )
{
    if( !ast.get() )
        return;

    ast->setParent( this );
    m_declarationList.append( ast.release() );
}

// ------------------------------------------------------------------------
LinkageSpecificationAST::LinkageSpecificationAST()
{
}

void LinkageSpecificationAST::setExternType( AST::Node& externType )
{
    m_externType = externType;
    if( m_externType.get() ) m_externType->setParent( this );
}

void LinkageSpecificationAST::setLinkageBody( LinkageBodyAST::Node& linkageBody )
{
    m_linkageBody = linkageBody;
    if( m_linkageBody.get() ) m_linkageBody->setParent( this );
}

void LinkageSpecificationAST::setDeclaration( DeclarationAST::Node& decl )
{
    m_declaration = decl;
    if( m_declaration.get() ) m_declaration->setParent( this );
}

// ------------------------------------------------------------------------
TranslationUnitAST::TranslationUnitAST()
{
    //kdDebug(9007) << "++ TranslationUnitAST::TranslationUnitAST()" << endl;
    m_declarationList.setAutoDelete( true );
}

void TranslationUnitAST::addDeclaration( DeclarationAST::Node& ast )
{
    if( !ast.get() )
        return;

    ast->setParent( this );
    m_declarationList.append( ast.release() );
}

// ------------------------------------------------------------------------
NamespaceAST::NamespaceAST()
{
}

void NamespaceAST::setNamespaceName( AST::Node& namespaceName )
{
    m_namespaceName = namespaceName;
    if( m_namespaceName.get() ) m_namespaceName->setParent( this );
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

void NamespaceAliasAST::setNamespaceName( AST::Node& namespaceName )
{
    m_namespaceName = namespaceName;
    if( m_namespaceName.get() ) m_namespaceName->setParent( this );
}

void NamespaceAliasAST::setAliasName( NameAST::Node& name )
{
    m_aliasName = name;
    if( m_aliasName.get() ) m_aliasName->setParent( this );
}

// ------------------------------------------------------------------------
UsingAST::UsingAST()
{
}

void UsingAST::setTypeName( AST::Node& typeName )
{
    m_typeName = typeName;
    if( m_typeName.get() ) m_typeName->setParent( this );
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

void UsingDirectiveAST::setName( NameAST::Node& name )
{
    m_name = name;
    if( m_name.get() ) m_name->setParent( this );
}

TypedefAST::TypedefAST()
{
}

void TypeSpecifierAST::setName( NameAST::Node& name )
{
    m_name = name;
    if( m_name.get() ) m_name->setParent( this );
}

void TypedefAST::setTypeSpec( TypeSpecifierAST::Node& typeSpec )
{
    m_typeSpec = typeSpec;
    if( m_typeSpec.get() ) m_typeSpec->setParent( this );
}

void TypedefAST::setInitDeclaratorList( InitDeclaratorListAST::Node& initDeclaratorList )
{
    m_initDeclaratorList = initDeclaratorList;
    if( m_initDeclaratorList.get() ) m_initDeclaratorList->setParent( this );
}

// ------------------------------------------------------------------------
TemplateArgumentListAST::TemplateArgumentListAST()
{
    m_argumentList.setAutoDelete( true );
}

void TemplateArgumentListAST::addArgument( AST::Node& arg )
{
    if( !arg.get() )
        return;

    arg->setParent( this );
    m_argumentList.append( arg.release() );
}


// ------------------------------------------------------------------------
TemplateDeclarationAST::TemplateDeclarationAST()
{
}

void TemplateDeclarationAST::setExported( AST::Node& exported )
{
    m_exported = exported;
    if( m_exported.get() ) m_exported->setParent( this );
}

void TemplateDeclarationAST::setTemplateParameterList( AST::Node& templateParameterList )
{
    m_templateParameterList = templateParameterList;
    if( m_templateParameterList.get() ) m_templateParameterList->setParent( this );
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

void ClassOrNamespaceNameAST::setName( AST::Node& name )
{
    m_name = name;
    if( m_name.get() ) m_name->setParent( this );
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

void TypeSpecifierAST::setCvQualify( GroupAST::Node& cvQualify )
{
    m_cvQualify = cvQualify;
    if( m_cvQualify.get() ) m_cvQualify->setParent( this );
}

void TypeSpecifierAST::setCv2Qualify( GroupAST::Node& cv2Qualify )
{
    m_cv2Qualify = cv2Qualify;
    if( m_cv2Qualify.get() ) m_cv2Qualify->setParent( this );
}

// ------------------------------------------------------------------------
ClassSpecifierAST::ClassSpecifierAST()
{
    m_declarationList.setAutoDelete( true );
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
    m_declarationList.append( declaration.release() );
}

void ClassSpecifierAST::setBaseClause( BaseClauseAST::Node& baseClause )
{
    m_baseClause = baseClause;
    if( m_baseClause.get() ) m_baseClause->setParent( this );
}

// ------------------------------------------------------------------------
EnumSpecifierAST::EnumSpecifierAST()
{
    m_enumeratorList.setAutoDelete( true );
}

void EnumSpecifierAST::addEnumerator( EnumeratorAST::Node& enumerator )
{
    if( !enumerator.get() )
        return;

    enumerator->setParent( this );
    m_enumeratorList.append( enumerator.release() );
}


// ------------------------------------------------------------------------
ElaboratedTypeSpecifierAST::ElaboratedTypeSpecifierAST()
{
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

// ------------------------------------------------------------------------
EnumeratorAST::EnumeratorAST()
{
}

void EnumeratorAST::setId( AST::Node& id )
{
    m_id = id;
    if( m_id.get() ) m_id->setParent( this );
}

void EnumeratorAST::setExpr( AST::Node& expr )
{
    m_expr = expr;
    if( m_expr.get() ) m_expr->setParent( this );
}

// ------------------------------------------------------------------------
BaseClauseAST::BaseClauseAST()
{
    m_baseSpecifierList.setAutoDelete( true );
}

void BaseClauseAST::addBaseSpecifier( BaseSpecifierAST::Node& baseSpecifier )
{
    if( !baseSpecifier.get() )
        return;

    baseSpecifier->setParent( this );
    m_baseSpecifierList.append( baseSpecifier.release() );
}

// ------------------------------------------------------------------------
BaseSpecifierAST::BaseSpecifierAST()
{
}

void BaseSpecifierAST::setIsVirtual( AST::Node& isVirtual )
{
    m_isVirtual = isVirtual;
    if( m_isVirtual.get() ) m_isVirtual->setParent( this );
}

void BaseSpecifierAST::setAccess( AST::Node& access )
{
    m_access = access;
    if( m_access.get() ) m_access->setParent( this );
}

void BaseSpecifierAST::setName( NameAST::Node& name )
{
    m_name = name;
    if( m_name.get() ) m_name->setParent( this );
}

// ------------------------------------------------------------------------
SimpleDeclarationAST::SimpleDeclarationAST()
{
}

void SimpleDeclarationAST::setFunctionSpecifier( GroupAST::Node& functionSpecifier )
{
    m_functionSpecifier = functionSpecifier;
    if( m_functionSpecifier.get() ) m_functionSpecifier->setParent( this );
}

void SimpleDeclarationAST::setStorageSpecifier( GroupAST::Node& storageSpecifier )
{
    m_storageSpecifier = storageSpecifier;
    if( m_storageSpecifier.get() ) m_storageSpecifier->setParent( this );
}

void SimpleDeclarationAST::setTypeSpec( TypeSpecifierAST::Node& typeSpec )
{
    m_typeSpec = typeSpec;
    if( m_typeSpec.get() ) m_typeSpec->setParent( this );
}

void SimpleDeclarationAST::setInitDeclaratorList( InitDeclaratorListAST::Node& initDeclaratorList )
{
    m_initDeclaratorList = initDeclaratorList;
    if( m_initDeclaratorList.get() ) m_initDeclaratorList->setParent( this );
}

// ------------------------------------------------------------------------
InitDeclaratorListAST::InitDeclaratorListAST()
{
    m_initDeclaratorList.setAutoDelete( true );
}

void InitDeclaratorListAST::addInitDeclarator( InitDeclaratorAST::Node& decl )
{
    if( !decl.get() )
	return;

    decl->setParent( this );
    m_initDeclaratorList.append( decl.release() );
}

// ------------------------------------------------------------------------
DeclaratorAST::DeclaratorAST()
{
    m_ptrOpList.setAutoDelete( true );
    m_arrayDimensionList.setAutoDelete( true );
}

void DeclaratorAST::setSubDeclarator( DeclaratorAST::Node& subDeclarator )
{
    m_subDeclarator = subDeclarator;
    if( m_subDeclarator.get() ) m_subDeclarator->setParent( this );
}

void DeclaratorAST::setDeclaratorId( NameAST::Node& declaratorId )
{
    m_declaratorId = declaratorId;
    if( m_declaratorId.get() ) m_declaratorId->setParent( this );
}

void DeclaratorAST::setBitfieldInitialization( AST::Node& bitfieldInitialization )
{
    m_bitfieldInitialization = bitfieldInitialization;
    if( m_bitfieldInitialization.get() ) m_bitfieldInitialization->setParent( this );
}

void DeclaratorAST::addArrayDimension( AST::Node& arrayDimension )
{
    if( !arrayDimension.get() )
        return;

    arrayDimension->setParent( this );
    m_arrayDimensionList.append( arrayDimension.release() );
}

void DeclaratorAST::setParameterDeclarationClause( ParameterDeclarationClauseAST::Node& parameterDeclarationClause )
{
    m_parameterDeclarationClause = parameterDeclarationClause;
    if( m_parameterDeclarationClause.get() ) m_parameterDeclarationClause->setParent( this );
}

void DeclaratorAST::setConstant( AST::Node& constant )
{
    m_constant = constant;
    if( m_constant.get() ) m_constant->setParent( this );
}

void DeclaratorAST::setExceptionSpecification( AST::Node& exceptionSpecification )
{
    m_exceptionSpecification = exceptionSpecification;
    if( m_exceptionSpecification.get() ) m_exceptionSpecification->setParent( this );
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

void InitDeclaratorAST::setDeclarator( DeclaratorAST::Node& declarator )
{
    m_declarator = declarator;
    if( m_declarator.get() ) m_declarator->setParent( this );
}

void InitDeclaratorAST::setInitializer( AST::Node& initializer )
{
    m_initializer = initializer;
    if( m_initializer.get() ) m_initializer->setParent( this );
}

// --------------------------------------------------------------------------
FunctionDefinitionAST::FunctionDefinitionAST()
{
}

void FunctionDefinitionAST::setFunctionSpecifier( GroupAST::Node& functionSpecifier )
{
    m_functionSpecifier = functionSpecifier;
    if( m_functionSpecifier.get() ) m_functionSpecifier->setParent( this );
}

void FunctionDefinitionAST::setStorageSpecifier( GroupAST::Node& storageSpecifier )
{
    m_storageSpecifier = storageSpecifier;
    if( m_storageSpecifier.get() ) m_storageSpecifier->setParent( this );
}

void FunctionDefinitionAST::setTypeSpec( TypeSpecifierAST::Node& typeSpec )
{
    m_typeSpec = typeSpec;
    if( m_typeSpec.get() ) m_typeSpec->setParent( this );
}
    
void FunctionDefinitionAST::setNestedName( NestedNameSpecifierAST::Node& nestedName )
{
    m_nestedName = nestedName;
    if( m_nestedName.get() ) m_nestedName->setParent( this );
}

void FunctionDefinitionAST::setInitDeclarator( InitDeclaratorAST::Node& initDeclarator )
{
    m_initDeclarator = initDeclarator;
    if( m_initDeclarator.get() ) m_initDeclarator->setParent( this );
}

void FunctionDefinitionAST::setFunctionBody( StatementListAST::Node& functionBody )
{
    m_functionBody = functionBody;
    if( m_functionBody.get() ) m_functionBody->setParent( this );
}

// --------------------------------------------------------------------------
StatementListAST::StatementListAST()
{
    m_statementList.setAutoDelete( true );
}

void StatementListAST::addStatement( StatementAST::Node& statement )
{
    if( !statement.get() )
	return;

    statement->setParent( this );
    m_statementList.append( statement.release() );
}

// --------------------------------------------------------------------------
IfStatementAST::IfStatementAST()
{
}

void IfStatementAST::setCondition( AST::Node& condition )
{
    m_condition = condition;
    if( m_condition.get() ) m_condition->setParent( this );
}

void IfStatementAST::setStatement( StatementAST::Node& statement )
{
    m_statement = statement;
    if( m_statement.get() ) m_statement->setParent( this );
}

void IfStatementAST::setElseStatement( StatementAST::Node& elseStatement )
{
    m_elseStatement = elseStatement;
    if( m_elseStatement.get() ) m_elseStatement->setParent( this );
}

// --------------------------------------------------------------------------
WhileStatementAST::WhileStatementAST()
{
}

void WhileStatementAST::setCondition( AST::Node& condition )
{
    m_condition = condition;
    if( m_condition.get() ) m_condition->setParent( this );
}

void WhileStatementAST::setStatement( StatementAST::Node& statement )
{
    m_statement = statement;
    if( m_statement.get() ) m_statement->setParent( this );
}

// --------------------------------------------------------------------------
DoStatementAST::DoStatementAST()
{
}

void DoStatementAST::setCondition( AST::Node& condition )
{
    m_condition = condition;
    if( m_condition.get() ) m_condition->setParent( this );
}

void DoStatementAST::setStatement( StatementAST::Node& statement )
{
    m_statement = statement;
    if( m_statement.get() ) m_statement->setParent( this );
}

// --------------------------------------------------------------------------
ForStatementAST::ForStatementAST()
{
}

void ForStatementAST::setCondition( AST::Node& condition )
{
    m_condition = condition;
    if( m_condition.get() ) m_condition->setParent( this );
}

void ForStatementAST::setExpression( AST::Node& expression )
{
    m_expression = expression;
    if( m_expression.get() ) m_expression->setParent( this );
}

void ForStatementAST::setStatement( StatementAST::Node& statement )
{
    m_statement = statement;
    if( m_statement.get() ) m_statement->setParent( this );
}

void ForStatementAST::setInitStatement( StatementAST::Node& initStatement )
{
    m_initStatement = initStatement;
    if( m_initStatement.get() ) m_initStatement->setParent( this );
}

// --------------------------------------------------------------------------
SwitchStatementAST::SwitchStatementAST()
{
}

void SwitchStatementAST::setCondition( AST::Node& condition )
{
    m_condition = condition;
    if( m_condition.get() ) m_condition->setParent( this );
}

void SwitchStatementAST::setStatement( StatementAST::Node& statement )
{
    m_statement = statement;
    if( m_statement.get() ) m_statement->setParent( this );
}

// --------------------------------------------------------------------------
DeclarationStatementAST::DeclarationStatementAST()
{
}

void DeclarationStatementAST::setDeclaration( DeclarationAST::Node& declaration )
{
    m_declaration = declaration;
    if( m_declaration.get() ) m_declaration->setParent( this );
}

// --------------------------------------------------------------------------
ExpressionStatementAST::ExpressionStatementAST()
{
}

void ExpressionStatementAST::setExpression( AST::Node& expression )
{
    m_expression = expression;
    if( m_expression.get() ) m_expression->setParent( this );
}


// --------------------------------------------------------------------------
ParameterDeclarationAST::ParameterDeclarationAST()
{
}

void ParameterDeclarationAST::setTypeSpec( TypeSpecifierAST::Node& typeSpec )
{
    m_typeSpec = typeSpec;
    if( m_typeSpec.get() ) m_typeSpec->setParent( this );
}

void ParameterDeclarationAST::setDeclarator( DeclaratorAST::Node& declarator )
{
    m_declarator = declarator;
    if( m_declarator.get() ) m_declarator->setParent( this );
}

void ParameterDeclarationAST::setExpression( AST::Node& expression )
{
    m_expression = expression;
    if( m_expression.get() ) m_expression->setParent( this );
}


// --------------------------------------------------------------------------
ParameterDeclarationListAST::ParameterDeclarationListAST()
{
    m_parameterList.setAutoDelete( true );
}

void ParameterDeclarationListAST::addParameter( ParameterDeclarationAST::Node& parameter )
{
    if( !parameter.get() )
        return;

    parameter->setParent( this );
    m_parameterList.append( parameter.release() );
}


// --------------------------------------------------------------------------
ParameterDeclarationClauseAST::ParameterDeclarationClauseAST()
{
}

void ParameterDeclarationClauseAST::setParameterDeclarationList( ParameterDeclarationListAST::Node& parameterDeclarationList )
{
    m_parameterDeclarationList = parameterDeclarationList;
    if( m_parameterDeclarationList.get() ) m_parameterDeclarationList->setParent( this );
}

void ParameterDeclarationClauseAST::setEllipsis( AST::Node& ellipsis )
{
    m_ellipsis = ellipsis;
    if( m_ellipsis.get() ) m_ellipsis->setParent( this );
}

// --------------------------------------------------------------------------
GroupAST::GroupAST()
{
    m_nodeList.setAutoDelete( true );
}

void GroupAST::addNode( AST::Node& node )
{
    if( !node.get() )
        return;

    node->setParent( this );
    m_nodeList.append( node.release() );
}

// --------------------------------------------------------------------------
AccessDeclarationAST::AccessDeclarationAST()
{
    m_accessList.setAutoDelete( true );
}

void AccessDeclarationAST::addAccess( AST::Node& access )
{
    if( !access.get() )
        return;

    access->setParent( this );
    m_accessList.append( access.release() );
}

// --------------------------------------------------------------------------
UnqualifiedNameAST::UnqualifiedNameAST()
    : m_isDestructor( false )
{
}

void UnqualifiedNameAST::setName( AST::Node& name )
{
    m_name = name;
    if( m_name.get() ) m_name->setParent( this );
}

void UnqualifiedNameAST::setIsDestructor( bool isDestructor )
{
    m_isDestructor = isDestructor;
}

void UnqualifiedNameAST::setTemplateArgumentList( TemplateArgumentListAST::Node& templateArgumentList )
{
    m_templateArgumentList = templateArgumentList;
    if( m_templateArgumentList.get() ) m_templateArgumentList->setParent( this );
}

