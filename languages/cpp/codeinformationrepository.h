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

#ifndef CODEINFORMATIONREPOSITORY_H
#define CODEINFORMATIONREPOSITORY_H

#include "catalog.h"
#include "cppcodecompletion.h"
#include <qmap.h>

#include <ktexteditor/codecompletioninterface.h>

struct TypeProcessor {
    virtual QString processType( const QString& type ) = 0;
    virtual QString parentType() = 0;
};

class KDevCodeRepository;

class CodeInformationRepository
{
public:
	CodeInformationRepository( KDevCodeRepository* rep );
	virtual ~CodeInformationRepository();

	static QValueList<KTextEditor::CompletionEntry> toEntryList( const QValueList<Tag>& tags,
	        CppCodeCompletion::CompletionMode mode = CppCodeCompletion::NormalCompletion );
	static KTextEditor::CompletionEntry toEntry( Tag& tag, CppCodeCompletion::CompletionMode mode = CppCodeCompletion::NormalCompletion, TypeProcessor* proc = 0 );
	QValueList<KTextEditor::CompletionEntry> getEntriesInScope( const QStringList& scope, bool isInstance, bool recompute = false );

	QValueList<Tag> query( const QValueList<Catalog::QueryArgument>& args );
	QValueList<Tag> getTagsInScope( const QStringList& scope, bool isInstance );
	QValueList<Tag> getTagsInScope( const QString& name, const QStringList& scope );

	QValueList<Tag> getTagsInFile( const QString& fileName );
	QValueList<Tag> getBaseClassList( const QString& className );
	QValueList<Tag> getClassOrNamespaceList( const QStringList& scope );

private:
	QValueList<KTextEditor::CompletionEntry> m_globalEntries;
	KDevCodeRepository* m_rep;

private:
	CodeInformationRepository( const CodeInformationRepository& source );
	void operator = ( const CodeInformationRepository& source );
};

#endif 
// kate: indent-mode csands; tab-width 4;

