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

class AST
{
public:
    typedef std::auto_ptr<AST> Ptr;
    
public:
    AST();
    virtual ~AST();
    
    void setStart( int start ) { m_start = start; }
    int start() const { return m_start; }
    
    void setEnd( int end ) { m_end = end; }
    int end() const { return m_end; }
    
private:
    int m_start;
    int m_end;
    
private:
    AST( const AST& source );
    void operator = ( const AST& source );
};

class NameAST: public AST
{
public:
    typedef std::auto_ptr<NameAST> Ptr;
    
public:
    NameAST();
    virtual ~NameAST();
        
    bool isGlobal() const;
    void setGlobal( bool b );
    
    void setNestedName( AST::Ptr& nestedName );
    AST* nestedName();
    
    void setUnqualifedName( AST::Ptr& unqualifiedName );
    AST* unqualifiedName();
    
private:
    bool m_global;
    AST::Ptr m_nestedName;
    AST::Ptr m_unqualifiedName;
    
private:
    NameAST( const NameAST& source );
    void operator = ( const NameAST& source );
};

class DeclarationAST: public AST
{
public:
    typedef std::auto_ptr<DeclarationAST> Ptr;
    
public:
    DeclarationAST();
    virtual ~DeclarationAST();
    
private:
    DeclarationAST( const DeclarationAST& source );
    void operator = ( const DeclarationAST& source );
};

class LinkageBodyAST: public AST
{
public:
    typedef std::auto_ptr<LinkageBodyAST> Ptr;
    
public:
    LinkageBodyAST();
    virtual ~LinkageBodyAST();
    
    void addDeclaration( DeclarationAST::Ptr& ast );
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
    typedef std::auto_ptr<LinkageSpecificationAST> Ptr;
    
public:
    LinkageSpecificationAST();
    virtual ~LinkageSpecificationAST();
    
    QString externType() const;
    void setExternType( const QString& type );
    
    LinkageBodyAST* linkageBody();
    void setLinkageBody( LinkageBodyAST::Ptr& linkageBody );
    
    DeclarationAST* declaration();
    void setDeclaration( DeclarationAST::Ptr& decl );
    
private:
    QString m_externType;
    LinkageBodyAST::Ptr m_linkageBody;
    DeclarationAST::Ptr m_declaration;
        
private:
    LinkageSpecificationAST( const LinkageSpecificationAST& source );
    void operator = ( const LinkageSpecificationAST& source );
};

class NamespaceAST: public DeclarationAST
{
public:
    typedef std::auto_ptr<NamespaceAST> Ptr;
    
public:
    NamespaceAST();
    virtual ~NamespaceAST();
    
    QString namespaceName() const;
    void setNamespaceName( const QString& name );
    
    LinkageBodyAST* linkageBody();
    void setLinkageBody( LinkageBodyAST::Ptr& linkageBody );
    
private:
    QString m_namespaceName;
    LinkageBodyAST::Ptr m_linkageBody;
    
private:
    NamespaceAST( const NamespaceAST& source );
    void operator = ( const NamespaceAST& source );
};

class NamespaceAliasAST: public DeclarationAST
{
public:
    typedef std::auto_ptr<NamespaceAliasAST> Ptr;
    
public:
    NamespaceAliasAST();
    virtual ~NamespaceAliasAST();
    
    QString namespaceName() const;
    void setNamespaceName( const QString& name );
    
    NameAST* aliasName();
    void setAliasName( NameAST::Ptr& name );
    
private:
    QString m_namespaceName;
    NameAST::Ptr m_aliasName;
    
private:
    NamespaceAliasAST( const NamespaceAliasAST& source );
    void operator = ( const NamespaceAliasAST& source );
};

class UsingAST: public DeclarationAST
{
public:
    typedef std::auto_ptr<UsingAST> Ptr;
    
public:
    UsingAST();
    virtual ~UsingAST();
    
    bool isTypename() const;
    void setTypename( bool b );
    
    NameAST* name();
    void setName( NameAST::Ptr& name );
    
private:
    bool m_typename;
    NameAST::Ptr m_name;
    
private:
    UsingAST( const UsingAST& source );
    void operator = ( const UsingAST& source );
};

class UsingDirectiveAST: public DeclarationAST
{
public:
    typedef std::auto_ptr<UsingDirectiveAST> Ptr;
    
public:
    UsingDirectiveAST();
    virtual ~UsingDirectiveAST();
        
    NameAST* name();
    void setName( NameAST::Ptr& name );
    
private:
    NameAST::Ptr m_name;
    
private:
    UsingDirectiveAST( const UsingDirectiveAST& source );
    void operator = ( const UsingDirectiveAST& source );
};

class TypedefAST: public DeclarationAST
{
public:
    typedef std::auto_ptr<TypedefAST> Ptr;
    
public:
    TypedefAST();
    virtual ~TypedefAST();
    
    AST* typeSpec();
    void setTypeSpec( AST::Ptr& typeSpec );
    
    AST* initDeclaratorList();
    void setInitDeclaratorList( AST::Ptr& initDeclaratorList );
    
private:
    AST::Ptr m_typeSpec;
    AST::Ptr m_initDeclaratorList;
    
private:
    TypedefAST( const TypedefAST& source );
    void operator = ( const TypedefAST& source );
};

class TranslationUnitAST: public AST
{
public:
    typedef std::auto_ptr<TranslationUnitAST> Ptr;
    
public:
    TranslationUnitAST();
    virtual ~TranslationUnitAST();
    
    void addDeclaration( DeclarationAST::Ptr& ast );
    QPtrList<DeclarationAST> declarations() { return m_declarations; }
    
private:
    QPtrList<DeclarationAST> m_declarations;
   
private:
    TranslationUnitAST( const TranslationUnitAST& source );
    void operator = ( const TranslationUnitAST& source );
};

#endif 
