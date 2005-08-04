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
    the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef __ast_h
#define __ast_h

#include <memory>
#include <qstring.h>
#include <q3ptrlist.h>

#if defined( Q_OS_WIN32 ) || defined( Q_CC_SUN )

#ifndef _THROW0
#  define _THROW0()
#endif

template <class _Tp> class AUTO_PTR {
private:
    _Tp* _M_ptr;

public:
    typedef _Tp element_type;

    explicit AUTO_PTR(_Tp* __p = 0)  _THROW0() : _M_ptr(__p) {}

    template <class _Tp1> AUTO_PTR(AUTO_PTR<_Tp1>& __a)  _THROW0()
	: _M_ptr(__a.release()) {}

    AUTO_PTR(AUTO_PTR& __a)  _THROW0() : _M_ptr(__a.release()) {}



    template <class _Tp1>
    AUTO_PTR& operator=(AUTO_PTR<_Tp1>& __a)  _THROW0() {
	if (__a.get() != this->get()) {
	    delete _M_ptr;
	    _M_ptr = __a.release();
	}
	return *this;
    }

    AUTO_PTR& operator=(AUTO_PTR& __a)  _THROW0() {
	if (&__a != this) {
	    delete _M_ptr;
	    _M_ptr = __a.release();
	}
	return *this;
    }

    ~AUTO_PTR()  _THROW0() { delete _M_ptr; }

    _Tp& operator*() const  _THROW0() {
	return *_M_ptr;
    }
    _Tp* operator->() const  _THROW0() {
	return _M_ptr;
    }
    _Tp* get() const  _THROW0() {
	return _M_ptr;
    }
    _Tp* release()  _THROW0() {
	_Tp* __tmp = _M_ptr;
	_M_ptr = 0;
	return __tmp;
    }
    void reset(_Tp* __p = 0)  _THROW0() {
	delete _M_ptr;
	_M_ptr = __p;
    }

    // According to the C++ standard, these conversions are required.  Most
    // present-day compilers, however, do not enforce that requirement---and,
    // in fact, most present-day compilers do not support the language
    // features that these conversions rely on.


private:
    template<class _Tp1> struct AUTO_PTR_ref {
	_Tp1* _M_ptr;
	AUTO_PTR_ref(_Tp1* __p) : _M_ptr(__p) {}
    };

public:
    AUTO_PTR(AUTO_PTR_ref<_Tp> __ref)  _THROW0()
	: _M_ptr(__ref._M_ptr) {}
    template <class _Tp1> operator AUTO_PTR_ref<_Tp1>()  _THROW0()
	{ return AUTO_PTR_ref<_Tp>(this->release()); }
    template <class _Tp1> operator AUTO_PTR<_Tp1>()  _THROW0()
	{ return AUTO_PTR<_Tp1>(this->release()) }

};

#else
#define AUTO_PTR std::auto_ptr
#endif

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
    NodeType_ForEachStatement, // qt4 [erbsland]
    NodeType_SwitchStatement,
	NodeType_CatchStatement,
    NodeType_CatchStatementList,
	NodeType_TryBlockStatement,
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

QString nodeTypeToString( int type );


#if defined(CPPPARSER_QUICK_ALLOCATOR)

#include <quick_allocator.h>

#define DECLARE_ALLOC(tp) \
    void * operator new(std::size_t) \
    { \
        return quick_allocator< tp >::alloc(); \
    } \
 \
    void operator delete(void * p) \
    { \
        quick_allocator< tp >::dealloc(p); \
    }
#else

#define DECLARE_ALLOC(tp)

#endif

struct Slice
{
    QString source;
    int position;
    int length;
    
    inline Slice()
        : position(0), length(0) {}
};

class AST
{
public:
    typedef AUTO_PTR<AST> Node;
    enum { Type=NodeType_Generic };
    
    DECLARE_ALLOC( AST )

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

#ifndef CPPPARSER_NO_CHILDREN
    Q3PtrList<AST> children() { return m_children; }
    void appendChild( AST* child );
    void removeChild( AST* child );
#endif

    virtual inline QString text() const 
    { return m_slice.source.mid(m_slice.position, m_slice.length); }
    
    inline void setSlice( const Slice& slice ) 
    { m_slice = slice; }

    inline void setSlice( const QString &text, int position, int length ) 
    {
        m_slice.source = text;
        m_slice.position = position;
        m_slice.length = length;
    }
    
    inline void setText(const QString &text)
    { setSlice(text, 0, text.length()); }

private:
    int m_nodeType;
    AST* m_parent;
    int m_startLine, m_startColumn;
    int m_endLine, m_endColumn;
    Slice m_slice;
#ifndef CPPPARSER_NO_CHILDREN
    Q3PtrList<AST> m_children;
#endif

private:
    AST( const AST& source );
    void operator = ( const AST& source );
};

class GroupAST: public AST
{
public:
    typedef AUTO_PTR<GroupAST> Node;
    enum { Type = NodeType_Group };

    DECLARE_ALLOC( GroupAST )
    
public:
    GroupAST();

    Q3PtrList<AST> nodeList() { return m_nodeList; }
    void addNode( AST::Node& node );

    virtual QString text() const;

private:
    Q3PtrList<AST> m_nodeList;

private:
    GroupAST( const GroupAST& source );
    void operator = ( const GroupAST& source );
};


class TemplateArgumentListAST: public AST
{
public:
    typedef AUTO_PTR<TemplateArgumentListAST> Node;
    enum { Type = NodeType_TemplateArgumentList };

    DECLARE_ALLOC( TemplateArgumentListAST )
    
public:
    TemplateArgumentListAST();

    void addArgument( AST::Node& arg );
    Q3PtrList<AST> argumentList() { return m_argumentList; }

    virtual QString text() const;

private:
    Q3PtrList<AST> m_argumentList;

private:
    TemplateArgumentListAST( const TemplateArgumentListAST& source );
    void operator = ( const TemplateArgumentListAST& source );
};

class ClassOrNamespaceNameAST: public AST
{
public:
    typedef AUTO_PTR<ClassOrNamespaceNameAST> Node;
    enum { Type = NodeType_ClassOrNamespaceName };

    DECLARE_ALLOC( ClassOrNamespaceNameAST )
    
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
    typedef AUTO_PTR<NameAST> Node;
    enum { Type = NodeType_Name };

    DECLARE_ALLOC( NameAST )
    
public:
    NameAST();

    bool isGlobal() const { return m_global; }
    void setGlobal( bool b );

    void addClassOrNamespaceName( ClassOrNamespaceNameAST::Node& classOrNamespaceName );
    Q3PtrList<ClassOrNamespaceNameAST> classOrNamespaceNameList() { return m_classOrNamespaceNameList; }

    ClassOrNamespaceNameAST* unqualifiedName() { return m_unqualifiedName.get(); }
    void setUnqualifiedName( ClassOrNamespaceNameAST::Node& unqualifiedName );

    virtual QString text() const;

private:
    bool m_global;
    ClassOrNamespaceNameAST::Node m_unqualifiedName;
    Q3PtrList<ClassOrNamespaceNameAST> m_classOrNamespaceNameList;

private:
    NameAST( const NameAST& source );
    void operator = ( const NameAST& source );
};

class TypeParameterAST: public AST
{
public:
    typedef AUTO_PTR<TypeParameterAST> Node;
    enum { Type = NodeType_TypeParameter };

    DECLARE_ALLOC( TypeParameterAST )
    
public:
    TypeParameterAST();

    AST* kind() { return m_kind.get(); }
    void setKind( AST::Node& kind );

    class TemplateParameterListAST* templateParameterList() { return m_templateParameterList.get(); }
    void setTemplateParameterList( AUTO_PTR<class TemplateParameterListAST>& templateParameterList );

    NameAST* name() { return m_name.get(); }
    void setName( NameAST::Node& name );

    AST* typeId() { return m_typeId.get(); }
    void setTypeId( AST::Node& typeId );

private:
    AST::Node m_kind;
    AUTO_PTR<class TemplateParameterListAST> m_templateParameterList;
    NameAST::Node m_name;
    AST::Node m_typeId;

private:
    TypeParameterAST( const TypeParameterAST& source );
    void operator = ( const TypeParameterAST& source );
};

class DeclarationAST: public AST
{
public:
    typedef AUTO_PTR<DeclarationAST> Node;
    enum { Type = NodeType_Declaration };

    DECLARE_ALLOC( DeclarationAST )
    
public:
    DeclarationAST();

private:
    DeclarationAST( const DeclarationAST& source );
    void operator = ( const DeclarationAST& source );
};

class AccessDeclarationAST: public DeclarationAST
{
public:
    typedef AUTO_PTR<AccessDeclarationAST> Node;
    enum { Type = NodeType_AccessDeclaration };

    DECLARE_ALLOC( AccessDeclarationAST )
    
public:
    AccessDeclarationAST();

    Q3PtrList<AST> accessList() { return m_accessList; }
    void addAccess( AST::Node& access );

    virtual QString text() const;

private:
    Q3PtrList<AST> m_accessList;

private:
    AccessDeclarationAST( const AccessDeclarationAST& source );
    void operator = ( const AccessDeclarationAST& source );
};

class TypeSpecifierAST: public AST
{
public:
    typedef AUTO_PTR<TypeSpecifierAST> Node;
    enum { Type = NodeType_TypeSpecifier };

    DECLARE_ALLOC( TypeSpecifierAST )
    
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
    typedef AUTO_PTR<BaseSpecifierAST> Node;
    enum { Type = NodeType_BaseSpecifier };

    DECLARE_ALLOC( BaseSpecifierAST )
    
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
    typedef AUTO_PTR<BaseClauseAST> Node;
    enum { Type = NodeType_BaseClause };

    DECLARE_ALLOC( BaseClauseAST )
    
public:
    BaseClauseAST();

    void addBaseSpecifier( BaseSpecifierAST::Node& baseSpecifier );
    Q3PtrList<BaseSpecifierAST> baseSpecifierList() { return m_baseSpecifierList; }

private:
    Q3PtrList<BaseSpecifierAST> m_baseSpecifierList;

private:
    BaseClauseAST( const BaseClauseAST& source );
    void operator = ( const BaseClauseAST& source );
};

class ClassSpecifierAST: public TypeSpecifierAST
{
public:
    typedef AUTO_PTR<ClassSpecifierAST> Node;
    enum { Type = NodeType_ClassSpecifier };

    DECLARE_ALLOC( ClassSpecifierAST )
    
public:
    ClassSpecifierAST();

    GroupAST* winDeclSpec() { return m_winDeclSpec.get(); }
    void setWinDeclSpec( GroupAST::Node& winDeclSpec );

    AST* classKey() { return m_classKey.get(); }
    void setClassKey( AST::Node& classKey );

    BaseClauseAST* baseClause() { return m_baseClause.get(); }
    void setBaseClause( BaseClauseAST::Node& baseClause );

    Q3PtrList<DeclarationAST> declarationList() { return m_declarationList; }
    void addDeclaration( DeclarationAST::Node& declaration );

private:
    GroupAST::Node m_winDeclSpec;
    AST::Node m_classKey;
    BaseClauseAST::Node m_baseClause;
    Q3PtrList<DeclarationAST> m_declarationList;

private:
    ClassSpecifierAST( const ClassSpecifierAST& source );
    void operator = ( const ClassSpecifierAST& source );
};

class EnumeratorAST: public AST
{
public:
    typedef AUTO_PTR<EnumeratorAST> Node;
    enum { Type = NodeType_Enumerator };

    DECLARE_ALLOC( EnumeratorAST )
    
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
    typedef AUTO_PTR<EnumSpecifierAST> Node;
    enum { Type = NodeType_EnumSpecifier };

    DECLARE_ALLOC( EnumSpecifierAST )
    
public:
    EnumSpecifierAST();

    void addEnumerator( EnumeratorAST::Node& enumerator );
    Q3PtrList<EnumeratorAST> enumeratorList() { return m_enumeratorList; }

private:
    Q3PtrList<EnumeratorAST> m_enumeratorList;

private:
    EnumSpecifierAST( const EnumSpecifierAST& source );
    void operator = ( const EnumSpecifierAST& source );
};

class ElaboratedTypeSpecifierAST: public TypeSpecifierAST
{
public:
    typedef AUTO_PTR<ElaboratedTypeSpecifierAST> Node;
    enum { Type = NodeType_ElaboratedTypeSpecifier };

    DECLARE_ALLOC( ElaboratedTypeSpecifierAST )
    
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
    typedef AUTO_PTR<LinkageBodyAST> Node;
    enum { Type = NodeType_LinkageBody };

    DECLARE_ALLOC( LinkageBodyAST )
    
public:
    LinkageBodyAST();

    void addDeclaration( DeclarationAST::Node& ast );
    Q3PtrList<DeclarationAST> declarationList() { return m_declarationList; }

private:
    Q3PtrList<DeclarationAST> m_declarationList;

private:
    LinkageBodyAST( const LinkageBodyAST& source );
    void operator = ( const LinkageBodyAST& source );
};

class LinkageSpecificationAST: public DeclarationAST
{
public:
    typedef AUTO_PTR<LinkageSpecificationAST> Node;
    enum { Type = NodeType_LinkageSpecification };

    DECLARE_ALLOC( LinkageSpecificationAST )
    
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
    typedef AUTO_PTR<NamespaceAST> Node;
    enum { Type = NodeType_Namespace };

    DECLARE_ALLOC( NamespaceAST )
    
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
    typedef AUTO_PTR<NamespaceAliasAST> Node;
    enum { Type = NodeType_NamespaceAlias };

    DECLARE_ALLOC( NamespaceAliasAST )
    
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
    typedef AUTO_PTR<UsingAST> Node;
    enum { Type = NodeType_Using };

    DECLARE_ALLOC( UsingAST )
    
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
    typedef AUTO_PTR<UsingDirectiveAST> Node;
    enum { Type = NodeType_UsingDirective };

    DECLARE_ALLOC( UsingDirectiveAST )
    
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
    typedef AUTO_PTR<DeclaratorAST> Node;
    enum { Type = NodeType_Declarator };

    DECLARE_ALLOC( DeclaratorAST )
    
public:
    DeclaratorAST();

    Q3PtrList<AST> ptrOpList() { return m_ptrOpList; }
    void addPtrOp( AST::Node& ptrOp );

    DeclaratorAST* subDeclarator() { return m_subDeclarator.get(); }
    void setSubDeclarator( AUTO_PTR<DeclaratorAST>& subDeclarator );

    NameAST* declaratorId() { return m_declaratorId.get(); }
    void setDeclaratorId( NameAST::Node& declaratorId );

    AST* bitfieldInitialization() { return m_bitfieldInitialization.get(); }
    void setBitfieldInitialization( AST::Node& bitfieldInitialization );

    Q3PtrList<AST> arrayDimensionList() { return m_arrayDimensionList; }
    void addArrayDimension( AST::Node& arrayDimension );

    class ParameterDeclarationClauseAST* parameterDeclarationClause() { return m_parameterDeclarationClause.get(); }
    void setParameterDeclarationClause( AUTO_PTR<class ParameterDeclarationClauseAST>& parameterDeclarationClause );

    // ### replace 'constant' with cvQualify
    AST* constant() { return m_constant.get(); }
    void setConstant( AST::Node& constant );

    GroupAST* exceptionSpecification() { return m_exceptionSpecification.get(); }
    void setExceptionSpecification( GroupAST::Node& exceptionSpecification );

private:
    Q3PtrList<AST> m_ptrOpList;
    AUTO_PTR<DeclaratorAST> m_subDeclarator;
    NameAST::Node m_declaratorId;
    AST::Node m_bitfieldInitialization;
    Q3PtrList<AST> m_arrayDimensionList;
    AUTO_PTR<class ParameterDeclarationClauseAST> m_parameterDeclarationClause;
    AST::Node m_constant;
    GroupAST::Node m_exceptionSpecification;

private:
    DeclaratorAST( const DeclaratorAST& source );
    void operator = ( const DeclaratorAST& source );
};

class ParameterDeclarationAST: public AST
{
public:
    typedef AUTO_PTR<ParameterDeclarationAST> Node;
    enum { Type = NodeType_ParameterDeclaration };

    DECLARE_ALLOC( ParameterDeclarationAST )
    
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
    typedef AUTO_PTR<ParameterDeclarationListAST> Node;
    enum { Type = NodeType_ParameterDeclarationList };

    DECLARE_ALLOC( ParameterDeclarationListAST )
    
public:
    ParameterDeclarationListAST();

    Q3PtrList<ParameterDeclarationAST> parameterList() { return m_parameterList; }
    void addParameter( ParameterDeclarationAST::Node& parameter );

    virtual QString text() const;

private:
    Q3PtrList<ParameterDeclarationAST> m_parameterList;

private:
    ParameterDeclarationListAST( const ParameterDeclarationListAST& source );
    void operator = ( const ParameterDeclarationListAST& source );
};

class ParameterDeclarationClauseAST: public AST
{
public:
    typedef AUTO_PTR<ParameterDeclarationClauseAST> Node;
    enum { Type = NodeType_ParameterDeclarationClause };

    DECLARE_ALLOC( ParameterDeclarationClauseAST )
    
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
    typedef AUTO_PTR<InitDeclaratorAST> Node;
    enum { Type = NodeType_InitDeclarator };

    DECLARE_ALLOC( InitDeclaratorAST )
    
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
    typedef AUTO_PTR<InitDeclaratorListAST> Node;
    enum { Type = NodeType_InitDeclaratorList };

    DECLARE_ALLOC( InitDeclaratorListAST )
    
public:
    InitDeclaratorListAST();

    Q3PtrList<InitDeclaratorAST> initDeclaratorList() { return m_initDeclaratorList; }
    void addInitDeclarator( InitDeclaratorAST::Node& decl );

private:
    Q3PtrList<InitDeclaratorAST> m_initDeclaratorList;

private:
    InitDeclaratorListAST( const InitDeclaratorListAST& source );
    void operator = ( const InitDeclaratorListAST& source );
};

class TypedefAST: public DeclarationAST
{
public:
    typedef AUTO_PTR<TypedefAST> Node;
    enum { Type = NodeType_Typedef };

    DECLARE_ALLOC( TypedefAST )
    
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
    typedef AUTO_PTR<TemplateParameterAST> Node;
    enum { Type = NodeType_TemplateParameter };

    DECLARE_ALLOC( TemplateParameterAST )
    
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
    typedef AUTO_PTR<TemplateParameterListAST> Node;
    enum { Type = NodeType_TemplateParameterList };

    DECLARE_ALLOC( TemplateParameterListAST )
    
public:
    TemplateParameterListAST();

    Q3PtrList<TemplateParameterAST> templateParameterList() { return m_templateParameterList; }
    void addTemplateParameter( TemplateParameterAST::Node& templateParameter );

private:
    Q3PtrList<TemplateParameterAST> m_templateParameterList;

private:
    TemplateParameterListAST( const TemplateParameterListAST& source );
    void operator = ( const TemplateParameterListAST& source );
};

class TemplateDeclarationAST: public DeclarationAST
{
public:
    typedef AUTO_PTR<TemplateDeclarationAST> Node;
    enum { Type = NodeType_TemplateDeclaration };

    DECLARE_ALLOC( TemplateDeclarationAST )
    
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
    typedef AUTO_PTR<SimpleDeclarationAST> Node;
    enum { Type = NodeType_SimpleDeclaration };

    DECLARE_ALLOC( SimpleDeclarationAST )
    
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

    GroupAST* winDeclSpec() { return m_winDeclSpec.get(); }
    void setWinDeclSpec( GroupAST::Node& winDeclSpec );

private:
    GroupAST::Node m_functionSpecifier;
    GroupAST::Node m_storageSpecifier;
    TypeSpecifierAST::Node m_typeSpec;
    InitDeclaratorListAST::Node m_initDeclaratorList;
    GroupAST::Node m_winDeclSpec;

private:
    SimpleDeclarationAST( const SimpleDeclarationAST& source );
    void operator = ( const SimpleDeclarationAST& source );
};

class StatementAST: public AST
{
public:
    typedef AUTO_PTR<StatementAST> Node;
    enum { Type = NodeType_Statement };

    DECLARE_ALLOC( StatementAST )
    
public:
    StatementAST();

private:
    StatementAST( const StatementAST& source );
    void operator = ( const StatementAST& source );
};

class ExpressionStatementAST: public StatementAST
{
public:
    typedef AUTO_PTR<ExpressionStatementAST> Node;
    enum { Type = NodeType_ExpressionStatement };

    DECLARE_ALLOC( ExpressionStatementAST )
    
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
    typedef AUTO_PTR<ConditionAST> Node;
    enum { Type = NodeType_Condition };

    DECLARE_ALLOC( ConditionAST )
    
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
    typedef AUTO_PTR<IfStatementAST> Node;
    enum { Type = NodeType_IfStatement };

    DECLARE_ALLOC( IfStatementAST )
    
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
    typedef AUTO_PTR<WhileStatementAST> Node;
    enum { Type = NodeType_WhileStatement };

    DECLARE_ALLOC( WhileStatementAST )
    
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
    typedef AUTO_PTR<DoStatementAST> Node;
    enum { Type = NodeType_DoStatement };

    DECLARE_ALLOC( DoStatementAST )
    
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
    typedef AUTO_PTR<ForStatementAST> Node;
    enum { Type = NodeType_ForStatement };

    DECLARE_ALLOC( ForStatementAST )
    
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

// qt4 [erbsland]
class ForEachStatementAST: public StatementAST
{
public:
    typedef AUTO_PTR<ForEachStatementAST> Node;
    enum { Type = NodeType_ForEachStatement };

    DECLARE_ALLOC( ForEachStatementAST )
    
public:
    ForEachStatementAST();

    StatementAST* initStatement() { return m_initStatement.get(); }
    void setInitStatement( StatementAST::Node& statement );

    StatementAST* statement() { return m_statement.get(); }
    void setStatement( StatementAST::Node& statement );
    
    AST* expression() const { return m_expression.get(); }
    void setExpression( AST::Node& expression );

private:
    StatementAST::Node m_initStatement;
    StatementAST::Node m_statement;
    AST::Node m_expression;

private:
    ForEachStatementAST( const ForEachStatementAST& source );
    void operator = ( const ForEachStatementAST& source );
};

class SwitchStatementAST: public StatementAST
{
public:
    typedef AUTO_PTR<SwitchStatementAST> Node;
    enum { Type = NodeType_SwitchStatement };

    DECLARE_ALLOC( SwitchStatementAST )
    
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
    typedef AUTO_PTR<StatementListAST> Node;
    enum { Type = NodeType_StatementList };

    DECLARE_ALLOC( StatementListAST )
    
public:
    StatementListAST();

    Q3PtrList<StatementAST> statementList() { return m_statementList; }
    void addStatement( StatementAST::Node& statement );

private:
    Q3PtrList<StatementAST> m_statementList;

private:
    StatementListAST( const StatementListAST& source );
    void operator = ( const StatementListAST& source );
};

class CatchStatementAST: public StatementAST
{
public:
    typedef AUTO_PTR<CatchStatementAST> Node;
    enum { Type = NodeType_CatchStatement };

    DECLARE_ALLOC( CatchStatementAST )
    
public:
    CatchStatementAST();

    ConditionAST* condition() const { return m_condition.get(); }
    void setCondition( ConditionAST::Node& condition );

    StatementAST* statement() { return m_statement.get(); }
    void setStatement( StatementAST::Node& statement );

private:
    ConditionAST::Node m_condition;
    StatementAST::Node m_statement;

private:
    CatchStatementAST( const CatchStatementAST& source );
    void operator = ( const CatchStatementAST& source );
};

class CatchStatementListAST: public StatementAST
{
public:
    typedef AUTO_PTR<CatchStatementListAST> Node;
    enum { Type = NodeType_CatchStatementList };

    DECLARE_ALLOC( CatchStatementListAST )
    
public:
    CatchStatementListAST();

    Q3PtrList<CatchStatementAST> statementList() { return m_statementList; }
    void addStatement( CatchStatementAST::Node& statement );

private:
    Q3PtrList<CatchStatementAST> m_statementList;

private:
    CatchStatementListAST( const CatchStatementListAST& source );
    void operator = ( const CatchStatementListAST& source );
};

class TryBlockStatementAST: public StatementAST
{
public:
    typedef AUTO_PTR<TryBlockStatementAST> Node;
    enum { Type = NodeType_TryBlockStatement };

    DECLARE_ALLOC( TryBlockStatementAST )
    
public:
    TryBlockStatementAST();

    StatementAST* statement() { return m_statement.get(); }
    void setStatement( StatementAST::Node& statement );

    CatchStatementListAST* catchStatementList() { return m_catchStatementList.get(); }
    void setCatchStatementList( CatchStatementListAST::Node& statementList );

private:
    StatementAST::Node m_statement;
    CatchStatementListAST::Node m_catchStatementList;

private:
    TryBlockStatementAST( const TryBlockStatementAST& source );
    void operator = ( const TryBlockStatementAST& source );
};

class DeclarationStatementAST: public StatementAST
{
public:
    typedef AUTO_PTR<DeclarationStatementAST> Node;
    enum { Type = NodeType_DeclarationStatement };

    DECLARE_ALLOC( DeclarationStatementAST )
    
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
    typedef AUTO_PTR<FunctionDefinitionAST> Node;
    enum { Type = NodeType_FunctionDefinition };

    DECLARE_ALLOC( FunctionDefinitionAST )
    
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

    GroupAST* winDeclSpec() { return m_winDeclSpec.get(); }
    void setWinDeclSpec( GroupAST::Node& winDeclSpec );

private:
    GroupAST::Node m_functionSpecifier;
    GroupAST::Node m_storageSpecifier;
    TypeSpecifierAST::Node m_typeSpec;
    InitDeclaratorAST::Node m_initDeclarator;
    StatementListAST::Node m_functionBody;
    GroupAST::Node m_winDeclSpec;

private:
    FunctionDefinitionAST( const FunctionDefinitionAST& source );
    void operator = ( const FunctionDefinitionAST& source );
};


class TranslationUnitAST: public AST
{
public:
    typedef AUTO_PTR<TranslationUnitAST> Node;
    enum { Type = NodeType_TranslationUnit };

    DECLARE_ALLOC( TranslationUnitAST )
    
public:
    TranslationUnitAST();

    void addDeclaration( DeclarationAST::Node& ast );
    Q3PtrList<DeclarationAST> declarationList() { return m_declarationList; }

private:
    Q3PtrList<DeclarationAST> m_declarationList;

private:
    TranslationUnitAST( const TranslationUnitAST& source );
    void operator = ( const TranslationUnitAST& source );
};

#endif
