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

#ifndef __ast_h
#define __ast_h

#include "smallobject.h"
#include "list.h"

#include <cassert>

class AST;
class NameAST;
class TypeIdAST;
class TypeSpecifierAST;
class DeclaratorAST;

class Symbol;
class Scope;

enum NodeType
{
    NodeType_Generic = 0,

    NodeType_TemplateArgumentList = 1000,
    NodeType_ClassOrNamespaceName,
    NodeType_Name,
    NodeType_Declaration,
    NodeType_TypeSpecifier,
    NodeType_BaseSpecifier,
    NodeType_BaseClause,
    NodeType_ClassSpecifier,
    NodeType_Enumerator,
    NodeType_EnumSpecifier,
    NodeType_ElaboratedTypeSpecifier,
    NodeType_LinkageBody,
    NodeType_LinkageSpecification,
    NodeType_Namespace,
    NodeType_NamespaceAlias,
    NodeType_Using,
    NodeType_UsingDirective,
    NodeType_InitDeclaratorList,
    NodeType_Typedef,
    NodeType_Declarator,
    NodeType_InitDeclarator,
    NodeType_TemplateDeclaration,
    NodeType_SimpleDeclaration,
    NodeType_Statement,
    NodeType_StatementList,
    NodeType_IfStatement,
    NodeType_WhileStatement,
    NodeType_DoStatement,
    NodeType_ForStatement,
    NodeType_SwitchStatement,
    NodeType_DeclarationStatement,
    NodeType_ReturnStatement,
    NodeType_TranslationUnit,
    NodeType_FunctionDefinition,
    NodeType_ExpressionOrDeclarationStatement,
    NodeType_ExpressionStatement,
    NodeType_ParameterDeclaration,
    NodeType_ParameterDeclarationList,
    NodeType_ParameterDeclarationClause,
    NodeType_AccessDeclaration,
    NodeType_TypeParameter,
    NodeType_TemplateParameter,
    NodeType_TemplateParameterList,
    NodeType_Condition,

    NodeType_TypeId,

    NodeType_Expression = 2000,
    NodeType_BinaryExpression,
    NodeType_PrimaryExpression,

//
// postfix expression
//
    NodeType_PostfixExpression,
    NodeType_Subscripting,
    NodeType_FunctionCall,
    NodeType_ExplicitTypeConversion,
    NodeType_PseudoConstructorCall,
    NodeType_ClassMemberAccess,
    NodeType_IncrDecr,
    NodeType_CppCastExpression,
    NodeType_TypeIdentification,

    NodeType_UnaryExpression,
    NodeType_NewExpression,
    NodeType_NewTypeId,
    NodeType_NewDeclarator,
    NodeType_NewInitializer,
    NodeType_DeleteExpression,
    NodeType_CastExpression,
    NodeType_ConditionalExpression,
    NodeType_ThrowExpression,

    NodeType_Custom = 3000
};


class AST
{
public:
    enum { Type=NodeType_Generic };

    pool *_pool;

public:
    inline int startToken() const
    { return _M_startToken; }

    inline int endToken() const
    { return _M_endToken; }

    inline void setPosition(int startToken, int endToken)
    {
        _M_startToken = startToken;
        _M_endToken = endToken;
    }

    inline int nodeType() const
    { return _M_nodeType; }

    inline void setNodeType(int nodeType)
    { _M_nodeType = nodeType; }

    inline AST *parent() const
    { return _M_parent; }

    void setParent(AST *parent);

    inline const ListNode<AST*> *children() const
    { return _M_children; }

    void appendChild(AST *node);

private:
    int _M_nodeType;
    int _M_startToken;
    int _M_endToken;
    AST *_M_parent;
    const ListNode<AST*> *_M_children;
};

class AbstractExpressionAST: public AST
{
public:
    enum { Type = NodeType_Expression };

public:
    inline Symbol *symbol() const
    { return _M_symbol; }

    inline void setSymbol(Symbol *symbol)
    { _M_symbol = symbol; }

private:
    Symbol *_M_symbol;
};

// ### remove me
template <int kind, class Base = AbstractExpressionAST>
class ExpressionAST: public Base
{
public:
    enum { Type = kind };
};

class BinaryExpressionAST: public AbstractExpressionAST
{
public:
    enum { Type = NodeType_BinaryExpression };

public:
    inline AST *op() const
    { return _M_op; }

    inline AbstractExpressionAST *leftExpression() const
    { return _M_left; }

    inline AbstractExpressionAST *rightExpression() const
    { return _M_right; }

    void setOp(AST *op);
    void setLeftExpression(AbstractExpressionAST *left);
    void setRightExpression(AbstractExpressionAST *right);

private:
    AST *_M_op;
    AbstractExpressionAST *_M_left;
    AbstractExpressionAST *_M_right;
};

class ConditionalExpressionAST: public AbstractExpressionAST
{
public:
    enum { Type = NodeType_ConditionalExpression };

public:
    inline AbstractExpressionAST *condition() const
    { return _M_condition; }

    inline AbstractExpressionAST *leftExpression() const
    { return _M_left; }

    inline AbstractExpressionAST *rightExpression() const
    { return _M_right; }

    void setCondition(AbstractExpressionAST *condition);
    void setLeftExpression(AbstractExpressionAST *left);
    void setRightExpression(AbstractExpressionAST *right);

private:
    AbstractExpressionAST *_M_condition;
    AbstractExpressionAST *_M_left;
    AbstractExpressionAST *_M_right;
};


//
// postfix expression
//

class SubscriptingAST: public AbstractExpressionAST
{
public:
    enum { Type = NodeType_Subscripting };

public:
    inline AbstractExpressionAST *expression() const
    { return _M_expression; }

    inline AbstractExpressionAST *subscript() const
    { return _M_subscript; }

    void setExpression(AbstractExpressionAST *expression);
    void setSubscript(AbstractExpressionAST *subscript);

private:
    AbstractExpressionAST *_M_expression;
    AbstractExpressionAST *_M_subscript;
};

class FunctionCallAST: public AbstractExpressionAST
{
public:
    enum { Type = NodeType_FunctionCall };

public:
    inline AbstractExpressionAST *expression() const
    { return _M_expression; }

    inline AbstractExpressionAST *arguments() const
    { return _M_arguments; }

    void setExpression(AbstractExpressionAST *expression);
    void setArguments(AbstractExpressionAST *arguments);

private:
    AbstractExpressionAST *_M_expression;
    AbstractExpressionAST *_M_arguments;
};

class ExplicitTypeConversionAST: public AbstractExpressionAST
{
public:
    enum { Type = NodeType_ExplicitTypeConversion };
};

class PseudoDestructorCallAST: public AbstractExpressionAST
{
public:
    enum { Type = NodeType_PseudoConstructorCall };
};

class ClassMemberAccessAST: public AbstractExpressionAST
{
public:
    enum { Type = NodeType_ClassMemberAccess };

public:
    inline AST *op() const
    { return _M_op; }

    inline AbstractExpressionAST *expression() const
    { return _M_expression; }

    inline NameAST *name() const
    { return _M_name; }

    void setOp(AST *op);
    void setExpression(AbstractExpressionAST *expression);
    void setName(NameAST *name);

private:
    AST *_M_op;
    AbstractExpressionAST *_M_expression;
    AST *_M_templ;
    NameAST *_M_name;
};

class IncrDecrAST: public AbstractExpressionAST
{
public:
    enum { Type = NodeType_IncrDecr };

public:
    inline AST *op() const
    { return _M_op; }

    inline AbstractExpressionAST *expression() const
    { return _M_expression; }

    void setOp(AST *op);
    void setExpression(AbstractExpressionAST *expression);

private:
    AST *_M_op;
    AbstractExpressionAST *_M_expression;
};

class CppCastExpressionAST: public AbstractExpressionAST
{
public:
    enum { Type = NodeType_CppCastExpression };

public:
    inline AST *castOp() const
    { return _M_castOp; }

    inline AST *typeId() const
    { return _M_typeId; }

    inline AbstractExpressionAST *expression() const
    { return _M_expression; }

    void setCastOp(AST *castOp);
    void setTypeId(AST *typeId);
    void setExpression(AbstractExpressionAST *expression);

private:
    AST *_M_castOp;
    AST *_M_typeId;
    AbstractExpressionAST *_M_expression;
};

class TypeIdentificationAST: public AbstractExpressionAST
{
public:
    enum { Type = NodeType_TypeIdentification };
};

class TypeIdAST: public AST
{
public:
    enum { Type = NodeType_TypeId };

public:
    inline TypeSpecifierAST *typeSpecifier() const
    { return _M_typeSpecifier; }

    inline DeclaratorAST *declarator() const
    { return _M_declarator; }

    void setTypeSpecifier(TypeSpecifierAST *typeSpecifier);
    void setDeclarator(DeclaratorAST *declarator);

private:
    TypeSpecifierAST *_M_typeSpecifier;
    DeclaratorAST *_M_declarator;
};

class StatementAST: public AST
{
public:
    enum { Type = NodeType_Statement };
};

class TemplateArgumentListAST: public AST
{
public:
    enum { Type = NodeType_TemplateArgumentList };

public:
    void addArgument(AST *arg);
    inline const ListNode<AST *> *argumentList() const { return _M_argumentList; }

private:
    const ListNode<AST *> *_M_argumentList;
};

class ClassOrNamespaceNameAST: public AST
{
public:
    enum { Type = NodeType_ClassOrNamespaceName };

public:
    inline AST *name() const { return _M_name; }
    void setName(AST *name);

    inline TemplateArgumentListAST *templateArgumentList() const { return _M_templateArgumentList; }
    void setTemplateArgumentList(TemplateArgumentListAST *templateArgumentList);

private:
    AST* _M_name;
    TemplateArgumentListAST* _M_templateArgumentList;
};

class NameAST: public AST
{
public:
    enum { Type = NodeType_Name };

public:
    inline bool isGlobal() const { return _M_global; }
    void setGlobal(bool b);

    void addClassOrNamespaceName(ClassOrNamespaceNameAST *classOrNamespaceName);
    inline const ListNode<ClassOrNamespaceNameAST *> *classOrNamespaceNameList() const { return _M_classOrNamespaceNameList; }

    inline ClassOrNamespaceNameAST *unqualifiedName() const { return _M_unqualifiedName; }
    void setUnqualifiedName(ClassOrNamespaceNameAST *unqualifiedName);

private:
    bool _M_global;
    ClassOrNamespaceNameAST* _M_unqualifiedName;
    const ListNode<ClassOrNamespaceNameAST *> *_M_classOrNamespaceNameList;
};

class TypeParameterAST: public AST
{
public:
    enum { Type = NodeType_TypeParameter };

public:
    inline AST *kind() const { return _M_kind; }
    void setKind(AST *kind);

    inline class TemplateParameterListAST *templateParameterList() const { return _M_templateParameterList; }
    void setTemplateParameterList(class TemplateParameterListAST *templateParameterList);

    inline NameAST *name() const { return _M_name; }
    void setName(NameAST *name);

    inline AST *typeId() const { return _M_typeId; }
    void setTypeId(AST *typeId);

private:
    AST* _M_kind;
    class TemplateParameterListAST *_M_templateParameterList;
    NameAST* _M_name;
    AST* _M_typeId;
};

class DeclarationAST: public AST
{
public:
    enum { Type = NodeType_Declaration };
};

class AccessDeclarationAST: public DeclarationAST
{
public:
    enum { Type = NodeType_AccessDeclaration };

public:
    inline const ListNode<AST *> *accessList() const { return _M_accessList; }
    void addAccess(AST *access);

private:
    const ListNode<AST *> *_M_accessList;
};

class TypeSpecifierAST: public AST
{
public:
    enum { Type = NodeType_TypeSpecifier };

public:
    inline NameAST *name() const { return _M_name; }
    void setName(NameAST *name);

    inline AST *cvQualify() const { return _M_cvQualify; }
    void setCvQualify(AST *cvQualify);

    inline AST *cv2Qualify() const { return _M_cv2Qualify; }
    void setCv2Qualify(AST *cv2Qualify);

private:
    NameAST* _M_name;
    AST* _M_cvQualify;
    AST* _M_cv2Qualify;
};

class BaseSpecifierAST: public AST
{
public:
    enum { Type = NodeType_BaseSpecifier };

public:
    inline AST *isVirtual() const { return _M_isVirtual; }
    void setIsVirtual(AST *isVirtual);

    inline AST *access() const { return _M_access; }
    void setAccess(AST *access);

    inline NameAST *name() const { return _M_name; }
    void setName(NameAST *name);

private:
    AST* _M_isVirtual;
    AST* _M_access;
    NameAST* _M_name;
};

class BaseClauseAST: public AST
{
public:
    enum { Type = NodeType_BaseClause };

public:
    inline const ListNode<BaseSpecifierAST *> *baseSpecifierList() const { return _M_baseSpecifierList; }
    void addBaseSpecifier(BaseSpecifierAST *baseSpecifier);

private:
    const ListNode<BaseSpecifierAST *> *_M_baseSpecifierList;
};

class ClassSpecifierAST: public TypeSpecifierAST
{
public:
    enum { Type = NodeType_ClassSpecifier };

public:
    inline AST *winDeclSpec() const { return _M_winDeclSpec; }
    void setWinDeclSpec(AST *winDeclSpec);

    inline AST *classKey() const { return _M_classKey; }
    void setClassKey(AST *classKey);

    inline BaseClauseAST *baseClause() const { return _M_baseClause; }
    void setBaseClause(BaseClauseAST *baseClause);

    inline const ListNode<DeclarationAST *> *declarationList() const { return _M_declarationList; }
    void addDeclaration(DeclarationAST *declaration);

private:
    AST* _M_winDeclSpec;
    AST* _M_classKey;
    BaseClauseAST* _M_baseClause;
    const ListNode<DeclarationAST *> *_M_declarationList;
};

class EnumeratorAST: public AST
{
public:
    enum { Type = NodeType_Enumerator };

public:
    inline AST *id() const { return _M_id; }
    void setId(AST *id);

    inline AbstractExpressionAST *expression() const { return _M_expression; }
    void setExpression(AbstractExpressionAST *expr);

private:
    AST* _M_id;
    AbstractExpressionAST* _M_expression;
};

class EnumSpecifierAST: public TypeSpecifierAST
{
public:
    enum { Type = NodeType_EnumSpecifier };

public:
    void addEnumerator(EnumeratorAST *enumerator);
    inline const ListNode<EnumeratorAST *> *enumeratorList() const { return _M_enumeratorList; }

private:
    const ListNode<EnumeratorAST *> *_M_enumeratorList;
};

class ElaboratedTypeSpecifierAST: public TypeSpecifierAST
{
public:
    enum { Type = NodeType_ElaboratedTypeSpecifier };

public:
    inline AST *kind() const { return _M_kind; }
    void setKind(AST *kind);

private:
    AST* _M_kind;
};


class LinkageBodyAST: public AST
{
public:
    enum { Type = NodeType_LinkageBody };

public:
    void addDeclaration(DeclarationAST *ast);
    inline const ListNode<DeclarationAST *> *declarationList() const { return _M_declarationList; }

private:
    const ListNode<DeclarationAST *> *_M_declarationList;
};

class LinkageSpecificationAST: public DeclarationAST
{
public:
    enum { Type = NodeType_LinkageSpecification };

public:
    inline AST *externType() const { return _M_externType; }
    void setExternType(AST *externType);

    inline LinkageBodyAST *linkageBody() const { return _M_linkageBody; }
    void setLinkageBody(LinkageBodyAST *linkageBody);

    inline DeclarationAST *declaration() const { return _M_declaration; }
    void setDeclaration(DeclarationAST *decl);

private:
    AST* _M_externType;
    LinkageBodyAST* _M_linkageBody;
    DeclarationAST* _M_declaration;
};

class NamespaceAST: public DeclarationAST
{
public:
    enum { Type = NodeType_Namespace };

public:
    inline AST *namespaceName() const { return _M_namespaceName; }
    void setNamespaceName(AST *namespaceName);

    inline LinkageBodyAST *linkageBody() const { return _M_linkageBody; }
    void setLinkageBody(LinkageBodyAST *linkageBody);

private:
    AST* _M_namespaceName;
    LinkageBodyAST* _M_linkageBody;
};

class NamespaceAliasAST: public DeclarationAST
{
public:
    enum { Type = NodeType_NamespaceAlias };

public:
    inline AST *namespaceName() const { return _M_namespaceName; }
    void setNamespaceName(AST *name);

    inline NameAST *aliasName() const { return _M_aliasName; }
    void setAliasName(NameAST *name);

private:
    AST* _M_namespaceName;
    NameAST* _M_aliasName;
};

class UsingAST: public DeclarationAST
{
public:
    enum { Type = NodeType_Using };

public:
    inline AST *typeName() const { return _M_typeName; }
    void setTypeName(AST *typeName);

    inline NameAST *name() const { return _M_name; }
    void setName(NameAST *name);

private:
    AST* _M_typeName;
    NameAST* _M_name;
};

class UsingDirectiveAST: public DeclarationAST
{
public:
    enum { Type = NodeType_UsingDirective };

public:
    inline NameAST *name() const { return _M_name; }
    void setName(NameAST *name);

private:
    NameAST* _M_name;
};

class DeclaratorAST: public AST
{
public:
    enum { Type = NodeType_Declarator };

public:
    inline const ListNode<AST *> *ptrOpList() const { return _M_ptrOpList; }
    void addPtrOp(AST *ptrOp);

    inline DeclaratorAST *subDeclarator() const { return _M_subDeclarator; }
    void setSubDeclarator(DeclaratorAST *subDeclarator);

    inline NameAST *declaratorId() const { return _M_declaratorId; }
    void setDeclaratorId(NameAST *declaratorId);

    inline AST *bitfieldInitialization() const { return _M_bitfieldInitialization; }
    void setBitfieldInitialization(AST *bitfieldInitialization);

    inline const ListNode<AST *> *arrayDimensionList() const { return _M_arrayDimensionList; }
    void addArrayDimension(AST *arrayDimension);

    inline class ParameterDeclarationClauseAST *parameterDeclarationClause() const { return _M_parameterDeclarationClause; }
    void setParameterDeclarationClause(class ParameterDeclarationClauseAST  *parameterDeclarationClause);

    // ### replace 'constant' with cvQualify
    inline AST *constant() const { return _M_constant; }
    void setConstant(AST *constant);

    inline AST *exceptionSpecification() const { return _M_exceptionSpecification; }
    void setExceptionSpecification(AST *exceptionSpecification);

private:
    const ListNode<AST *> *_M_ptrOpList;
    DeclaratorAST * _M_subDeclarator;
    NameAST* _M_declaratorId;
    AST* _M_bitfieldInitialization;
    const ListNode<AST *> *_M_arrayDimensionList;
    class ParameterDeclarationClauseAST * _M_parameterDeclarationClause;
    AST* _M_constant;
    AST* _M_exceptionSpecification;
};

class ParameterDeclarationAST: public AST
{
public:
    enum { Type = NodeType_ParameterDeclaration };

public:
    inline TypeSpecifierAST *typeSpec() const { return _M_typeSpec; }
    void setTypeSpec(TypeSpecifierAST *typeSpec);

    inline DeclaratorAST *declarator() const { return _M_declarator; }
    void setDeclarator(DeclaratorAST *declarator);

    inline AbstractExpressionAST *expression() const { return _M_expression; }
    void setExpression(AbstractExpressionAST *expression);

private:
    TypeSpecifierAST* _M_typeSpec;
    DeclaratorAST* _M_declarator;
    AbstractExpressionAST* _M_expression;
};

class ParameterDeclarationListAST: public AST
{
public:
    enum { Type = NodeType_ParameterDeclarationList };

public:
    inline const ListNode<ParameterDeclarationAST *> *parameterList() const { return _M_parameterList; }
    void addParameter(ParameterDeclarationAST *parameter);

private:
    const ListNode<ParameterDeclarationAST *> *_M_parameterList;
};

class ParameterDeclarationClauseAST: public AST
{
public:
    enum { Type = NodeType_ParameterDeclarationClause };

public:
    inline ParameterDeclarationListAST *parameterDeclarationList() const { return _M_parameterDeclarationList; }
    void setParameterDeclarationList(ParameterDeclarationListAST *parameterDeclarationList);

    inline AST *ellipsis() const { return _M_ellipsis; }
    void setEllipsis(AST *ellipsis);

private:
    ParameterDeclarationListAST* _M_parameterDeclarationList;
    AST* _M_ellipsis;
};


class InitDeclaratorAST: public AST
{
public:
    enum { Type = NodeType_InitDeclarator };

public:
    inline DeclaratorAST *declarator() const { return _M_declarator; }
    void setDeclarator(DeclaratorAST *declarator);

    inline AST *initializer() const { return _M_initializer; }
    void setInitializer(AST *initializer);

private:
    DeclaratorAST* _M_declarator;
    AST* _M_initializer;
};

class InitDeclaratorListAST: public AST
{
public:
    enum { Type = NodeType_InitDeclaratorList };

public:
    inline const ListNode<InitDeclaratorAST *> *initDeclaratorList() const { return _M_initDeclaratorList; }
    void addInitDeclarator(InitDeclaratorAST *decl);

private:
    const ListNode<InitDeclaratorAST *> *_M_initDeclaratorList;
};

class TypedefAST: public DeclarationAST
{
public:
    enum { Type = NodeType_Typedef };

public:
    inline TypeSpecifierAST *typeSpec() const { return _M_typeSpec; }
    void setTypeSpec(TypeSpecifierAST *typeSpec);

    inline InitDeclaratorListAST *initDeclaratorList() const { return _M_initDeclaratorList; }
    void setInitDeclaratorList(InitDeclaratorListAST *initDeclaratorList);

private:
    TypeSpecifierAST* _M_typeSpec;
    InitDeclaratorListAST* _M_initDeclaratorList;
};

class TemplateParameterAST: public AST
{
public:
    enum { Type = NodeType_TemplateParameter };

public:
    inline TypeParameterAST *typeParameter() const { return _M_typeParameter; }
    void setTypeParameter(TypeParameterAST *typeParameter);

    inline ParameterDeclarationAST *typeValueParameter() const { return _M_typeValueParameter; }
    void setTypeValueParameter(ParameterDeclarationAST *typeValueParameter);

private:
    TypeParameterAST* _M_typeParameter;
    ParameterDeclarationAST* _M_typeValueParameter;
};

class TemplateParameterListAST: public AST
{
public:
    enum { Type = NodeType_TemplateParameterList };

public:
    inline const ListNode<TemplateParameterAST *> *templateParameterList() const { return _M_templateParameterList; }
    void addTemplateParameter(TemplateParameterAST *templateParameter);

private:
    const ListNode<TemplateParameterAST *> *_M_templateParameterList;
};

class TemplateDeclarationAST: public DeclarationAST
{
public:
    enum { Type = NodeType_TemplateDeclaration };

public:
    inline AST *exported() const { return _M_exported; }
    void setExported(AST *exported);

    inline TemplateParameterListAST *templateParameterList() const { return _M_templateParameterList; }
    void setTemplateParameterList(TemplateParameterListAST *templateParameterList);

    inline DeclarationAST *declaration() const { return _M_declaration; }
    void setDeclaration(DeclarationAST *declaration);

private:
    AST* _M_exported;
    TemplateParameterListAST* _M_templateParameterList;
    DeclarationAST* _M_declaration;
};

class SimpleDeclarationAST: public DeclarationAST
{
public:
    enum { Type = NodeType_SimpleDeclaration };

public:
    inline AST *functionSpecifier() const { return _M_functionSpecifier; }
    void setFunctionSpecifier(AST *functionSpecifier);

    inline AST *storageSpecifier() const { return _M_storageSpecifier; }
    void setStorageSpecifier(AST *storageSpecifier);

    inline TypeSpecifierAST *typeSpec() const { return _M_typeSpec; }
    void setTypeSpec(TypeSpecifierAST *typeSpec);

    inline InitDeclaratorListAST *initDeclaratorList() const { return _M_initDeclaratorList; }
    void setInitDeclaratorList(InitDeclaratorListAST *initDeclaratorList);

    inline AST *winDeclSpec() const { return _M_winDeclSpec; }
    void setWinDeclSpec(AST *winDeclSpec);

private:
    AST* _M_functionSpecifier;
    AST* _M_storageSpecifier;
    TypeSpecifierAST* _M_typeSpec;
    InitDeclaratorListAST* _M_initDeclaratorList;
    AST* _M_winDeclSpec;
};

class ExpressionStatementAST: public StatementAST
{
public:
    enum { Type = NodeType_ExpressionStatement };

public:
    inline AbstractExpressionAST *expression() const { return _M_expression; }
    void setExpression(AbstractExpressionAST *expression);

private:
    AbstractExpressionAST* _M_expression;
};

class ExpressionOrDeclarationStatementAST: public StatementAST
{
public:
    enum { Type = NodeType_ExpressionOrDeclarationStatement };

public:
    inline StatementAST *expression() const { return _M_expression; }
    void setExpression(StatementAST *expression);

    inline StatementAST *declaration() const { return _M_declaration; }
    void setDeclaration(StatementAST *declaration);

private:
    StatementAST* _M_expression;
    StatementAST*_M_declaration;
};

class ReturnStatementAST: public StatementAST
{
public:
    enum { Type = NodeType_ReturnStatement };

public:
    inline AbstractExpressionAST *expression() const { return _M_expression; }
    void setExpression(AbstractExpressionAST *expression);

private:
    AbstractExpressionAST* _M_expression;
};


class ConditionAST: public AST
{
public:
    enum { Type = NodeType_Condition };

public:
    inline TypeSpecifierAST *typeSpec() const { return _M_typeSpec; }
    void setTypeSpec(TypeSpecifierAST *typeSpec);

    inline DeclaratorAST *declarator() const { return _M_declarator; }
    void setDeclarator(DeclaratorAST *declarator);

    inline AbstractExpressionAST *expression() const { return _M_expression; }
    void setExpression(AbstractExpressionAST *expression);

private:
    TypeSpecifierAST* _M_typeSpec;
    DeclaratorAST* _M_declarator;
    AbstractExpressionAST* _M_expression;
};

class IfStatementAST: public StatementAST
{
public:
    enum { Type = NodeType_IfStatement };

public:
    inline ConditionAST *condition() const { return _M_condition; }
    void setCondition(ConditionAST *condition);

    inline StatementAST *statement() const { return _M_statement; }
    void setStatement(StatementAST *statement);

    inline StatementAST *elseStatement() const { return _M_elseStatement; }
    void setElseStatement(StatementAST *statement);

private:
    ConditionAST* _M_condition;
    StatementAST* _M_statement;
    StatementAST* _M_elseStatement;
};

class WhileStatementAST: public StatementAST
{
public:
    enum { Type = NodeType_WhileStatement };

public:
    inline ConditionAST *condition() const { return _M_condition; }
    void setCondition(ConditionAST *condition);

    inline StatementAST *statement() const { return _M_statement; }
    void setStatement(StatementAST *statement);

private:
    ConditionAST* _M_condition;
    StatementAST* _M_statement;
};

class DoStatementAST: public StatementAST
{
public:
    enum { Type = NodeType_DoStatement };

public:
    inline ConditionAST *condition() const { return _M_condition; }
    void setCondition(ConditionAST *condition);

    inline StatementAST *statement() const { return _M_statement; }
    void setStatement(StatementAST *statement);

private:
    ConditionAST* _M_condition;
    StatementAST* _M_statement;
};

class ForStatementAST: public StatementAST
{
public:
    enum { Type = NodeType_ForStatement };

public:
    inline StatementAST *initStatement() const { return _M_initStatement; }
    void setInitStatement(StatementAST *statement);

    inline ConditionAST *condition() const { return _M_condition; }
    void setCondition(ConditionAST *condition);

    inline AbstractExpressionAST *expression() const { return _M_expression; }
    void setExpression(AbstractExpressionAST *expression);

    inline StatementAST *statement() const { return _M_statement; }
    void setStatement(StatementAST *statement);

private:
    ConditionAST* _M_condition;
    StatementAST* _M_initStatement;
    StatementAST* _M_statement;
    AbstractExpressionAST* _M_expression;
};

class SwitchStatementAST: public StatementAST
{
public:
    enum { Type = NodeType_SwitchStatement };

public:
    inline ConditionAST *condition() const { return _M_condition; }
    void setCondition(ConditionAST *condition);

    inline StatementAST *statement() const { return _M_statement; }
    void setStatement(StatementAST *statement);

private:
    ConditionAST* _M_condition;
    StatementAST* _M_statement;
};

class StatementListAST: public StatementAST
{
public:
    enum { Type = NodeType_StatementList };

public:
    inline const ListNode<StatementAST *> *statementList() const { return _M_statementList; }
    void addStatement(StatementAST *statement);

private:
    const ListNode<StatementAST *> *_M_statementList;
};

class DeclarationStatementAST: public StatementAST
{
public:
    enum { Type = NodeType_DeclarationStatement };

public:
    inline DeclarationAST *declaration() const { return _M_declaration; }
    void setDeclaration(DeclarationAST *declaration);

private:
    DeclarationAST* _M_declaration;
};

class FunctionDefinitionAST: public DeclarationAST
{
public:
    enum { Type = NodeType_FunctionDefinition };

public:
    inline AST *functionSpecifier() const { return _M_functionSpecifier; }
    void setFunctionSpecifier(AST *functionSpecifier);

    inline AST *storageSpecifier() const { return _M_storageSpecifier; }
    void setStorageSpecifier(AST *storageSpecifier);

    inline TypeSpecifierAST *typeSpec() const { return _M_typeSpec; }
    void setTypeSpec(TypeSpecifierAST *typeSpec);

    inline InitDeclaratorAST *initDeclarator() const { return _M_initDeclarator; }
    void setInitDeclarator(InitDeclaratorAST *initDeclarator);

    inline StatementListAST *functionBody() const { return _M_functionBody; }
    void setFunctionBody(StatementListAST *functionBody);

    inline AST *winDeclSpec() const { return _M_winDeclSpec; }
    void setWinDeclSpec(AST *winDeclSpec);

private:
    AST* _M_functionSpecifier;
    AST* _M_storageSpecifier;
    TypeSpecifierAST* _M_typeSpec;
    InitDeclaratorAST* _M_initDeclarator;
    StatementListAST* _M_functionBody;
    AST* _M_winDeclSpec;
};

class TranslationUnitAST: public AST
{
public:
    enum { Type = NodeType_TranslationUnit };

public:
    void addDeclaration(DeclarationAST *ast);
    inline const ListNode<DeclarationAST *> *declarationList() const { return _M_declarationList; }

private:
    const ListNode<DeclarationAST *> *_M_declarationList;
};

template <class T> T* CreateNode(pool *p)
{
    T* node(new (p->allocate(sizeof(T))) T);
    assert(node->nodeType() == 0);
    assert(node->_pool == 0);
    node->setNodeType(T::Type);
    node->_pool = p;
    return node;
}

template <int kind> ExpressionAST<kind> *CreateExpression(pool *p)
{
    ExpressionAST<kind>* node(new (p->allocate(sizeof(ExpressionAST<kind>))) ExpressionAST<kind>);
    assert(node->nodeType() == 0);
    assert(node->_pool == 0);
    node->setNodeType(kind);
    node->_pool = p;
    return node;
}

#endif
