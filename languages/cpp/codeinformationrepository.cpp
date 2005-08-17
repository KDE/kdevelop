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

#include "codeinformationrepository.h"
#include "cpp_tags.h"

#include <kdevcoderepository.h>
#include <kdebug.h>
//Added by qt3to4:
#include <Q3ValueList>

/// @todo move in utils.cpp
static Q3ValueList<KTextEditor::CompletionEntry>
my_unique( const Q3ValueList<KTextEditor::CompletionEntry>& entryList )
{

	Q3ValueList< KTextEditor::CompletionEntry > l;
	QMap<QString, bool> map;
	Q3ValueList< KTextEditor::CompletionEntry >::ConstIterator it = entryList.begin();
	while ( it != entryList.end() )
	{
		KTextEditor::CompletionEntry e = *it++;
		QString key = e.type + " " +
		              e.text + " " +
		              e.prefix + " " +
		              e.postfix + " ";
		if ( map.find( key ) == map.end() )
		{
			map[ key ] = TRUE;
			l << e;
		}
	}
	return l;
}

CodeInformationRepository::CodeInformationRepository( KDevCodeRepository* rep )
		: m_rep( rep )
{}

CodeInformationRepository::~CodeInformationRepository()
{}

Q3ValueList<Tag> CodeInformationRepository::query( const Q3ValueList<Catalog :: QueryArgument> & args )
{
	kdDebug( 9007 ) << "CodeInformationRepository::query()" << endl;

	Q3ValueList<Tag> tags;

	Q3ValueList<Catalog*> catalogs = m_rep->registeredCatalogs();
	Q3ValueList<Catalog*>::Iterator it = catalogs.begin();
	while ( it != catalogs.end() )
	{
		Catalog * catalog = *it;
		++it;

		if ( !catalog->enabled() )
			continue;

		tags += catalog->query( args );
	}

	return tags;
}

Q3ValueList<Tag> CodeInformationRepository::getTagsInFile( const QString & fileName )
{
	kdDebug( 9007 ) << "CodeInformationRepository::getTagsInFile()" << endl;

	Q3ValueList<Catalog::QueryArgument> args;
	args << Catalog::QueryArgument( "fileName", fileName );

	Q3ValueList<Catalog*> catalogs = m_rep->registeredCatalogs();
	Q3ValueList<Catalog*>::Iterator it = catalogs.begin();
	while ( it != catalogs.end() )
	{
		Catalog * catalog = *it;
		++it;

		Q3ValueList<Tag> tags = catalog->query( args );

		if ( tags.size() )
			return tags;
	}

	return Q3ValueList<Tag>();
}

Q3ValueList<Tag> CodeInformationRepository::getTagsInScope( const QStringList & scope, bool // isInstance
                                                         )
{
	kdDebug( 9007 ) << "CodeInformationRepository::getTagsInScope()" << endl;

	Q3ValueList<Tag> tags;
	Q3ValueList<Catalog::QueryArgument> args;

#if 0

	args.clear();
	args << Catalog::QueryArgument( "kind", Tag::Kind_Namespace )
	<< Catalog::QueryArgument( "scope", scope );
	tags += query( args );

	args.clear();
	args << Catalog::QueryArgument( "kind", Tag::Kind_Class )
	<< Catalog::QueryArgument( "scope", scope );
	tags += query( args );
#endif

	args.clear();
	args << Catalog::QueryArgument( "kind", Tag::Kind_FunctionDeclaration )
	<< Catalog::QueryArgument( "scope", scope );
	tags += query( args );

	args.clear();
	args << Catalog::QueryArgument( "kind", Tag::Kind_Variable )
	<< Catalog::QueryArgument( "scope", scope );
	tags += query( args );

	if ( true /*!isInstance*/ )
	{
		args.clear();
		args << Catalog::QueryArgument( "kind", Tag::Kind_Enumerator )
		<< Catalog::QueryArgument( "scope", scope );
		tags += query( args );
	}

	return tags;
}

Q3ValueList<KTextEditor::CompletionEntry> CodeInformationRepository::getEntriesInScope( const QStringList & scope, bool isInstance, bool recompute )
{
	kdDebug( 9007 ) << "CodeInformationRepository::getEntriesInScope()" << endl;

	if ( !recompute && !scope.size() && m_globalEntries.size() )
		return m_globalEntries;
	else if ( scope.size() == 0 )
	{
		m_globalEntries = my_unique( toEntryList( getTagsInScope( scope, isInstance ) ) );
		return m_globalEntries;
	}

	return toEntryList( getTagsInScope( scope, isInstance ) );
}


Q3ValueList<Tag> CodeInformationRepository::getBaseClassList( const QString& className )
{
	kdDebug( 9007 ) << "CodeInformationRepository::getBaseClassList()" << endl;

	if ( className.isEmpty() )
		return Q3ValueList<Tag>();

	Q3ValueList<Catalog::QueryArgument> args;
	args << Catalog::QueryArgument( "kind", Tag::Kind_Base_class );
	/*    if( className.length() >= 2 )
	        args << Catalog::QueryArgument( "prefix", className.left(2) );*/
	args << Catalog::QueryArgument( "name", className );
	return query( args );
}

Q3ValueList<Tag> CodeInformationRepository::getClassOrNamespaceList( const QStringList & scope )
{
	kdDebug( 9007 ) << "CodeInformationRepository::getClassOrNamespaceList()" << endl;

	Q3ValueList<Tag> tags;
	Q3ValueList<Catalog::QueryArgument> args;

	args << Catalog::QueryArgument( "kind", Tag::Kind_Namespace )
	<< Catalog::QueryArgument( "scope", scope );
	tags += query( args );

	args.clear();
	args << Catalog::QueryArgument( "kind", Tag::Kind_Class )
	<< Catalog::QueryArgument( "scope", scope );
	tags += query( args );

	return tags;
}

Q3ValueList<Tag> CodeInformationRepository::getTagsInScope( const QString & name, const QStringList & scope )
{
	Q3ValueList<Tag> tags;
	Q3ValueList<Catalog::QueryArgument> args;

	args.clear();
	args << Catalog::QueryArgument( "scope", scope );
	/*    if( name.length() >= 2 )
	        args << Catalog::QueryArgument( "prefix", name.left(2) );    */
	args << Catalog::QueryArgument( "name", name );

	tags += query( args );

	return tags;
}

KTextEditor::CompletionEntry CodeInformationRepository::toEntry( Tag & tag, CppCodeCompletion::CompletionMode completionMode )
{
	KTextEditor::CompletionEntry entry;

	if ( tag.name().isEmpty() )
		return entry;

	switch ( tag.kind() )
	{
	case Tag::Kind_Typedef:
		entry.prefix = "typedef";
		entry.text = tag.name();
		break;

	case Tag::Kind_Class:
		entry.prefix = "class";
		entry.text = tag.name();
		break;

	case Tag::Kind_Namespace:
		entry.prefix = "namespace";
		entry.text = tag.name();
		break;

	case Tag::Kind_FunctionDeclaration:
		//case Tag::Kind_Function:
		{

			CppFunction<Tag> tagInfo( tag );
			QStringList arguments = tagInfo.arguments();
			QStringList argumentNames = tagInfo.argumentNames();

			if ( completionMode == CppCodeCompletion::VirtualDeclCompletion )
			{
				//Ideally the type info would be a entry.prefix, but we need them to be
				//inserted upon completion so they have to be part of entry.text
				entry.text = tagInfo.type();
				entry.text += " ";
				entry.text += tag.name();
			}
			else
				entry.text = tag.name();
			
			if ( !arguments.size() )
				entry.text += "(";
			else
				entry.text += "( ";
			
			QString signature;
			for ( uint i = 0; i < arguments.size(); ++i )
			{
				signature += arguments[ i ];
				if ( completionMode == CppCodeCompletion::NormalCompletion ||
					 completionMode == CppCodeCompletion::VirtualDeclCompletion )
				{
					QString argName = argumentNames[ i ];
					if ( !argName.isEmpty() )
						signature += QString::fromLatin1( " " ) + argName;
				}

				if ( i != ( arguments.size() - 1 ) )
				{
					signature += ", ";
				}
			}

			if ( signature.isEmpty() )
				entry.text += ")";
			else
				entry.postfix = signature + " )";

			if ( tagInfo.isConst() )
				entry.postfix += " const";

			if ( completionMode == CppCodeCompletion::VirtualDeclCompletion )
			{
				entry.text += entry.postfix + ";";
				entry.postfix = QString::null;
			}
			else if ( completionMode != CppCodeCompletion::NormalCompletion )
			{
				entry.text += entry.postfix;
				entry.postfix = QString::null;
			}

			QString comment = tag.attribute( "description" ).toString();
			if ( !comment.isNull() )
				entry.comment = comment;
			//else
			//entry.comment = "no documentation available!";
		}

		break;

	case Tag::Kind_Enumerator:
	case Tag::Kind_Variable:
		entry.text = tag.name();
		break;

	default:
		;
	}

	return entry;
}

Q3ValueList<KTextEditor :: CompletionEntry> CodeInformationRepository::toEntryList( const Q3ValueList<Tag> & tags, CppCodeCompletion::CompletionMode completionMode )
{
	Q3ValueList<KTextEditor :: CompletionEntry> entryList;
	QMap<QString, bool> ns;

	Q3ValueList<Tag>::ConstIterator it = tags.begin();
	while ( it != tags.end() )
	{
		Tag tag = *it;
		++it;

		KTextEditor::CompletionEntry entry = toEntry( tag, completionMode );
		if ( !entry.text.isEmpty() )
			entryList << entry;
	}

	return entryList;
}


