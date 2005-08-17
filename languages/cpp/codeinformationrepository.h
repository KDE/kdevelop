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
//Added by qt3to4:
#include <Q3ValueList>

#include <ktexteditor/codecompletioninterface.h>

class KDevCodeRepository;

class CodeInformationRepository
{
public:
	CodeInformationRepository( KDevCodeRepository* rep );
	virtual ~CodeInformationRepository();

	static Q3ValueList<KTextEditor::CompletionEntry> toEntryList( const Q3ValueList<Tag>& tags,
	        CppCodeCompletion::CompletionMode mode = CppCodeCompletion::NormalCompletion );
	static KTextEditor::CompletionEntry toEntry( Tag& tag, CppCodeCompletion::CompletionMode mode = CppCodeCompletion::NormalCompletion );
	Q3ValueList<KTextEditor::CompletionEntry> getEntriesInScope( const QStringList& scope, bool isInstance, bool recompute = false );

	Q3ValueList<Tag> query( const Q3ValueList<Catalog::QueryArgument>& args );
	Q3ValueList<Tag> getTagsInScope( const QStringList& scope, bool isInstance );
	Q3ValueList<Tag> getTagsInScope( const QString& name, const QStringList& scope );

	Q3ValueList<Tag> getTagsInFile( const QString& fileName );
	Q3ValueList<Tag> getBaseClassList( const QString& className );
	Q3ValueList<Tag> getClassOrNamespaceList( const QStringList& scope );

private:
	Q3ValueList<KTextEditor::CompletionEntry> m_globalEntries;
	KDevCodeRepository* m_rep;

private:
	CodeInformationRepository( const CodeInformationRepository& source );
	void operator = ( const CodeInformationRepository& source );
};

#endif 
// kate: indent-mode csands; tab-width 4;

