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

#include "lexer.h"

#include <qstring.h>
#include <qptrlist.h>

class AST{
public:
    AST() {}
    virtual ~AST() {}
	
// properties:
	Token start;
	Token end;
};

class DeclarationAST: public AST{
public:
	DeclarationAST() {}
	virtual ~DeclarationAST() {}
};

class LinkageBodyAST: public AST{
public:
	LinkageBodyAST()
	{
		m_declarations.setAutoDelete( true );
	}
	
	virtual ~LinkageBodyAST() {}
	
	void addDeclaration( DeclarationAST* decl )
	{
		m_declarations.append( decl );
	}
		
private:
	QPtrList<DeclarationAST> m_declarations;
};

class LinkageSpecificationAST: public DeclarationAST{
public:
	LinkageSpecificationAST() {}
	virtual ~LinkageSpecificationAST() {}
	
	QString type() const { return m_type; }
	void setType( const QString& type ) { m_type = type; }
	
private:
	QString m_type;
};

class SimpleLinkageSpecificationAST: public LinkageSpecificationAST{
public:
	SimpleLinkageSpecificationAST()
		: m_declaration( 0 )
		{}
		
	virtual ~SimpleLinkageSpecificationAST() 
	{
		delete( m_declaration );
	}
	
	DeclarationAST* declaration() { return m_declaration; }
	void setDeclaration( DeclarationAST* decl ) { m_declaration = decl; }
	
private:
	DeclarationAST* m_declaration;
};

class BlockLinkageSpecificationAST: public LinkageSpecificationAST{
public:
	BlockLinkageSpecificationAST()
		: m_linkageBody(0) 
		{}
		
	virtual ~BlockLinkageSpecificationAST() 
	{
		delete( m_linkageBody );
	}
	
	LinkageBodyAST* linkageBody() const { return m_linkageBody; }
	void setLinkageBody( LinkageBodyAST* linkageBody ) { m_linkageBody = linkageBody; }
	
private:
	LinkageBodyAST* m_linkageBody;
};

class NamespaceDeclarationAST: public DeclarationAST{
public:
	NamespaceDeclarationAST()
		: m_linkageBody( 0 )
		{}
		
	virtual ~NamespaceDeclarationAST()
	{
		delete( m_linkageBody );
	}
	
	QString name() const { return m_name; }
	void setName( const QString& name ) { m_name = name; }
	
	LinkageBodyAST* linkageBody() const { return m_linkageBody; }
	void setLinkageBody( LinkageBodyAST* linkageBody ) { m_linkageBody = linkageBody; }
	
private:
	QString m_name;
	LinkageBodyAST* m_linkageBody;
};

class NamespaceAliasDefinitionAST: public DeclarationAST{
public:
	NamespaceAliasDefinitionAST() {}
	virtual ~NamespaceAliasDefinitionAST() {}
	
	QString name() const { return m_name; }
	void setName( const QString& name ) { m_name = name; }
	
	QString alias() const { return m_alias; }
	void setAlias( const QString& alias ) { m_alias = alias; }
	
private:
	QString m_name;
	QString m_alias;
};

class UsingDeclarationAST: public DeclarationAST{
public:
	UsingDeclarationAST() {}
	virtual ~UsingDeclarationAST() {}
	
	QString name() const { return m_name; }
	void setName( const QString& name ) { m_name = name; }
	
private:
	QString m_name;
};

class UsingDirectiveAST: public DeclarationAST{
public:
	UsingDirectiveAST() {}
	virtual ~UsingDirectiveAST() {}
	
	QString name() const { return m_name; }
	void setName( const QString& name ) { m_name = name; }
	
private:
	QString m_name;
};

class TypedefDeclarationAST: public DeclarationAST{
public:
	TypedefDeclarationAST() {}
	virtual ~TypedefDeclarationAST() {}

	QString name() const { return m_name; }
	void setName( const QString& name ) { m_name = name; }	
	
private:
	QString m_name;
};

class AsmDefinitionAST: public DeclarationAST{
public:
	AsmDefinitionAST() {}
	virtual ~AsmDefinitionAST() {}
};

class TemplateDeclarationAST: public DeclarationAST{
public:
	TemplateDeclarationAST()
		: m_declaration( 0 )
		{}
		
	virtual ~TemplateDeclarationAST()
	{
		delete( m_declaration );
	}
	
	QString parameters() const { return m_parameters; }
	void setParameters( const QString& p ) { m_parameters = p; }
	
	void setDeclaration( DeclarationAST* decl )
	{
		m_declaration = decl;
	}
	
private:
	QString m_parameters;
	DeclarationAST* m_declaration;
};

class ClassDeclarationAST: public DeclarationAST{
public:
	ClassDeclarationAST()
	{
		m_declarations.setAutoDelete( true );
	}
	
	virtual ~ClassDeclarationAST() 
		{}
		
	QString name() const { return m_name; }
	void setName( const QString& name ) { m_name = name; }
		
	void addDeclaration( DeclarationAST* decl )
	{
		m_declarations.append( decl );
	}
	
private:
	QString m_name;
	QPtrList<DeclarationAST> m_declarations;
};

class EnumDeclarationAST: public DeclarationAST{
public:
	EnumDeclarationAST() {}
	virtual ~EnumDeclarationAST() {}
	
	QString name() const { return m_name; }
	void setName( const QString& name ) { m_name = name; }
	
private:
	QString m_name;
};

class DeclaratorAST: public AST{
public:
	DeclaratorAST() {}
	virtual ~DeclaratorAST() {}
};


class TranslationUnitAST: public AST{
public:
	TranslationUnitAST()
	{
		m_declarations.setAutoDelete( true );
	}
	
	virtual ~TranslationUnitAST() 
		{}
	
	void addDeclaration( DeclarationAST* decl )
	{
		m_declarations.append( decl );
	}
		
private:
	QPtrList<DeclarationAST> m_declarations;
};

#endif
