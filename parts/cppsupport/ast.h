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
    NodeType_NestedNameSpecifier,
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
    NodeType_TranslationUnit,

    NodeType_Custom = 2000
};

class AST
{
public:
    typedef std::auto_ptr<AST> Node;
    enum { Type=NodeType_Generic };

public:
    AST();
    virtual ~AST();

    virtual int nodeType() const { return m_nodeType; }
    virtual void setNodeType( int nodeType ) { m_nodeType = nodeType; }

    virtual AST* parent() { return m_parent; }
    virtual void setParent( AST* parent );

    virtual void setStartPosition( int line, int col );
    virtual void getStartPosition( int* line, int* col ) const;

    virtual void setEndPosition( int line, int col );
    virtual void getEndPosition( int* line, int* col ) const;

    virtual QString text() const { return m_text; }
    virtual void setText( const QString& text ) { m_text = text; }

    virtual QPtrList<AST> children() { return m_children; }
    virtual void appendChild( AST* child );
    virtual void removeChild( AST* child );

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

class TemplateArgumentListAST: public AST
{
public:
    typedef std::auto_ptr<TemplateArgumentListAST> Node;
    enum { Type = NodeType_TemplateArgumentList };

public:
    TemplateArgumentListAST();
    virtual ~TemplateArgumentListAST();

    void addArgument( AST::Node& arg );
    QPtrList<AST> arguments() { return m_arguments; }

private:
    QPtrList<AST> m_arguments;

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
    virtual ~ClassOrNamespaceNameAST();
    
    AST* name();
    void setName( AST::Node& name );
    
    TemplateArgumentListAST* templateArgumentList();
    void setTemplateArgumentList( TemplateArgumentListAST::Node& templateArgumentList );
                
private:
    AST::Node m_name;
    TemplateArgumentListAST::Node m_templateArgumentList;
    
private:
    ClassOrNamespaceNameAST( const ClassOrNamespaceNameAST& source );
    void operator = ( const ClassOrNamespaceNameAST& source );
};

class NestedNameSpecifierAST: public AST
{
public:
    typedef std::auto_ptr<NestedNameSpecifierAST> Node;
    enum { Type = NodeType_NestedNameSpecifier };

public:
    NestedNameSpecifierAST();
    virtual ~NestedNameSpecifierAST();
    
    void addClassOrNamespaceName( ClassOrNamespaceNameAST::Node& classOrNamespaceName );
    QPtrList<ClassOrNamespaceNameAST> classOrNamespaceNameList() { return m_classOrNamespaceNameList; }
                    
private:
    QPtrList<ClassOrNamespaceNameAST> m_classOrNamespaceNameList;
    
private:
    NestedNameSpecifierAST( const NestedNameSpecifierAST& source );
    void operator = ( const NestedNameSpecifierAST& source );
};

class NameAST: public AST
{
public:
    typedef std::auto_ptr<NameAST> Node;
    enum { Type = NodeType_Name };

public:
    NameAST();
    virtual ~NameAST();
        
    bool isGlobal() const;
    void setGlobal( bool b );
    
    void setNestedName( NestedNameSpecifierAST::Node& nestedName );
    NestedNameSpecifierAST* nestedName();
    
    void setUnqualifedName( AST::Node& unqualifiedName );
    AST* unqualifiedName();
    
private:
    bool m_global;
    NestedNameSpecifierAST::Node m_nestedName;
    AST::Node m_unqualifiedName;
    
private:
    NameAST( const NameAST& source );
    void operator = ( const NameAST& source );
};

class DeclarationAST: public AST
{
public:
    typedef std::auto_ptr<DeclarationAST> Node;
    enum { Type = NodeType_Declaration };

public:
    DeclarationAST();
    virtual ~DeclarationAST();
    
private:
    DeclarationAST( const DeclarationAST& source );
    void operator = ( const DeclarationAST& source );
};

class TypeSpecifierAST: public AST
{
public:
    typedef std::auto_ptr<TypeSpecifierAST> Node;
    enum { Type = NodeType_TypeSpecifier };

public:
    TypeSpecifierAST();
    virtual ~TypeSpecifierAST();
    
private:
    TypeSpecifierAST( const TypeSpecifierAST& source );
    void operator = ( const TypeSpecifierAST& source );
};

class BaseSpecifierAST: public AST
{
public:
    typedef std::auto_ptr<BaseSpecifierAST> Node;
    
public:
    BaseSpecifierAST();
    virtual ~BaseSpecifierAST();
            
private:
    BaseSpecifierAST( const BaseSpecifierAST& source );
    void operator = ( const BaseSpecifierAST& source );
};

class BaseClauseAST: public AST
{
public:
    typedef std::auto_ptr<BaseClauseAST> Node;
    
public:
    BaseClauseAST();
    virtual ~BaseClauseAST();
    
    void addBaseSpecifier( BaseSpecifierAST::Node& baseSpecifier );
    QPtrList<BaseSpecifierAST> baseSpecifiers() { return m_baseSpecifiers; }
    
private:
    QPtrList<BaseSpecifierAST> m_baseSpecifiers;
    
private:
    BaseClauseAST( const BaseClauseAST& source );
    void operator = ( const BaseClauseAST& source );
};

class ClassSpecifierAST: public TypeSpecifierAST
{
public:
    typedef std::auto_ptr<ClassSpecifierAST> Node;
    enum { Type = NodeType_ClassOrNamespaceName };

public:
    ClassSpecifierAST();
    virtual ~ClassSpecifierAST();    
    
    AST* classKey();
    void setClassKey( AST::Node& classKey );
    
    NameAST* name();
    void setName( NameAST::Node& name );
    
    void addDeclaration( DeclarationAST::Node& declaration );
    QPtrList<DeclarationAST> declarations() { return m_declarations; }
    
private:
    AST::Node m_classKey;
    NameAST::Node m_name;
    QPtrList<DeclarationAST> m_declarations;
    
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
    virtual ~EnumeratorAST();
    
    AST* id();
    void setId( AST::Node& id );
    
    AST* expr();
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
    virtual ~EnumSpecifierAST();
    
    AST* name();
    void setName( AST::Node& name );
    
    void addEnumerator( EnumeratorAST::Node& enumerator );
    QPtrList<EnumeratorAST> enumerators() { return m_enumerators; }

private:
    AST::Node m_name;
    QPtrList<EnumeratorAST> m_enumerators;
        
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
    virtual ~ElaboratedTypeSpecifierAST();
    
    AST* kind();
    void setKind( AST::Node& kind );
    
    NameAST* name();
    void setName( NameAST::Node& name );
        
private:
    AST::Node m_kind;
    NameAST::Node m_name;
    
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
    virtual ~LinkageBodyAST();

    void addDeclaration( DeclarationAST::Node& ast );
    QPtrList<DeclarationAST> declarations() { return m_declarations; }

private:
    QPtrList<DeclarationAST> m_declarations;

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
    virtual ~LinkageSpecificationAST();
    
    QString externType() const;
    void setExternType( const QString& type );
    
    LinkageBodyAST* linkageBody();
    void setLinkageBody( LinkageBodyAST::Node& linkageBody );
    
    DeclarationAST* declaration();
    void setDeclaration( DeclarationAST::Node& decl );
    
private:
    QString m_externType;
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
    virtual ~NamespaceAST();
    
    QString namespaceName() const;
    void setNamespaceName( const QString& name );
    
    LinkageBodyAST* linkageBody();
    void setLinkageBody( LinkageBodyAST::Node& linkageBody );
    
private:
    QString m_namespaceName;
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
    virtual ~NamespaceAliasAST();
    
    QString namespaceName() const;
    void setNamespaceName( const QString& name );
    
    NameAST* aliasName();
    void setAliasName( NameAST::Node& name );
    
private:
    QString m_namespaceName;
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
    virtual ~UsingAST();
    
    bool isTypename() const;
    void setTypename( bool b );
    
    NameAST* name();
    void setName( NameAST::Node& name );
    
private:
    bool m_typename;
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
    virtual ~UsingDirectiveAST();
        
    NameAST* name();
    void setName( NameAST::Node& name );
    
private:
    NameAST::Node m_name;
    
private:
    UsingDirectiveAST( const UsingDirectiveAST& source );
    void operator = ( const UsingDirectiveAST& source );
};

class InitDeclaratorListAST: public AST
{
public:
    typedef std::auto_ptr<InitDeclaratorListAST> Node;
    enum { Type = NodeType_InitDeclaratorList };

public:
    InitDeclaratorListAST();
    virtual ~InitDeclaratorListAST();
    
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
    virtual ~TypedefAST();
    
    TypeSpecifierAST* typeSpec();
    void setTypeSpec( TypeSpecifierAST::Node& typeSpec );
    
    InitDeclaratorListAST* initDeclaratorList();
    void setInitDeclaratorList( InitDeclaratorListAST::Node& initDeclaratorList );
    
private:
    TypeSpecifierAST::Node m_typeSpec;
    InitDeclaratorListAST::Node m_initDeclaratorList;
    
private:
    TypedefAST( const TypedefAST& source );
    void operator = ( const TypedefAST& source );
};

class DeclaratorAST: public AST
{
public:
    typedef std::auto_ptr<DeclaratorAST> Node;
    enum { Type = NodeType_Declarator };

public:
    DeclaratorAST();
    virtual ~DeclaratorAST();
    
    QPtrList<AST> ptrOpList();
    void addPtrOp( AST::Node& ptrOp );
    
    DeclaratorAST* subDeclarator();
    void setSubDeclarator( std::auto_ptr<DeclaratorAST>& subDeclarator );
    
    NameAST* declaratorId();
    void setDeclaratorId( NameAST::Node& declaratorId );
    
    AST* bitfieldInitialization();
    void setBitfieldInitialization( AST::Node& bitfieldInitialization );
    
    QPtrList<AST> arrayDimensionList();
    void addArrayDimension( AST::Node& arrayDimension );
    
    AST* parameterDeclarationClause();
    void setParameterDeclarationClause( AST::Node& parameterDeclarationClause );
    
    bool isConstMethod();
    void setIsConstMethod( bool isConstMethod );
    
    AST* exceptionSpecification();
    void setExceptionSpecification( AST::Node& exceptionSpecification );
    
private:
    QPtrList<AST> m_ptrOpList;
    std::auto_ptr<DeclaratorAST> m_subDeclarator;
    NameAST::Node m_declaratorId;
    AST::Node m_bitfieldInitialization;
    QPtrList<AST> m_arrayDimensionList;
    AST::Node m_parameterDeclarationClause;
    bool m_isConstMethod;
    AST::Node m_exceptionSpecification;
};

class InitDeclaratorAST: public AST
{
public:
    typedef std::auto_ptr<InitDeclaratorAST> Node;
    enum { Type = NodeType_InitDeclarator };

public:
    InitDeclaratorAST();
    virtual ~InitDeclaratorAST();
    
    DeclaratorAST* declarator();
    void setDeclarator( DeclaratorAST::Node& declarator );
    
    AST* initializer();
    void setInitializer( AST::Node& initializer );
    
private:
    DeclaratorAST::Node m_declarator;
    AST::Node m_initializer;
};

class TemplateDeclarationAST: public DeclarationAST
{
public:
    typedef std::auto_ptr<TemplateDeclarationAST> Node;
    enum { Type = NodeType_TemplateDeclaration };

public:
    TemplateDeclarationAST();
    virtual ~TemplateDeclarationAST();
    
    bool isExport() const;
    void setExport( bool b );
    
    AST* templateParameterList();
    void setTemplateParameterList( AST::Node& templateParameterList );
    
    DeclarationAST* declaration();
    void setDeclaration( DeclarationAST::Node& declaration );
    
private:
    bool m_export;
    AST::Node m_templateParameterList;
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
    virtual ~SimpleDeclarationAST();
    
    TypeSpecifierAST* typeSpec();
    void setTypeSpec( TypeSpecifierAST::Node& typeSpec );

    InitDeclaratorListAST* initDeclaratorList();
    void setInitDeclaratorList( InitDeclaratorListAST::Node& initDeclaratorList );
    
private:
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
    virtual ~StatementAST();
    
private:
    StatementAST( const StatementAST& source );
    void operator = ( const StatementAST& source );
};

class TranslationUnitAST: public AST
{
public:
    typedef std::auto_ptr<TranslationUnitAST> Node;
    enum { Type = NodeType_TranslationUnit };

public:
    TranslationUnitAST();
    virtual ~TranslationUnitAST();
    
    void addDeclaration( DeclarationAST::Node& ast );
    QPtrList<DeclarationAST> declarations() { return m_declarations; }
    
private:
    QPtrList<DeclarationAST> m_declarations;
   
private:
    TranslationUnitAST( const TranslationUnitAST& source );
    void operator = ( const TranslationUnitAST& source );
};

#endif 
