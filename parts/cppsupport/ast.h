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

#ifndef AST_H
#define AST_H

#include <qstring.h>
#include <qptrlist.h>
#include <qdom.h>

class AST{
public:
    AST();
    virtual ~AST();

    virtual int start() const;
    virtual void setStart( int start );

    virtual int end() const;
    virtual void setEnd( int end );

    virtual QDomElement toXML( QDomDocument& ) { return QDomElement(); }

private:
    int m_start;
    int m_end;
};

class DeclaratorAST: public AST{
public:
    DeclaratorAST();
    ~DeclaratorAST();

    int nameStart() const;
    void setNameStart( int start );

    int nameEnd() const;
    void setNameEnd( int end );

    DeclaratorAST* subDeclarator() const;
    void setSubDeclarator( DeclaratorAST* sub );

    bool isArray() const;
    void setIsArray( bool b );

    bool isFunction() const;
    void setIsFunction( bool b );

private:
    int m_nameStart;
    int m_nameEnd;
    DeclaratorAST* m_sub;
    bool m_array;
    bool m_function;
};

class DeclarationAST: public AST{
public:
    DeclarationAST();
    virtual ~DeclarationAST();

    int nameStart() const;
    void setNameStart( int start );

    int nameEnd() const;
    void setNameEnd( int end );

private:
    int m_nameStart;
    int m_nameEnd;
};

class ClassDeclarationAST: public DeclarationAST{
public:
    ClassDeclarationAST();
    ~ClassDeclarationAST();
};

class MethodDeclarationAST: public DeclarationAST{
public:
    MethodDeclarationAST();
    ~MethodDeclarationAST();
};

class FieldDeclarationAST: public DeclarationAST{
public:
    FieldDeclarationAST();
    ~FieldDeclarationAST();
};

class NamespaceDeclarationAST: public DeclarationAST{
public:
    NamespaceDeclarationAST();
    ~NamespaceDeclarationAST();
};

class UsingDeclarationAST: public DeclarationAST{
public:
    UsingDeclarationAST();
    ~UsingDeclarationAST();
};

class TranslationUnitAST: public AST{
public:
    TranslationUnitAST();
    ~TranslationUnitAST();

    void addDeclaration( DeclarationAST* decl );

private:
    QPtrList<DeclarationAST> m_declarations;
};

class TypedefDeclarationAST: public DeclarationAST{
public:
    TypedefDeclarationAST();
    ~TypedefDeclarationAST();

    void setDeclarator( DeclaratorAST* declarator );

private:
    DeclaratorAST* m_declarator;
};

class AsmDeclaratationAST: public DeclarationAST{
public:
    AsmDeclaratationAST();
    ~AsmDeclaratationAST();
};

class TemplateDeclarationAST: public DeclarationAST{
public:
    TemplateDeclarationAST();
    ~TemplateDeclarationAST();

    int nameStart() const;
    int nameEnd() const;
    void setDeclaration( DeclarationAST* decl );

private:
    DeclarationAST* m_declaration;
};

class LinkageBodyAST: public DeclarationAST{
public:
    LinkageBodyAST();
    ~LinkageBodyAST();

    void addDeclaration( DeclarationAST* decl );

private:
    QPtrList<DeclarationAST> m_declarations;
};

class NullDeclarationAST: public DeclarationAST{
public:
    NullDeclarationAST();
    ~NullDeclarationAST();
};

class FriendDeclarationAST: public DeclarationAST{
public:
    FriendDeclarationAST();
    ~FriendDeclarationAST();

    int nameStart() const;
    int nameEnd() const;

    void setDeclaration( DeclarationAST* decl );

private:
    DeclarationAST* m_declaration;
};

class DeclaratorListAST: public AST{
public:
    DeclaratorListAST();
    ~DeclaratorListAST();

    void addDeclarator( DeclaratorAST* declarator );

private:
    QPtrList<DeclaratorAST> m_declarators;
};

#endif
