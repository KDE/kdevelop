/***************************************************************************
 *   Copyright (C) 2002 by Roberto Raggi                                   *
 *   raggi@cli.di.unipi.it                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef  __ast_h
#define __ast_h

#include <memory>
#include <qstring.h>
#include <qptrlist.h>

template <class T> typename T::Node CreateNode()
{
    typename T::Node node( new T );
    return node;
}

template <class T> typename T::Node NullNode()
{
    typename T::Node node;
    return node;
}

class AST
{
public:
    typedef std::auto_ptr<AST> Node;
    
public:
    AST();
    virtual ~AST();
    
    void setStartPosition( int line, int col );
    void getStartPosition( int* line, int* col ) const;
    
    void setEndPosition( int line, int col );
    void getEndPosition( int* line, int* col ) const;
    
private:
    int m_startLine, m_startColumn;
    int m_endLine, m_endColumn;
    
private:
    AST( const AST& source );
    void operator = ( const AST& source );
};

class TemplateArgumentListAST: public AST
{
public:
    typedef std::auto_ptr<TemplateArgumentListAST> Node;
    
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
    
public:
    DeclarationAST();
    virtual ~DeclarationAST();
    
private:
    DeclarationAST( const DeclarationAST& source );
    void operator = ( const DeclarationAST& source );
};

// type-specifier
//   simple-type-specifier
//   class-specifier
//   enum-specifier
//   elaborated-type-specifier
//   cv-qualifier

class TypeSpecifierAST: public AST
{
public:
    typedef std::auto_ptr<TypeSpecifierAST> Node;
    
public:
    TypeSpecifierAST();
    virtual ~TypeSpecifierAST();
    
private:
    TypeSpecifierAST( const TypeSpecifierAST& source );
    void operator = ( const TypeSpecifierAST& source );
};

class ClassSpecifierAST: public TypeSpecifierAST
{
public:
    typedef std::auto_ptr<ClassSpecifierAST> Node;
    
public:
    ClassSpecifierAST();
    virtual ~ClassSpecifierAST();    
    
private:
    ClassSpecifierAST( const ClassSpecifierAST& source );
    void operator = ( const ClassSpecifierAST& source );
};

class EnumeratorAST: public AST
{
public:
    typedef std::auto_ptr<EnumeratorAST> Node;
    
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

class TypedefAST: public DeclarationAST
{
public:
    typedef std::auto_ptr<TypedefAST> Node;
    
public:
    TypedefAST();
    virtual ~TypedefAST();
    
    TypeSpecifierAST* typeSpec();
    void setTypeSpec( TypeSpecifierAST::Node& typeSpec );
    
    AST* initDeclaratorList();
    void setInitDeclaratorList( AST::Node& initDeclaratorList );
    
private:
    TypeSpecifierAST::Node m_typeSpec;
    AST::Node m_initDeclaratorList;
    
private:
    TypedefAST( const TypedefAST& source );
    void operator = ( const TypedefAST& source );
};

class TemplateDeclarationAST: public DeclarationAST
{
public:
    typedef std::auto_ptr<TemplateDeclarationAST> Node;
    
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

class StatementAST: public AST
{
public:
    typedef std::auto_ptr<StatementAST> Node;
    
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
