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

    QMap<QString, Catalog*>::Iterator it = m_catalogs.begin();
    while( it != m_catalogs.end() ){
        kdDebug(9020) << "try with the catalog " << it.key() << endl;

        Catalog* catalog = it.data();
        ++it;

        QValueList<Tag> tags = catalog->query( args );
        kdDebug(9020) << "found " << tags.size() << " tags" << endl;

        if( tags.size() )
            return tags;
    }

    return QValueList<Tag>();
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

QValueList<KTextEditor::CompletionEntry> CodeInformationRepository::getEntriesInScope( const QStringList & scope )
{
    kdDebug(9020) << "CodeInformationRepository::getEntriesInScope()" << endl;
    QValueList<KTextEditor::CompletionEntry> entryList;

    QValueList<Catalog::QueryArgument> args;
    args << Catalog::QueryArgument( "kind", Tag::Kind_FunctionDeclaration )
    	<< Catalog::QueryArgument( "scope", scope );
    QValueList<Tag> tags = query( args );

    args.clear();
    args << Catalog::QueryArgument( "kind", Tag::Kind_Variable )
    	<< Catalog::QueryArgument( "scope", scope );
    tags += query( args );

    QValueList<Tag>::Iterator it = tags.begin();
    while( it != tags.end() ){
        const Tag& tag = *it;
        ++it;

        KTextEditor::CompletionEntry entry;
        entry.text = tag.name();

        entry.prefix = QString::number( tag.kind() );
        if( tag.hasAttribute("arguments") ){
            entry.text += "(";
            entry.postfix += tag.attribute( "arguments" ).toStringList().join( ", " ) + ")";
        }

        entryList << entry;
    }

    return entryList;
}

QValueList<Tag> CodeInformationRepository::getBaseClassList( const QString& className )
{
    kdDebug(9020) << "CodeInformationRepository::getBaseClasseList()" << endl;

    QValueList<Catalog::QueryArgument> args;
    args << Catalog::QueryArgument( "kind", Tag::Kind_Base_class )
    	<< Catalog::QueryArgument( "name", className );
    return query( args );
}














