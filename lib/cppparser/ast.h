/* This file is part of KDevelop
    Copyright (C) 2002,2003 Roberto Raggi <roberto@kdevelop.org>

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

#include <memory>
#include <qstring.h>
#include <qptrlist.h>

template <class T> typename T::Node CreateNode()
{
    typename T::Node node( new T );
    node->setNodeType( T::Type );
    return node;
}

template <class T> typename T::Node NullNode()
{
    typename T::Node node;
    return node;
}

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
    NodeType_TranslationUnit,
    NodeType_FunctionDefinition,
    NodeType_ExpressionStatement,
    NodeType_ParameterDeclaration,
    NodeType_ParameterDeclarationList,
    NodeType_ParameterDeclarationClause,
    NodeType_Group,
    NodeType_AccessDeclaration,
    NodeType_TypeParameter,
    NodeType_TemplateParameter,
    NodeType_TemplateParameterList,
    NodeType_Condition,

    NodeType_Custom = 2000
};

QString nodeTypeToString( NodeType type );

class AST
{
public:
    typedef std::auto_ptr<AST> Node;
    enum { Type=NodeType_Generic };

public:
    AST();
    virtual ~AST();

    int nodeType() const { return m_nodeType; }
    void setNodeType( int nodeType ) { m_nodeType = nodeType; }

    AST* parent() { return m_parent; }
    void setParent( AST* parent );

    void setStartPosition( int line, int col );
    void getStartPosition( int* line, int* col ) const;

    void setEndPosition( int line, int col );
    void getEndPosition( int* line, int* col ) const;

    QPtrList<AST> children() { return m_children; }
    void appendChild( AST* child );
    void removeChild( AST* child );

    virtual QString text() const { return m_text; }
    virtual void setText( const QString& text ) { m_text = text; }

private:
    int m_nodeType;
    AST* m_parent;
    int m_startLine, m_startColumn;
    int m_endLine, m_endColumn;
    QString m_text;
    QPtrList<AST> m_children;

private:
    AST( const AST& source );
    void operator = ( const AST& source );
};

class GroupAST: public AST
{
public:
    typedef std::auto_ptr<GroupAST> Node;
    enum { Type = NodeType_Group };

public:
    GroupAST();

    QPtrList<AST> nodeList() { return m_nodeList; }
    void addNode( AST::Node& node );

    virtual QString text() const;

private:
    QPtrList<AST> m_nodeList;

private:
    GroupAST( const GroupAST& source );
    void operator = ( const GroupAST& source );
};


class TemplateArgumentListAST: public AST
{
public:
    typedef std::auto_ptr<TemplateArgumentListAST> Node;
    enum { Type = NodeType_TemplateArgumentList };

public:
    TemplateArgumentListAST();

    void addArgument( AST::Node& arg );
    QPtrList<AST> argumentList() { return m_argumentList; }

    virtual QString text() const;

private:
    QPtrList<AST> m_argumentList;

private:
    TemplateArgumentListAST( const TemplateArgumentListAST& source );
    void operator = ( const TemplateArgumentListAST& source );
};

class ClassOrNamespaceNameAST: public AST
{
public:
    typedef std::auto_ptr<ClassOrNamespaceNameAST> Node;
    enum { Type = NodeType_ClassOrNamespaceName };

public:
    ClassOrNamespaceNameAST();

    AST* name() { return m_name.get(); }
    void setName( AST::Node& name );

    TemplateArgumentListAST* templateArgumentList() { return m_templateArgumentList.get(); }
    void setTemplateArgumentList( TemplateArgumentListAST::Node& templateArgumentList );

    virtual QString text() const;

private:
    AST::Node m_name;
    TemplateArgumentListAST::Node m_templateArgumentList;

private:
    ClassOrNamespaceNameAST( const ClassOrNamespaceNameAST& source );
    void operator = ( const ClassOrNamespaceNameAST& source );
};

class NameAST: public AST
{
public:
    typedef std::auto_ptr<NameAST> Node;
    enum { Type = NodeType_Name };

public:
    NameAST();

    bool isGlobal() const { return m_global; }
    void setGlobal( bool b );

    void addClassOrNamespaceName( ClassOrNamespaceNameAST::Node& classOrNamespaceName );
    QPtrList<ClassOrNamespaceNameAST> classOrNamespaceNameList() { return m_classOrNamespaceNameList; }

    ClassOrNamespaceNameAST* unqualifiedName() { return m_unqualifiedName.get(); }
    void setUnqualifedName( ClassOrNamespaceNameAST::Node& unqualifiedName );

    virtual QString text() const;

private:
    bool m_global;
    ClassOrNamespaceNameAST::Node m_unqualifiedName;
    QPtrList<ClassOrNamespaceNameAST> m_classOrNamespaceNameList;

private:
    NameAST( const NameAST& source );
    void operator = ( const NameAST& source );
};

class TypeParameterAST: public AST
{
public:
    typedef std::auto_ptr<TypeParameterAST> Node;
    enum { Type = NodeType_TypeParameter };

public:
    TypeParameterAST();

    AST* kind() { return m_kind.get(); }
    void setKind( AST::Node& kind );

    class TemplateParameterListAST* templateParameterList() { return m_templateParameterList.get(); }
    void setTemplateParameterList( std::auto_ptr<class TemplateParameterListAST>& templateParameterList );

    NameAST* name() { return m_name.get(); }
    void setName( NameAST::Node& name );

    AST* typeId() { return m_typeId.get(); }
    void setTypeId( AST::Node& typeId );

private:
    AST::Node m_kind;
    std::auto_ptr<class TemplateParameterListAST> m_templateParameterList;
    NameAST::Node m_name;
    AST::Node m_typeId;

private:
    TypeParameterAST( const TypeParameterAST& source );
    void operator = ( const TypeParameterAST& source );
};

class DeclarationAST: public AST
{
public:
    typedef std::auto_ptr<DeclarationAST> Node;
    enum { Type = NodeType_Declaration };

public:
    DeclarationAST();

private:
    DeclarationAST( const DeclarationAST& source );
    void operator = ( const DeclarationAST& source );
};

class AccessDeclarationAST: public DeclarationAST
{
public:
    typedef std::auto_ptr<AccessDeclarationAST> Node;
    enum { Type = NodeType_AccessDeclaration };

public:
    AccessDeclarationAST();

    QPtrList<AST> accessList() { return m_accessList; }
    void addAccess( AST::Node& access );

    virtual QString text() const;

private:
    QPtrList<AST> m_accessList;

private:
    AccessDeclarationAST( const AccessDeclarationAST& source );
    void operator = ( const AccessDeclarationAST& source );
};

class TypeSpecifierAST: public AST
{
public:
    typedef std::auto_ptr<TypeSpecifierAST> Node;
    enum { Type = NodeType_TypeSpecifier };

public:
    TypeSpecifierAST();

    virtual NameAST* name() { return m_name.get(); }
    virtual void setName( NameAST::Node& name );

    GroupAST* cvQualify() { return m_cvQualify.get(); }
    void setCvQualify( GroupAST::Node& cvQualify );

    GroupAST* cv2Qualify() { return m_cv2Qualify.get(); }
    void setCv2Qualify( GroupAST::Node& cv2Qualify );

    virtual QString text() const;

private:
    NameAST::Node m_name;
    GroupAST::Node m_cvQualify;
    GroupAST::Node m_cv2Qualify;

private:
    TypeSpecifierAST( const TypeSpecifierAST& source );
    void operator = ( const TypeSpecifierAST& source );
};

class BaseSpecifierAST: public AST
{
public:
    typedef std::auto_ptr<BaseSpecifierAST> Node;
    enum { Type = NodeType_BaseSpecifier };

public:
    BaseSpecifierAST();

    AST* isVirtual() { return m_isVirtual.get(); }
    void setIsVirtual( AST::Node& isVirtual );

    AST* access() { return m_access.get(); }
    void setAccess( AST::Node& access );

    NameAST* name() { return m_name.get(); }
    void setName( NameAST::Node& name );

private:
    AST::Node m_isVirtual;
    AST::Node m_access;
    NameAST::Node m_name;

private:
    BaseSpecifierAST( const BaseSpecifierAST& source );
    void operator = ( const BaseSpecifierAST& source );
};

class BaseClauseAST: public AST
{
public:
    typedef std::auto_ptr<BaseClauseAST> Node;
    enum { Type = NodeType_BaseClause };

public:
    BaseClauseAST();

    void addBaseSpecifier( BaseSpecifierAST::Node& baseSpecifier );
    QPtrList<BaseSpecifierAST> baseSpecifierList() { return m_baseSpecifierList; }

private:
    QPtrList<BaseSpecifierAST> m_baseSpecifierList;

private:
    BaseClauseAST( const BaseClauseAST& source );
    void operator = ( const BaseClauseAST& source );
};

class ClassSpecifierAST: public TypeSpecifierAST
{
public:
    typedef std::auto_ptr<ClassSpecifierAST> Node;
    enum { Type = NodeType_ClassSpecifier };

public:
    ClassSpecifierAST();

    AST* classKey() { return m_classKey.get(); }
    void setClassKey( AST::Node& classKey );

    BaseClauseAST* baseClause() { return m_baseClause.get(); }
    void setBaseClause( BaseClauseAST::Node& baseClause );

    QPtrList<DeclarationAST> declarationList() { return m_declarationList; }
    void addDeclaration( DeclarationAST::Node& declaration );

private:
    AST::Node m_classKey;
    BaseClauseAST::Node m_baseClause;
    QPtrList<DeclarationAST> m_declarationList;

private:
    ClassSpecifierAST( const ClassSpecifierAST& source );
    void operator = ( const ClassSpecifierAST& source );
};

class EnumeratorAST: public AST
{
public:
    typedef std::auto_ptr<EnumeratorAST> Node;
    enum { Type = NodeType_Enumerator };

public:
    EnumeratorAST();

    AST* id() { return m_id.get(); }
    void setId( AST::Node& id );

    AST* expr() { return m_expr.get(); }
    void setExpr( AST::Node& expr );

private:
    AST::Node m_id;
    AST::Node m_expr;

private:
    EnumeratorAST( const EnumeratorAST& source );
    void operator = ( const EnumeratorAST& source );
};

class EnumSpecifierAST: public TypeSpecifierAST
{
public:
    typedef std::auto_ptr<EnumSpecifierAST> Node;
    enum { Type = NodeType_EnumSpecifier };

public:
    EnumSpecifierAST();

    void addEnumerator( EnumeratorAST::Node& enumerator );
    QPtrList<EnumeratorAST> enumeratorList() { return m_enumeratorList; }

private:
    QPtrList<EnumeratorAST> m_enumeratorList;

private:
    EnumSpecifierAST( const EnumSpecifierAST& source );
    void operator = ( const EnumSpecifierAST& source );
};

class ElaboratedTypeSpecifierAST: public TypeSpecifierAST
{
public:
    typedef std::auto_ptr<ElaboratedTypeSpecifierAST> Node;
    enum { Type = NodeType_ElaboratedTypeSpecifier };

public:
    ElaboratedTypeSpecifierAST();

    AST* kind() { return m_kind.get(); }
    void setKind( AST::Node& kind );

    virtual QString text() const;

private:
    AST::Node m_kind;

private:
    ElaboratedTypeSpecifierAST( const ElaboratedTypeSpecifierAST& source );
    void operator = ( const ElaboratedTypeSpecifierAST& source );
};


class LinkageBodyAST: public AST
{
public:
    typedef std::auto_ptr<LinkageBodyAST> Node;
    enum { Type = NodeType_LinkageBody };

public:
    LinkageBodyAST();

    void addDeclaration( DeclarationAST::Node& ast );
    QPtrList<DeclarationAST> declarationList() { return m_declarationList; }

private:
    QPtrList<DeclarationAST> m_declarationList;

private:
    LinkageBodyAST( const LinkageBodyAST& source );
    void operator = ( const LinkageBodyAST& source );
};

class LinkageSpecificationAST: public DeclarationAST
{
public:
    typedef std::auto_ptr<LinkageSpecificationAST> Node;
    enum { Type = NodeType_LinkageSpecification };

public:
    LinkageSpecificationAST();

    AST* externType() { return m_externType.get(); }
    void setExternType( AST::Node& externType );

    LinkageBodyAST* linkageBody() { return m_linkageBody.get(); }
    void setLinkageBody( LinkageBodyAST::Node& linkageBody );

    DeclarationAST* declaration() { return m_declaration.get(); }
    void setDeclaration( DeclarationAST::Node& decl );

private:
    AST::Node m_externType;
    LinkageBodyAST::Node m_linkageBody;
    DeclarationAST::Node m_declaration;

private:
    LinkageSpecificationAST( const LinkageSpecificationAST& source );
    void operator = ( const LinkageSpecificationAST& source );
};

class NamespaceAST: public DeclarationAST
{
public:
    typedef std::auto_ptr<NamespaceAST> Node;
    enum { Type = NodeType_Namespace };

public:
    NamespaceAST();

    AST* namespaceName() { return m_namespaceName.get(); }
    void setNamespaceName( AST::Node& namespaceName );

    LinkageBodyAST* linkageBody() { return m_linkageBody.get(); }
    void setLinkageBody( LinkageBodyAST::Node& linkageBody );

private:
    AST::Node m_namespaceName;
    LinkageBodyAST::Node m_linkageBody;

private:
    NamespaceAST( const NamespaceAST& source );
    void operator = ( const NamespaceAST& source );
};

class NamespaceAliasAST: public DeclarationAST
{
public:
    typedef std::auto_ptr<NamespaceAliasAST> Node;
    enum { Type = NodeType_NamespaceAlias };

public:
    NamespaceAliasAST();

    AST* namespaceName() { return m_namespaceName.get(); }
    void setNamespaceName( AST::Node& name );

    NameAST* aliasName() { return m_aliasName.get(); }
    void setAliasName( NameAST::Node& name );

private:
    AST::Node m_namespaceName;
    NameAST::Node m_aliasName;

private:
    NamespaceAliasAST( const NamespaceAliasAST& source );
    void operator = ( const NamespaceAliasAST& source );
};

class UsingAST: public DeclarationAST
{
public:
    typedef std::auto_ptr<UsingAST> Node;
    enum { Type = NodeType_Using };

public:
    UsingAST();

    AST* typeName() { return m_typeName.get(); }
    void setTypeName( AST::Node& typeName );

    NameAST* name() { return m_name.get(); }
    void setName( NameAST::Node& name );

private:
    AST::Node m_typeName;
    NameAST::Node m_name;

private:
    UsingAST( const UsingAST& source );
    void operator = ( const UsingAST& source );
};

class UsingDirectiveAST: public DeclarationAST
{
public:
    typedef std::auto_ptr<UsingDirectiveAST> Node;
    enum { Type = NodeType_UsingDirective };

public:
    UsingDirectiveAST();

    NameAST* name() { return m_name.get(); }
    void setName( NameAST::Node& name );

private:
    NameAST::Node m_name;

private:
    UsingDirectiveAST( const UsingDirectiveAST& source );
    void operator = ( const UsingDirectiveAST& source );
};

class DeclaratorAST: public AST
{
public:
    typedef std::auto_ptr<DeclaratorAST> Node;
    enum { Type = NodeType_Declarator };

public:
    DeclaratorAST();

    QPtrList<AST> ptrOpList() { return m_ptrOpList; }
    void addPtrOp( AST::Node& ptrOp );

    DeclaratorAST* subDeclarator() { return m_subDeclarator.get(); }
    void setSubDeclarator( std::auto_ptr<DeclaratorAST>& subDeclarator );

    NameAST* declaratorId() { return m_declaratorId.get(); }
    void setDeclaratorId( NameAST::Node& declaratorId );

    AST* bitfieldInitialization() { return m_bitfieldInitialization.get(); }
    void setBitfieldInitialization( AST::Node& bitfieldInitialization );

    QPtrList<AST> arrayDimensionList() { return m_arrayDimensionList; }
    void addArrayDimension( AST::Node& arrayDimension );

    class ParameterDeclarationClauseAST* parameterDeclarationClause() { return m_parameterDeclarationClause.get(); }
    void setParameterDeclarationClause( std::auto_ptr<class ParameterDeclarationClauseAST>& parameterDeclarationClause );

    // ### replace 'constant' with cvQualify
    AST* constant() { return m_constant.get(); }
    void setConstant( AST::Node& constant );

    GroupAST* exceptionSpecification() { return m_exceptionSpecification.get(); }
    void setExceptionSpecification( GroupAST::Node& exceptionSpecification );

private:
    QPtrList<AST> m_ptrOpList;
    std::auto_ptr<DeclaratorAST> m_subDeclarator;
    NameAST::Node m_declaratorId;
    AST::Node m_bitfieldInitialization;
    QPtrList<AST> m_arrayDimensionList;
    std::auto_ptr<class ParameterDeclarationClauseAST> m_parameterDeclarationClause;
    AST::Node m_constant;
    GroupAST::Node m_exceptionSpecification;

private:
    DeclaratorAST( const DeclaratorAST& source );
    void operator = ( const DeclaratorAST& source );
};

class ParameterDeclarationAST: public AST
{
public:
    typedef std::auto_ptr<ParameterDeclarationAST> Node;
    enum { Type = NodeType_ParameterDeclaration };

public:
    ParameterDeclarationAST();

    TypeSpecifierAST* typeSpec() { return m_typeSpec.get(); }
    void setTypeSpec( TypeSpecifierAST::Node& typeSpec );

    DeclaratorAST* declarator() { return m_declarator.get(); }
    void setDeclarator( DeclaratorAST::Node& declarator );

    AST* expression() { return m_expression.get(); }
    void setExpression( AST::Node& expression );

    virtual QString text() const;

private:
    TypeSpecifierAST::Node m_typeSpec;
    DeclaratorAST::Node m_declarator;
    AST::Node m_expression;

private:
    ParameterDeclarationAST( const ParameterDeclarationAST& source );
    void operator = ( const ParameterDeclarationAST& source );
};

class ParameterDeclarationListAST: public AST
{
public:
    typedef std::auto_ptr<ParameterDeclarationListAST> Node;
    enum { Type = NodeType_ParameterDeclarationList };

public:
    ParameterDeclarationListAST();

    QPtrList<ParameterDeclarationAST> parameterList() { return m_parameterList; }
    void addParameter( ParameterDeclarationAST::Node& parameter );

    virtual QString text() const;

private:
    QPtrList<ParameterDeclarationAST> m_parameterList;

private:
    ParameterDeclarationListAST( const ParameterDeclarationListAST& source );
    void operator = ( const ParameterDeclarationListAST& source );
};

class ParameterDeclarationClauseAST: public AST
{
public:
    typedef std::auto_ptr<ParameterDeclarationClauseAST> Node;
    enum { Type = NodeType_ParameterDeclarationClause };

public:
    ParameterDeclarationClauseAST();

    ParameterDeclarationListAST* parameterDeclarationList() { return m_parameterDeclarationList.get(); }
    void setParameterDeclarationList( ParameterDeclarationListAST::Node& parameterDeclarationList );

    AST* ellipsis() { return m_ellipsis.get(); }
    void setEllipsis( AST::Node& ellipsis );

    virtual QString text() const;

private:
    ParameterDeclarationListAST::Node m_parameterDeclarationList;
    AST::Node m_ellipsis;

private:
    ParameterDeclarationClauseAST( const ParameterDeclarationClauseAST& source );
    void operator = ( const ParameterDeclarationClauseAST& source );
};


class InitDeclaratorAST: public AST
{
public:
    typedef std::auto_ptr<InitDeclaratorAST> Node;
    enum { Type = NodeType_InitDeclarator };

public:
    InitDeclaratorAST();

    DeclaratorAST* declarator() { return m_declarator.get(); }
    void setDeclarator( DeclaratorAST::Node& declarator );

    AST* initializer() { return m_initializer.get(); }
    void setInitializer( AST::Node& initializer );

private:
    DeclaratorAST::Node m_declarator;
    AST::Node m_initializer;

private:
    InitDeclaratorAST( const InitDeclaratorAST& source );
    void operator = ( const InitDeclaratorAST& source );
};

class InitDeclaratorListAST: public AST
{
public:
    typedef std::auto_ptr<InitDeclaratorListAST> Node;
    enum { Type = NodeType_InitDeclaratorList };

public:
    InitDeclaratorListAST();

    QPtrList<InitDeclaratorAST> initDeclaratorList() { return m_initDeclaratorList; }
    void addInitDeclarator( InitDeclaratorAST::Node& decl );

private:
    QPtrList<InitDeclaratorAST> m_initDeclaratorList;

private:
    InitDeclaratorListAST( const InitDeclaratorListAST& source );
    void operator = ( const InitDeclaratorListAST& source );
};

class TypedefAST: public DeclarationAST
{
public:
    typedef std::auto_ptr<TypedefAST> Node;
    enum { Type = NodeType_Typedef };

public:
    TypedefAST();

    TypeSpecifierAST* typeSpec() { return m_typeSpec.get(); }
    void setTypeSpec( TypeSpecifierAST::Node& typeSpec );

    InitDeclaratorListAST* initDeclaratorList() { return m_initDeclaratorList.get(); }
    void setInitDeclaratorList( InitDeclaratorListAST::Node& initDeclaratorList );

private:
    TypeSpecifierAST::Node m_typeSpec;
    InitDeclaratorListAST::Node m_initDeclaratorList;

private:
    TypedefAST( const TypedefAST& source );
    void operator = ( const TypedefAST& source );
};

class TemplateParameterAST: public AST
{
public:
    typedef std::auto_ptr<TemplateParameterAST> Node;
    enum { Type = NodeType_TemplateParameter };

public:
    TemplateParameterAST();

    TypeParameterAST* typeParameter() { return m_typeParameter.get(); }
    void setTypeParameter( TypeParameterAST::Node& typeParameter );

    ParameterDeclarationAST* typeValueParameter() { return m_typeValueParameter.get(); }
    void setTypeValueParameter( ParameterDeclarationAST::Node& typeValueParameter );

private:
    TypeParameterAST::Node m_typeParameter;
    ParameterDeclarationAST::Node m_typeValueParameter;

private:
    TemplateParameterAST( const TemplateParameterAST& source );
    void operator = ( const TemplateParameterAST& source );
};

class TemplateParameterListAST: public AST
{
public:
    typedef std::auto_ptr<TemplateParameterListAST> Node;
    enum { Type = NodeType_TemplateParameterList };

public:
    TemplateParameterListAST();

    QPtrList<TemplateParameterAST> templateParameterList() { return m_templateParameterList; }
    void addTemplateParameter( TemplateParameterAST::Node& templateParameter );

private:
    QPtrList<TemplateParameterAST> m_templateParameterList;

private:
    TemplateParameterListAST( const TemplateParameterListAST& source );
    void operator = ( const TemplateParameterListAST& source );
};

class TemplateDeclarationAST: public DeclarationAST
{
public:
    typedef std::auto_ptr<TemplateDeclarationAST> Node;
    enum { Type = NodeType_TemplateDeclaration };

public:
    TemplateDeclarationAST();

    AST* exported() { return m_exported.get(); }
    void setExported( AST::Node& exported );

    TemplateParameterListAST* templateParameterList() { return m_templateParameterList.get(); }
    void setTemplateParameterList( TemplateParameterListAST::Node& templateParameterList );

    DeclarationAST* declaration() { return m_declaration.get(); }
    void setDeclaration( DeclarationAST::Node& declaration );

private:
    AST::Node m_exported;
    TemplateParameterListAST::Node m_templateParameterList;
    DeclarationAST::Node m_declaration;

private:
    TemplateDeclarationAST( const TemplateDeclarationAST& source );
    void operator = ( const TemplateDeclarationAST& source );
};

class SimpleDeclarationAST: public DeclarationAST
{
public:
    typedef std::auto_ptr<SimpleDeclarationAST> Node;
    enum { Type = NodeType_SimpleDeclaration };

public:
    SimpleDeclarationAST();

    GroupAST* functionSpecifier() { return m_functionSpecifier.get(); }
    void setFunctionSpecifier( GroupAST::Node& functionSpecifier );

    GroupAST* storageSpecifier() { return m_storageSpecifier.get(); }
    void setStorageSpecifier( GroupAST::Node& storageSpecifier );

    TypeSpecifierAST* typeSpec() { return m_typeSpec.get(); }
    void setTypeSpec( TypeSpecifierAST::Node& typeSpec );

    InitDeclaratorListAST* initDeclaratorList() { return m_initDeclaratorList.get(); }
    void setInitDeclaratorList( InitDeclaratorListAST::Node& initDeclaratorList );

private:
    GroupAST::Node m_functionSpecifier;
    GroupAST::Node m_storageSpecifier;
    TypeSpecifierAST::Node m_typeSpec;
    InitDeclaratorListAST::Node m_initDeclaratorList;

private:
    SimpleDeclarationAST( const SimpleDeclarationAST& source );
    void operator = ( const SimpleDeclarationAST& source );
};

class StatementAST: public AST
{
public:
    typedef std::auto_ptr<StatementAST> Node;
    enum { Type = NodeType_Statement };

public:
    StatementAST();

private:
    StatementAST( const StatementAST& source );
    void operator = ( const StatementAST& source );
};

class ExpressionStatementAST: public StatementAST
{
public:
    typedef std::auto_ptr<ExpressionStatementAST> Node;
    enum { Type = NodeType_ExpressionStatement };

public:
    ExpressionStatementAST();

    AST* expression() { return m_expression.get(); }
    void setExpression( AST::Node& expression );

private:
    AST::Node m_expression;

private:
    ExpressionStatementAST( const ExpressionStatementAST& source );
    void operator = ( const ExpressionStatementAST& source );
};

class ConditionAST: public AST
{
public:
    typedef std::auto_ptr<ConditionAST> Node;
    enum { Type = NodeType_Condition };

public:
    ConditionAST();

    TypeSpecifierAST* typeSpec() { return m_typeSpec.get(); }
    void setTypeSpec( TypeSpecifierAST::Node& typeSpec );

    DeclaratorAST* declarator() { return m_declarator.get(); }
    void setDeclarator( DeclaratorAST::Node& declarator );

    AST* expression() { return m_expression.get(); }
    void setExpression( AST::Node& expression );

private:
    TypeSpecifierAST::Node m_typeSpec;
    DeclaratorAST::Node m_declarator;
    AST::Node m_expression;

private:
    ConditionAST( const ConditionAST& source );
    void operator = ( const ConditionAST& source );
};

class IfStatementAST: public StatementAST
{
public:
    typedef std::auto_ptr<IfStatementAST> Node;
    enum { Type = NodeType_IfStatement };

public:
    IfStatementAST();

    ConditionAST* condition() const { return m_condition.get(); }
    void setCondition( ConditionAST::Node& condition );

    StatementAST* statement() { return m_statement.get(); }
    void setStatement( StatementAST::Node& statement );

    StatementAST* elseStatement() { return m_elseStatement.get(); }
    void setElseStatement( StatementAST::Node& statement );

private:
    ConditionAST::Node m_condition;
    StatementAST::Node m_statement;
    StatementAST::Node m_elseStatement;

private:
    IfStatementAST( const IfStatementAST& source );
    void operator = ( const IfStatementAST& source );
};

class WhileStatementAST: public StatementAST
{
public:
    typedef std::auto_ptr<WhileStatementAST> Node;
    enum { Type = NodeType_WhileStatement };

public:
    WhileStatementAST();

    ConditionAST* condition() const { return m_condition.get(); }
    void setCondition( ConditionAST::Node& condition );

    StatementAST* statement() { return m_statement.get(); }
    void setStatement( StatementAST::Node& statement );

private:
    ConditionAST::Node m_condition;
    StatementAST::Node m_statement;

private:
    WhileStatementAST( const WhileStatementAST& source );
    void operator = ( const WhileStatementAST& source );
};

class DoStatementAST: public StatementAST
{
public:
    typedef std::auto_ptr<DoStatementAST> Node;
    enum { Type = NodeType_DoStatement };

public:
    DoStatementAST();

    ConditionAST* condition() const { return m_condition.get(); }
    void setCondition( ConditionAST::Node& condition );

    StatementAST* statement() { return m_statement.get(); }
    void setStatement( StatementAST::Node& statement );

private:
    ConditionAST::Node m_condition;
    StatementAST::Node m_statement;

private:
    DoStatementAST( const DoStatementAST& source );
    void operator = ( const DoStatementAST& source );
};

class ForStatementAST: public StatementAST
{
public:
    typedef std::auto_ptr<ForStatementAST> Node;
    enum { Type = NodeType_ForStatement };

public:
    ForStatementAST();

    StatementAST* initStatement() { return m_initStatement.get(); }
    void setInitStatement( StatementAST::Node& statement );

    ConditionAST* condition() const { return m_condition.get(); }
    void setCondition( ConditionAST::Node& condition );

    AST* expression() const { return m_expression.get(); }
    void setExpression( AST::Node& expression );

    StatementAST* statement() { return m_statement.get(); }
    void setStatement( StatementAST::Node& statement );

private:
    ConditionAST::Node m_condition;
    StatementAST::Node m_initStatement;
    StatementAST::Node m_statement;
    AST::Node m_expression;

private:
    ForStatementAST( const ForStatementAST& source );
    void operator = ( const ForStatementAST& source );
};

class SwitchStatementAST: public StatementAST
{
public:
    typedef std::auto_ptr<SwitchStatementAST> Node;
    enum { Type = NodeType_SwitchStatement };

public:
    SwitchStatementAST();

    ConditionAST* condition() const { return m_condition.get(); }
    void setCondition( ConditionAST::Node& condition );

    StatementAST* statement() { return m_statement.get(); }
    void setStatement( StatementAST::Node& statement );

private:
    ConditionAST::Node m_condition;
    StatementAST::Node m_statement;

private:
    SwitchStatementAST( const SwitchStatementAST& source );
    void operator = ( const SwitchStatementAST& source );
};

class StatementListAST: public StatementAST
{
public:
    typedef std::auto_ptr<StatementListAST> Node;
    enum { Type = NodeType_StatementList };

public:
    StatementListAST();

    QPtrList<StatementAST> statementList() { return m_statementList; }
    void addStatement( StatementAST::Node& statement );

private:
    QPtrList<StatementAST> m_statementList;

private:
    StatementListAST( const StatementListAST& source );
    void operator = ( const StatementListAST& source );
};

class DeclarationStatementAST: public StatementAST
{
public:
    typedef std::auto_ptr<DeclarationStatementAST> Node;
    enum { Type = NodeType_DeclarationStatement };

public:
    DeclarationStatementAST();

    DeclarationAST* declaration() { return m_declaration.get(); }
    void setDeclaration( DeclarationAST::Node& declaration );

private:
    DeclarationAST::Node m_declaration;

private:
    DeclarationStatementAST( const DeclarationStatementAST& source );
    void operator = ( const DeclarationStatementAST& source );
};

class FunctionDefinitionAST: public DeclarationAST
{
public:
    typedef std::auto_ptr<FunctionDefinitionAST> Node;
    enum { Type = NodeType_FunctionDefinition };

public:
    FunctionDefinitionAST();

    GroupAST* functionSpecifier() { return m_functionSpecifier.get(); }
    void setFunctionSpecifier( GroupAST::Node& functionSpecifier );

    GroupAST* storageSpecifier() { return m_storageSpecifier.get(); }
    void setStorageSpecifier( GroupAST::Node& storageSpecifier );

    TypeSpecifierAST* typeSpec() { return m_typeSpec.get(); }
    void setTypeSpec( TypeSpecifierAST::Node& typeSpec );

    InitDeclaratorAST* initDeclarator() { return m_initDeclarator.get(); }
    void setInitDeclarator( InitDeclaratorAST::Node& initDeclarator );

    StatementListAST* functionBody() { return m_functionBody.get(); }
    void setFunctionBody( StatementListAST::Node& functionBody );

private:
    GroupAST::Node m_functionSpecifier;
    GroupAST::Node m_storageSpecifier;
    TypeSpecifierAST::Node m_typeSpec;
    InitDeclaratorAST::Node m_initDeclarator;
    StatementListAST::Node m_functionBody;

private:
    FunctionDefinitionAST( const FunctionDefinitionAST& source );
    void operator = ( const FunctionDefinitionAST& source );
};


class TranslationUnitAST: public AST
{
public:
    typedef std::auto_ptr<TranslationUnitAST> Node;
    enum { Type = NodeType_TranslationUnit };

public:
    TranslationUnitAST();

    void addDeclaration( DeclarationAST::Node& ast );
    QPtrList<DeclarationAST> declarationList() { return m_declarationList; }

private:
    QPtrList<DeclarationAST> m_declarationList;

private:
    TranslationUnitAST( const TranslationUnitAST& source );
    void operator = ( const TranslationUnitAST& source );
};

#endif
