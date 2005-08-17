/***************************************************************************
*   Copyright (C) 2003 by Roberto Raggi                                   *
*   roberto@kdevelop.org                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef __tag_creator_h
#define __tag_creator_h

#include "tree_parser.h"

#include <qstringlist.h>
//Added by qt3to4:
#include <Q3ValueList>

class Catalog;
class Tag;

namespace TagUtils
{
	int stringToAccess( const QString& access );
	QString accessToString( int id );
}

class TagCreator: public TreeParser
{
public:
	TagCreator( const QString& fileName, Catalog* c );
	virtual ~TagCreator();

	// translation-unit
	virtual void parseTranslationUnit( TranslationUnitAST* );

	// declarations
	virtual void parseDeclaration( DeclarationAST* );
	virtual void parseNamespace( NamespaceAST* );
	virtual void parseUsingDirective( UsingDirectiveAST* );
	virtual void parseTypedef( TypedefAST* );
	virtual void parseTemplateDeclaration( TemplateDeclarationAST* );
	virtual void parseSimpleDeclaration( SimpleDeclarationAST* );
	virtual void parseFunctionDefinition( FunctionDefinitionAST* );
	virtual void parseLinkageBody( LinkageBodyAST* );
	virtual void parseAccessDeclaration( AccessDeclarationAST* );

	// type-specifier
	virtual void parseClassSpecifier( ClassSpecifierAST* );
	virtual void parseEnumSpecifier( EnumSpecifierAST* );
	virtual void parseElaboratedTypeSpecifier( ElaboratedTypeSpecifierAST* );

	virtual void parseMyDeclaration( GroupAST* funSpec, GroupAST* storageSpec, TypeSpecifierAST* typeSpec, InitDeclaratorAST* decl );
	virtual void parseFunctionDeclaration( GroupAST* funSpec, GroupAST* storageSpec, TypeSpecifierAST* typeSpec, InitDeclaratorAST* decl );
	virtual void parseFunctionArguments( Tag& tag, DeclaratorAST* declarator );
	virtual void parseBaseClause( const QString& className, BaseClauseAST* baseClause );

	/**
	 call this function after the last use of a TagCreator object, to do cleanup work
	*/
	static void destroyDocumentation();

	/**
	This sets the directories, where the doxygen documentation should be searched in.
	@param dirs QStringList containing strings, which define the pathes, where documentation is searched in
	*/
	static void setDocumentationDirectories( const QStringList& dirs );


private:
	QString scopeOfDeclarator( DeclaratorAST* d );
	QString typeOfDeclaration( TypeSpecifierAST* typeSpec, DeclaratorAST* declarator );

private:
	Catalog* m_catalog;
	QString m_fileName;
	QStringList m_currentScope;
	Q3ValueList<QStringList> m_imports;
	QString m_currentAccess;
	bool m_inClass;
	bool m_inSlots;
	bool m_inSignals;
	int m_anon;

	static class DoxyDoc* m_documentation;

private:
	TagCreator( const TagCreator& source );
	void operator = ( const TagCreator& source );
};

#endif // __tag_creator_h 
// kate: indent-mode csands; tab-width 4;

