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
#include <kdebug.h>

CodeInformationRepository::CodeInformationRepository()
{
}

CodeInformationRepository::~CodeInformationRepository()
{
}

void CodeInformationRepository::addCatalog( const QString & id, Catalog * catalog )
{
    m_catalogs[ id ] = catalog;
}

void CodeInformationRepository::removeCatalog( const QString & id )
{
    m_catalogs.remove( id );
}

QValueList<Tag> CodeInformationRepository::query( const QValueList<Catalog :: QueryArgument> & args )
{
    kdDebug(9020) << "CodeInformationRepository::query()" << endl;

    QValueList<Tag> tags;
	
    QMap<QString, Catalog*>::Iterator it = m_catalogs.begin();
    while( it != m_catalogs.end() ){
        kdDebug(9020) << "try with the catalog " << it.key() << endl;

        Catalog* catalog = it.data();
        ++it;

        tags += catalog->query( args );
        kdDebug(9020) << "found " << tags.size() << " tags" << endl;
    }

    return tags;
}

QValueList<Tag> CodeInformationRepository::getTagsInFile( const QString & fileName )
{
    kdDebug(9020) << "CodeInformationRepository::getTagsInFile()" << endl;

    QValueList<Catalog::QueryArgument> args;
    args << Catalog::QueryArgument( "fileName", fileName );

    QMap<QString, Catalog*>::Iterator it = m_catalogs.begin();
    while( it != m_catalogs.end() ){
        Catalog* catalog = it.data();
        ++it;

        QValueList<Tag> tags = catalog->query( args );

        if( tags.size() )
            return tags;
    }

    return QValueList<Tag>();
}

QValueList<Tag> CodeInformationRepository::getTagsInScope( const QStringList & scope, bool isInstance )
{
    kdDebug(9020) << "CodeInformationRepository::getTagsInScope()" << endl;
    
    QValueList<Tag> tags;
    QValueList<Catalog::QueryArgument> args;
    
    args.clear();
    args << Catalog::QueryArgument( "kind", Tag::Kind_Namespace )
	<< Catalog::QueryArgument( "scope", scope );
    tags += query( args );
    
    args.clear();
    args << Catalog::QueryArgument( "kind", Tag::Kind_Class )
	<< Catalog::QueryArgument( "scope", scope );
    tags += query( args );
    
    args.clear();
    args << Catalog::QueryArgument( "kind", Tag::Kind_FunctionDeclaration )
	<< Catalog::QueryArgument( "scope", scope );
    
    if( !isInstance ){
	QValueList<Tag> temps = query( args );
	QValueList<Tag>::Iterator it = temps.begin();
	while( it != temps.end() ){
	    const Tag& tag = *it;
	    ++it;
	    
	    if( tag.hasAttribute("isStatic") && tag.attribute("isStatic").toBool() )
		continue; // 
	    
	    tags << tag;
	}
    } else {
	tags += query( args );
    }

    args.clear();
    args << Catalog::QueryArgument( "kind", Tag::Kind_Variable )
    	<< Catalog::QueryArgument( "scope", scope );
    
    if( !isInstance ){
	QValueList<Tag> temps = query( args );
	QValueList<Tag>::Iterator it = temps.begin();
	while( it != temps.end() ){
	    const Tag& tag = *it;
	    ++it;
	    
	    if( tag.hasAttribute("isStatic") && tag.attribute("isStatic").toBool() )
		continue; // 
	    
	    tags << tag;
	}
    } else {
	tags += query( args );
    }
    
    if( !isInstance ){
	args.clear();
	args << Catalog::QueryArgument( "kind", Tag::Kind_Enumerator )
	    << Catalog::QueryArgument( "scope", scope );
	tags += query( args );
    }
    
    return tags;
}

QValueList<KTextEditor::CompletionEntry> CodeInformationRepository::getEntriesInScope( const QStringList & scope, bool isInstance )
{
    kdDebug(9020) << "CodeInformationRepository::getEntriesInScope()" << endl;
    
    return toEntryList( getTagsInScope(scope, isInstance) );
}

QValueList<Tag> CodeInformationRepository::getBaseClassList( const QString& className )
{
    kdDebug(9020) << "CodeInformationRepository::getBaseClasseList()" << endl;

    QValueList<Catalog::QueryArgument> args;
    args << Catalog::QueryArgument( "kind", Tag::Kind_Base_class )
    	<< Catalog::QueryArgument( "name", className );
    return query( args );
}

QStringList CodeInformationRepository::getSignatureList( const QStringList & scope, const QString & functionName, bool /*isInstance*/ )
{
    kdDebug(9020) << "CodeInformationRepository::getSignatureList()" << endl;

    QValueList<Catalog::QueryArgument> args;
    args << Catalog::QueryArgument( "kind", Tag::Kind_FunctionDeclaration )
    	<< Catalog::QueryArgument( "scope", scope )
    	<< Catalog::QueryArgument( "name", functionName );

    QValueList<Tag> tags = query( args );

    QStringList list;
    QValueList<Tag>::Iterator it = tags.begin();
    while( it != tags.end() ){
        const Tag& tag = *it;
        ++it;

        QString signature;
        signature += tag.name() + "(";
	QStringList arguments = tag.attribute( "arguments" ).toStringList();
	QStringList argumentNames = tag.attribute( "argumentNames" ).toStringList();
	
	for( uint i=0; i<arguments.size(); ++i ){
	    signature += arguments[ i ];
	    QString argName = argumentNames[ i ];
	    if( !argName.isEmpty() )
		signature += QString::fromLatin1( " " ) + argName;
	    
	    if( i != (arguments.size()-1) ){
		signature += ", ";
	    }
	}
        signature += ")";

        list << signature;
    }

    return list;
}

QValueList<Tag> CodeInformationRepository::getClassOrNamespaceList( const QStringList & scope )
{
    kdDebug(9020) << "CodeInformationRepository::getClassOrNamespaceList()" << endl;

    QValueList<Tag> tags;    
    QValueList<Catalog::QueryArgument> args;
    
    args.clear();
    args << Catalog::QueryArgument( "kind", Tag::Kind_Namespace )
	<< Catalog::QueryArgument( "scope", scope );
    tags += query( args );
    
    args << Catalog::QueryArgument( "kind", Tag::Kind_Class )
    	<< Catalog::QueryArgument( "scope", scope );
    
    tags += query( args );
        
    return tags;
}

QValueList<Tag> CodeInformationRepository::getTagsInScope( const QString & name, const QStringList & scope )
{
    QValueList<Tag> tags;    
    QValueList<Catalog::QueryArgument> args;
    
    args.clear();
    args << Catalog::QueryArgument( "scope", scope )
	<< Catalog::QueryArgument( "name", name );
    
    tags += query( args );
    
    return tags;
}

QValueList<KTextEditor :: CompletionEntry> CodeInformationRepository::toEntryList( const QValueList<Tag> & tags )
{
    QValueList<KTextEditor :: CompletionEntry> entryList;
    
    QValueList<Tag>::ConstIterator it = tags.begin();
    while( it != tags.end() ){
	const Tag& tag = *it;
	++it;
	
	KTextEditor::CompletionEntry entry;
	switch( tag.kind() ){
	    case Tag::Kind_Class:
		entry.prefix = "class";
		entry.text = tag.name();
		break;
		
	    case Tag::Kind_Namespace:
		entry.prefix = "namespace";
		entry.text = tag.name();
		break;
		
	    case Tag::Kind_FunctionDeclaration:
	    case Tag::Kind_Function:
	    {
		entry.text = tag.name();
		
		entry.text += "(";
		
		QStringList arguments = tag.attribute( "arguments" ).toStringList();
		QStringList argumentNames = tag.attribute( "argumentNames" ).toStringList();
		
		QString signature;
		for( uint i=0; i<arguments.size(); ++i ){
		    signature += arguments[ i ];
		    QString argName = argumentNames[ i ];
		    if( !argName.isEmpty() )
			signature += QString::fromLatin1( " " ) + argName;
		    
		    if( i != (arguments.size()-1) ){
			signature += ", ";
		    }
		}
		
		if( signature.isEmpty() )
		    entry.text += ")";
		else
		    entry.postfix = signature + ")";		
	    }
	    break;
	    
	    case Tag::Kind_Enumerator:
	    case Tag::Kind_Variable:
	    case Tag::Kind_VariableDeclaration:
	        entry.text = tag.name();
	        break;
	    
	    default:
		;
	}
	
	entryList << entry;
    }
    
    return entryList;
}

