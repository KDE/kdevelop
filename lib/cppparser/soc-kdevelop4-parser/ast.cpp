/* This file is part of KDevelop
    Copyright (C) 2002,2003,2004 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "ast.h"

// ------------------------------------------------------------------------
void AST::setParent(AST *parent)
{
    _M_parent = parent;

    if (_M_parent)
        _M_parent->appendChild(this);
}

void AST::appendChild(AST *node)
{
    _M_children = snoc(_M_children, node, _pool);
}

// ------------------------------------------------------------------------
void NameAST::setGlobal(bool b)
{
    _M_global = b;
}

void NameAST::setUnqualifiedName(ClassOrNamespaceNameAST *unqualifiedName)
{
    _M_unqualifiedName = unqualifiedName;
    if (_M_unqualifiedName) _M_unqualifiedName->setParent(this);
}

void NameAST::addClassOrNamespaceName(ClassOrNamespaceNameAST *classOrNamespaceName)
{
    if(!classOrNamespaceName)
        return;

    classOrNamespaceName->setParent(this);
    _M_classOrNamespaceNameList = snoc(_M_classOrNamespaceNameList, classOrNamespaceName, _pool);
}

// ------------------------------------------------------------------------
void LinkageBodyAST::addDeclaration(DeclarationAST *ast)
{
    if(!ast)
        return;

    ast->setParent(this);
    _M_declarationList = snoc(_M_declarationList, ast, _pool);
}

// ------------------------------------------------------------------------
void LinkageSpecificationAST::setExternType(AST *externType)
{
    _M_externType = externType;
    if (_M_externType) _M_externType->setParent(this);
}

void LinkageSpecificationAST::setLinkageBody(LinkageBodyAST *linkageBody)
{
    _M_linkageBody = linkageBody;
    if (_M_linkageBody) _M_linkageBody->setParent(this);
}

void LinkageSpecificationAST::setDeclaration(DeclarationAST *decl)
{
    _M_declaration = decl;
    if (_M_declaration) _M_declaration->setParent(this);
}

// ------------------------------------------------------------------------
void TranslationUnitAST::addDeclaration(DeclarationAST *ast)
{
    if(!ast)
        return;

    ast->setParent(this);
    _M_declarationList = snoc(_M_declarationList, ast, _pool);
}

// ------------------------------------------------------------------------
void NamespaceAST::setNamespaceName(AST *namespaceName)
{
    _M_namespaceName = namespaceName;
    if (_M_namespaceName) _M_namespaceName->setParent(this);
}

void NamespaceAST::setLinkageBody(LinkageBodyAST *linkageBody)
{
    _M_linkageBody = linkageBody;
    if (_M_linkageBody) _M_linkageBody->setParent(this);
}


// ------------------------------------------------------------------------
void NamespaceAliasAST::setNamespaceName(AST *namespaceName)
{
    _M_namespaceName = namespaceName;
    if (_M_namespaceName) _M_namespaceName->setParent(this);
}

void NamespaceAliasAST::setAliasName(NameAST *name)
{
    _M_aliasName = name;
    if (_M_aliasName) _M_aliasName->setParent(this);
}

// ------------------------------------------------------------------------
void UsingAST::setTypeName(AST *typeName)
{
    _M_typeName = typeName;
    if (_M_typeName) _M_typeName->setParent(this);
}

void UsingAST::setName(NameAST *name)
{
    _M_name = name;
    if (_M_name) _M_name->setParent(this);
}

// ------------------------------------------------------------------------
void UsingDirectiveAST::setName(NameAST *name)
{
    _M_name = name;
    if (_M_name) _M_name->setParent(this);
}

// ------------------------------------------------------------------------
void TypeSpecifierAST::setName(NameAST *name)
{
    _M_name = name;
    if (_M_name) _M_name->setParent(this);
}

void TypedefAST::setTypeSpec(TypeSpecifierAST *typeSpec)
{
    _M_typeSpec = typeSpec;
    if (_M_typeSpec) _M_typeSpec->setParent(this);
}

void TypedefAST::setInitDeclaratorList(InitDeclaratorListAST *initDeclaratorList)
{
    _M_initDeclaratorList = initDeclaratorList;
    if (_M_initDeclaratorList) _M_initDeclaratorList->setParent(this);
}

// ------------------------------------------------------------------------
void TemplateArgumentListAST::addArgument(AST *arg)
{
    if(!arg)
        return;

    arg->setParent(this);
    _M_argumentList = snoc(_M_argumentList, arg, _pool);
}

// ------------------------------------------------------------------------
void TemplateDeclarationAST::setExported(AST *exported)
{
    _M_exported = exported;
    if (_M_exported) _M_exported->setParent(this);
}

void TemplateDeclarationAST::setTemplateParameterList(TemplateParameterListAST *templateParameterList)
{
    _M_templateParameterList = templateParameterList;
    if (_M_templateParameterList) _M_templateParameterList->setParent(this);
}

void TemplateDeclarationAST::setDeclaration(DeclarationAST *declaration)
{
    _M_declaration = declaration;
    if (_M_declaration) _M_declaration->setParent(this);
}

// ------------------------------------------------------------------------
void ClassOrNamespaceNameAST::setName(AST *name)
{
    _M_name = name;
    if (_M_name) _M_name->setParent(this);
}

void ClassOrNamespaceNameAST::setTemplateArgumentList(TemplateArgumentListAST *templateArgumentList)
{
    _M_templateArgumentList = templateArgumentList;
    if (_M_templateArgumentList) _M_templateArgumentList->setParent(this);
}

// ------------------------------------------------------------------------
void TypeSpecifierAST::setCvQualify(AST *cvQualify)
{
    _M_cvQualify = cvQualify;
    if (_M_cvQualify) _M_cvQualify->setParent(this);
}

void TypeSpecifierAST::setCv2Qualify(AST *cv2Qualify)
{
    _M_cv2Qualify = cv2Qualify;
    if (_M_cv2Qualify) _M_cv2Qualify->setParent(this);
}

// ------------------------------------------------------------------------
void ClassSpecifierAST::setClassKey(AST *classKey)
{
    _M_classKey = classKey;
    if (_M_classKey) _M_classKey->setParent(this);
}

void ClassSpecifierAST::addDeclaration(DeclarationAST *declaration)
{
    if(!declaration)
        return;

    declaration->setParent(this);
    _M_declarationList = snoc(_M_declarationList, declaration, _pool);
}

void ClassSpecifierAST::setBaseClause(BaseClauseAST *baseClause)
{
    _M_baseClause = baseClause;
    if (_M_baseClause) _M_baseClause->setParent(this);
}

// ------------------------------------------------------------------------
void EnumSpecifierAST::addEnumerator(EnumeratorAST *enumerator)
{
    if(!enumerator)
        return;

    enumerator->setParent(this);
    _M_enumeratorList = snoc(_M_enumeratorList, enumerator, _pool);
}


// ------------------------------------------------------------------------
void ElaboratedTypeSpecifierAST::setKind(AST *kind)
{
    _M_kind = kind;
    if (_M_kind) _M_kind->setParent(this);
}

// ------------------------------------------------------------------------
void EnumeratorAST::setId(AST *id)
{
    _M_id = id;
    if (_M_id) _M_id->setParent(this);
}

void EnumeratorAST::setExpression(AbstractExpressionAST *expression)
{
    _M_expression = expression;
    if (_M_expression) _M_expression->setParent(this);
}

// ------------------------------------------------------------------------
void BaseClauseAST::addBaseSpecifier(BaseSpecifierAST *baseSpecifier)
{
    if(!baseSpecifier)
        return;

    baseSpecifier->setParent(this);
    _M_baseSpecifierList = snoc(_M_baseSpecifierList, baseSpecifier, _pool);
}

// ------------------------------------------------------------------------
void BaseSpecifierAST::setIsVirtual(AST *isVirtual)
{
    _M_isVirtual = isVirtual;
    if (_M_isVirtual) _M_isVirtual->setParent(this);
}

void BaseSpecifierAST::setAccess(AST *access)
{
    _M_access = access;
    if (_M_access) _M_access->setParent(this);
}

void BaseSpecifierAST::setName(NameAST *name)
{
    _M_name = name;
    if (_M_name) _M_name->setParent(this);
}

// ------------------------------------------------------------------------
void SimpleDeclarationAST::setFunctionSpecifier(AST *functionSpecifier)
{
    _M_functionSpecifier = functionSpecifier;
    if (_M_functionSpecifier) _M_functionSpecifier->setParent(this);
}

void SimpleDeclarationAST::setStorageSpecifier(AST *storageSpecifier)
{
    _M_storageSpecifier = storageSpecifier;
    if (_M_storageSpecifier) _M_storageSpecifier->setParent(this);
}

void SimpleDeclarationAST::setTypeSpec(TypeSpecifierAST *typeSpec)
{
    _M_typeSpec = typeSpec;
    if (_M_typeSpec) _M_typeSpec->setParent(this);
}

void SimpleDeclarationAST::setInitDeclaratorList(InitDeclaratorListAST *initDeclaratorList)
{
    _M_initDeclaratorList = initDeclaratorList;
    if (_M_initDeclaratorList) _M_initDeclaratorList->setParent(this);
}

void SimpleDeclarationAST::setWinDeclSpec(AST *winDeclSpec)
{
    _M_winDeclSpec = winDeclSpec;
    if (_M_winDeclSpec) _M_winDeclSpec->setParent(this);
}

// ------------------------------------------------------------------------
void InitDeclaratorListAST::addInitDeclarator(InitDeclaratorAST *decl)
{
    if(!decl)
        return;

    decl->setParent(this);
    _M_initDeclaratorList = snoc(_M_initDeclaratorList, decl, _pool);
}

// ------------------------------------------------------------------------
void DeclaratorAST::setSubDeclarator(DeclaratorAST *subDeclarator)
{
    _M_subDeclarator = subDeclarator;
    if (_M_subDeclarator) _M_subDeclarator->setParent(this);
}

void DeclaratorAST::setDeclaratorId(NameAST *declaratorId)
{
    _M_declaratorId = declaratorId;
    if (_M_declaratorId) _M_declaratorId->setParent(this);
}

void DeclaratorAST::setBitfieldInitialization(AST *bitfieldInitialization)
{
    _M_bitfieldInitialization = bitfieldInitialization;
    if (_M_bitfieldInitialization) _M_bitfieldInitialization->setParent(this);
}

void DeclaratorAST::addArrayDimension(AST *arrayDimension)
{
    if(!arrayDimension)
        return;

    arrayDimension->setParent(this);
    _M_arrayDimensionList = snoc(_M_arrayDimensionList, arrayDimension, _pool);
}

void DeclaratorAST::setParameterDeclarationClause(ParameterDeclarationClauseAST *parameterDeclarationClause)
{
    _M_parameterDeclarationClause = parameterDeclarationClause;
    if (_M_parameterDeclarationClause) _M_parameterDeclarationClause->setParent(this);
}

void DeclaratorAST::setConstant(AST *constant)
{
    _M_constant = constant;
    if (_M_constant) _M_constant->setParent(this);
}

void DeclaratorAST::setExceptionSpecification(AST *exceptionSpecification)
{
    _M_exceptionSpecification = exceptionSpecification;
    if (_M_exceptionSpecification) _M_exceptionSpecification->setParent(this);
}

void DeclaratorAST::addPtrOp(AST *ptrOp)
{
    if(!ptrOp)
        return;

    ptrOp->setParent(this);
    _M_ptrOpList = snoc(_M_ptrOpList, ptrOp, _pool);
}

// --------------------------------------------------------------------------
void InitDeclaratorAST::setDeclarator(DeclaratorAST *declarator)
{
    _M_declarator = declarator;
    if (_M_declarator) _M_declarator->setParent(this);
}

void InitDeclaratorAST::setInitializer(AST *initializer)
{
    _M_initializer = initializer;
    if (_M_initializer) _M_initializer->setParent(this);
}

// --------------------------------------------------------------------------
void FunctionDefinitionAST::setFunctionSpecifier(AST *functionSpecifier)
{
    _M_functionSpecifier = functionSpecifier;
    if (_M_functionSpecifier) _M_functionSpecifier->setParent(this);
}

void FunctionDefinitionAST::setStorageSpecifier(AST *storageSpecifier)
{
    _M_storageSpecifier = storageSpecifier;
    if (_M_storageSpecifier) _M_storageSpecifier->setParent(this);
}

void FunctionDefinitionAST::setTypeSpec(TypeSpecifierAST *typeSpec)
{
    _M_typeSpec = typeSpec;
    if (_M_typeSpec) _M_typeSpec->setParent(this);
}

void FunctionDefinitionAST::setInitDeclarator(InitDeclaratorAST *initDeclarator)
{
    _M_initDeclarator = initDeclarator;
    if (_M_initDeclarator) _M_initDeclarator->setParent(this);
}

void FunctionDefinitionAST::setFunctionBody(StatementListAST *functionBody)
{
    _M_functionBody = functionBody;
    if (_M_functionBody) _M_functionBody->setParent(this);
}

void FunctionDefinitionAST::setWinDeclSpec(AST *winDeclSpec)
{
    _M_winDeclSpec = winDeclSpec;
    if (_M_winDeclSpec) _M_winDeclSpec->setParent(this);
}

// --------------------------------------------------------------------------
void StatementListAST::addStatement(StatementAST *statement)
{
    if(!statement)
        return;

    statement->setParent(this);
    _M_statementList = snoc(_M_statementList, statement, _pool);
}

// --------------------------------------------------------------------------
void IfStatementAST::setCondition(ConditionAST *condition)
{
    _M_condition = condition;
    if (_M_condition) _M_condition->setParent(this);
}

void IfStatementAST::setStatement(StatementAST *statement)
{
    _M_statement = statement;
    if (_M_statement) _M_statement->setParent(this);
}

void IfStatementAST::setElseStatement(StatementAST *elseStatement)
{
    _M_elseStatement = elseStatement;
    if (_M_elseStatement) _M_elseStatement->setParent(this);
}

// --------------------------------------------------------------------------
void WhileStatementAST::setCondition(ConditionAST *condition)
{
    _M_condition = condition;
    if (_M_condition) _M_condition->setParent(this);
}

void WhileStatementAST::setStatement(StatementAST *statement)
{
    _M_statement = statement;
    if (_M_statement) _M_statement->setParent(this);
}

// --------------------------------------------------------------------------
void DoStatementAST::setCondition(ConditionAST *condition)
{
    _M_condition = condition;
    if (_M_condition) _M_condition->setParent(this);
}

void DoStatementAST::setStatement(StatementAST *statement)
{
    _M_statement = statement;
    if (_M_statement) _M_statement->setParent(this);
}

// --------------------------------------------------------------------------
void ForStatementAST::setCondition(ConditionAST *condition)
{
    _M_condition = condition;
    if (_M_condition) _M_condition->setParent(this);
}

void ForStatementAST::setExpression(AbstractExpressionAST *expression)
{
    _M_expression = expression;
    if (_M_expression) _M_expression->setParent(this);
}

void ForStatementAST::setStatement(StatementAST *statement)
{
    _M_statement = statement;
    if (_M_statement) _M_statement->setParent(this);
}

void ForStatementAST::setInitStatement(StatementAST *initStatement)
{
    _M_initStatement = initStatement;
    if (_M_initStatement) _M_initStatement->setParent(this);
}

// --------------------------------------------------------------------------
void SwitchStatementAST::setCondition(ConditionAST *condition)
{
    _M_condition = condition;
    if (_M_condition) _M_condition->setParent(this);
}

void SwitchStatementAST::setStatement(StatementAST *statement)
{
    _M_statement = statement;
    if (_M_statement) _M_statement->setParent(this);
}

// --------------------------------------------------------------------------
void DeclarationStatementAST::setDeclaration(DeclarationAST *declaration)
{
    _M_declaration = declaration;
    if (_M_declaration) _M_declaration->setParent(this);
}

// --------------------------------------------------------------------------
void ExpressionStatementAST::setExpression(AbstractExpressionAST *expression)
{
    _M_expression = expression;
    if (_M_expression) _M_expression->setParent(this);
}

// --------------------------------------------------------------------------
void ExpressionOrDeclarationStatementAST::setExpression(StatementAST *expression)
{
    _M_expression = expression;
    if (_M_expression) _M_expression->setParent(this);
}

void ExpressionOrDeclarationStatementAST::setDeclaration(StatementAST *declaration)
{
    _M_declaration = declaration;
    if (_M_declaration) _M_declaration->setParent(this);
}

// --------------------------------------------------------------------------
void ParameterDeclarationAST::setTypeSpec(TypeSpecifierAST *typeSpec)
{
    _M_typeSpec = typeSpec;
    if (_M_typeSpec) _M_typeSpec->setParent(this);
}

void ParameterDeclarationAST::setDeclarator(DeclaratorAST *declarator)
{
    _M_declarator = declarator;
    if (_M_declarator) _M_declarator->setParent(this);
}

void ParameterDeclarationAST::setExpression(AbstractExpressionAST *expression)
{
    _M_expression = expression;
    if (_M_expression) _M_expression->setParent(this);
}

// --------------------------------------------------------------------------
void ParameterDeclarationListAST::addParameter(ParameterDeclarationAST *parameter)
{
    if(!parameter)
        return;

    parameter->setParent(this);
    _M_parameterList = snoc(_M_parameterList, parameter, _pool);
}

// --------------------------------------------------------------------------
void ParameterDeclarationClauseAST::setParameterDeclarationList(ParameterDeclarationListAST *parameterDeclarationList)
{
    _M_parameterDeclarationList = parameterDeclarationList;
    if (_M_parameterDeclarationList) _M_parameterDeclarationList->setParent(this);
}

void ParameterDeclarationClauseAST::setEllipsis(AST *ellipsis)
{
    _M_ellipsis = ellipsis;
    if (_M_ellipsis) _M_ellipsis->setParent(this);
}

// --------------------------------------------------------------------------
void AccessDeclarationAST::addAccess(AST *access)
{
    if(!access)
        return;

    access->setParent(this);
    _M_accessList = snoc(_M_accessList, access, _pool);
}

// --------------------------------------------------------------------------
void TypeParameterAST::setKind(AST *kind)
{
    _M_kind = kind;
}

void TypeParameterAST::setTemplateParameterList(TemplateParameterListAST *templateParameterList)
{
    _M_templateParameterList = templateParameterList;
    if (_M_templateParameterList) _M_templateParameterList->setParent(this);
}

void TypeParameterAST::setName(NameAST *name)
{
    _M_name = name;
    if (_M_name) _M_name->setParent(this);
}

void TypeParameterAST::setTypeId(AST *typeId)
{
    _M_typeId = typeId;
    if (_M_typeId) _M_typeId->setParent(this);
}

// --------------------------------------------------------------------------
void TemplateParameterAST::setTypeParameter(TypeParameterAST *typeParameter)
{
    _M_typeParameter = typeParameter;
    if (_M_typeParameter) _M_typeParameter->setParent(this);
}

void TemplateParameterAST::setTypeValueParameter(ParameterDeclarationAST *typeValueParameter)
{
    _M_typeValueParameter = typeValueParameter;
    if (_M_typeValueParameter) _M_typeValueParameter->setParent(this);
}

// --------------------------------------------------------------------------
void TemplateParameterListAST::addTemplateParameter(TemplateParameterAST *templateParameter)
{
    if(!templateParameter)
        return;

    templateParameter->setParent(this);
    _M_templateParameterList = snoc(_M_templateParameterList, templateParameter, _pool);
}

// --------------------------------------------------------------------------
void ConditionAST::setTypeSpec(TypeSpecifierAST *typeSpec)
{
    _M_typeSpec = typeSpec;
    if (_M_typeSpec) _M_typeSpec->setParent(this);
}

void ConditionAST::setDeclarator(DeclaratorAST *declarator)
{
    _M_declarator = declarator;
    if (_M_declarator) _M_declarator->setParent(this);
}

void ConditionAST::setExpression(AbstractExpressionAST *expression)
{
    _M_expression = expression;
    if (_M_expression) _M_expression->setParent(this);
}

void ClassSpecifierAST::setWinDeclSpec(AST *winDeclSpec)
{
    _M_winDeclSpec = winDeclSpec;
    if (_M_winDeclSpec) _M_winDeclSpec->setParent(this);
}

// --------------------------------------------------------------------------
void ReturnStatementAST::setExpression(AbstractExpressionAST *expression)
{
    _M_expression = expression;
    if (_M_expression) _M_expression->setParent(this);
}

// --------------------------------------------------------------------------
void BinaryExpressionAST::setOp(AST *op)
{
    _M_op = op;
    if (_M_op)
        _M_op->setParent(this);
}

void BinaryExpressionAST::setLeftExpression(AbstractExpressionAST *left)
{
    _M_left = left;
    if (_M_left)
        _M_left->setParent(this);
}

void BinaryExpressionAST::setRightExpression(AbstractExpressionAST *right)
{
    _M_right = right;
    if (_M_right)
        _M_right->setParent(this);
}

// --------------------------------------------------------------------------
void ConditionalExpressionAST::setCondition(AbstractExpressionAST *condition)
{
    _M_condition = condition;
    if (_M_condition)
        _M_condition->setParent(this);
}

void ConditionalExpressionAST::setLeftExpression(AbstractExpressionAST *left)
{
    _M_left = left;
    if (_M_left)
        _M_left->setParent(this);
}

void ConditionalExpressionAST::setRightExpression(AbstractExpressionAST *right)
{
    _M_right = right;
    if (_M_right)
        _M_right->setParent(this);
}

// --------------------------------------------------------------------------
void CppCastExpressionAST::setCastOp(AST *castOp)
{
    _M_castOp = castOp;
    if (_M_castOp)
        _M_castOp->setParent(this);
}

void CppCastExpressionAST::setTypeId(AST *typeId)
{
    _M_typeId = typeId;
    if (_M_typeId)
        _M_typeId->setParent(this);
}

void CppCastExpressionAST::setExpression(AbstractExpressionAST *expression)
{
    _M_expression = expression;
    if (_M_expression)
        _M_expression->setParent(this);
}

// --------------------------------------------------------------------------
void SubscriptingAST::setSubscript(AbstractExpressionAST *subscript)
{
    _M_subscript = subscript;
    if (_M_subscript)
        _M_subscript->setParent(this);
}

void SubscriptingAST::setExpression(AbstractExpressionAST *expression)
{
    _M_expression = expression;
    if (_M_expression)
        _M_expression->setParent(this);
}

// --------------------------------------------------------------------------
void FunctionCallAST::setExpression(AbstractExpressionAST *expression)
{
    _M_expression = expression;
    if (_M_expression)
        _M_expression->setParent(this);
}

void FunctionCallAST::setArguments(AbstractExpressionAST *arguments)
{
    _M_arguments = arguments;
    if (_M_arguments)
        _M_arguments->setParent(this);
}

// --------------------------------------------------------------------------
void ClassMemberAccessAST::setOp(AST *op)
{
    _M_op = op;
    if (_M_op)
        _M_op->setParent(this);
}

void ClassMemberAccessAST::setExpression(AbstractExpressionAST *expression)
{
    _M_expression = expression;
    if (_M_expression)
        _M_expression->setParent(this);
}

void ClassMemberAccessAST::setName(NameAST *name)
{
    _M_name = name;
    if (_M_name)
        _M_name->setParent(this);
}

// --------------------------------------------------------------------------
void IncrDecrAST::setOp(AST *op)
{
    _M_op = op;
    if (_M_op)
        _M_op->setParent(this);
}

void IncrDecrAST::setExpression(AbstractExpressionAST *expression)
{
    _M_expression = expression;
    if (_M_expression)
        _M_expression->setParent(this);
}

// --------------------------------------------------------------------------
void TypeIdAST::setTypeSpecifier(TypeSpecifierAST *typeSpecifier)
{
    _M_typeSpecifier = typeSpecifier;
    if (_M_typeSpecifier)
        _M_typeSpecifier->setParent(this);
}

void TypeIdAST::setDeclarator(DeclaratorAST *declarator)
{
    _M_declarator = declarator;
    if (_M_declarator)
        _M_declarator->setParent(this);
}

