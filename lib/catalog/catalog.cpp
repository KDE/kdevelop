/* This file is part of KDevelop
    Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "catalog.h"

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdatastream.h>

#include <krandomsequence.h>
#include <kdebug.h>

#include <cstring>
#include <cstdlib>
#include <db.h>

#include <config.h>

struct Catalog::Private
{
    QString dbName;

    DB* dbp;
    QMap<QCString, DB*> indexList;
    KRandomSequence rnd;
    
    Private()
	: dbp( 0 )
    {
    }
    
    bool hasIndex( const QCString& name ) const
    {
        return indexList.contains( name );
    }

    DB* index( const QCString& name )
    {
        return indexList[ name ];
    }

    QValueList<Tag> getAllItems( DB* dbp )
    {
	Q_ASSERT( dbp != 0 );

	DBC* cursor;

	int ret = dbp->cursor( dbp, 0, &cursor, 0 );
	Q_ASSERT( cursor != 0 );

	DBT key, data;
	std::memset( &key, 0, sizeof(key) );
	std::memset( &data, 0, sizeof(data) );

	QValueList<Tag> tags;
	QByteArray a2;
	while( (ret = cursor->c_get(cursor, &key, &data, DB_NEXT)) == 0 ){

	    a2.setRawData( (const char*) data.data, data.size );
	    QDataStream s( a2, IO_ReadOnly );

	    Tag tag;
	    tag.load( s );

	    a2.resetRawData( (const char*) data.data, data.size );

	    tags << tag;
	}

	cursor->c_close( cursor );

	return tags;
    }


    bool addItem( DB* dbp, const QCString& id, const Tag& tag )
    {
	Q_ASSERT( dbp != 0 );

	DBT key, data;
	int ret;

	std::memset( &key, 0, sizeof(key) );
	std::memset( &data, 0, sizeof(data) );

	QByteArray a1;
	{
	    QDataStream stream( a1, IO_WriteOnly );
	    stream << id;
	    key.data = a1.data();
	    key.size = a1.size();
	}

	QByteArray a2;
	{
	    QDataStream stream( a2, IO_WriteOnly );
	    tag.store( stream );
	    data.data = a2.data();
	    data.size = a2.size();
	}

	ret = dbp->put( dbp, 0, &key, &data, 0 );

	return ret == 0;
    }

    bool addItem( DB* dbp, const QVariant& id, const QCString& v )
    {
	Q_ASSERT( dbp != 0 );

	DBT key, data;
	int ret;

	std::memset( &key, 0, sizeof(key) );
	std::memset( &data, 0, sizeof(data) );

	QByteArray a1;
	{
	    QDataStream stream( a1, IO_WriteOnly );
	    stream << id;
	    key.data = a1.data();
	    key.size = a1.size();
	}

	QByteArray a2;
	{
	    QDataStream stream( a2, IO_WriteOnly );
	    stream << v;
	    data.data = a2.data();
	    data.size = a2.size();
	}

	ret = dbp->put( dbp, 0, &key, &data, 0 );

	return ret == 0;
    }

    bool removeItem( DB* dbp, const QCString& id )
    {
	Q_ASSERT( dbp != 0 );

	DBT key, data;
	std::memset( &key, 0, sizeof(key) );
	std::memset( &data, 0, sizeof(data) );

	QByteArray a1;
	{
	    QDataStream stream( a1, IO_WriteOnly );
	    stream << id;
	    key.data = a1.data();
	    key.size = a1.size();
	}

	int ret = 0;	
	ret = dbp->del( dbp, 0, &key, 0 );
	Q_ASSERT( ret == 0 );
		
	return ret == 0;
    }
    
    bool removeItem( DB* dbp, const QVariant& id, const QCString& v )
    {
        Q_ASSERT( dbp != 0 );

        DBT key, data;
        int ret;

        std::memset( &key, 0, sizeof(key) );
        std::memset( &data, 0, sizeof(data) );

        QByteArray a1;
        {
            QDataStream stream( a1, IO_WriteOnly );
            stream << id;
            key.data = a1.data();
            key.size = a1.size();
        }

        QByteArray a2;
        {
            QDataStream stream( a2, IO_WriteOnly );
            stream << v;
            data.data = a2.data();
            data.size = a2.size();
        }

        DBC* dbc = 0;
 
	ret = dbp->cursor( dbp, 0, &dbc, 0 );
	Q_ASSERT( ret == 0 );
	if( ret != 0 )
	    kdDebug() << "dbp->cursor: " << db_strerror(ret) << endl;
     
	ret = dbc->c_get( dbc, &key, &data, DB_GET_BOTH );
	Q_ASSERT( ret == 0 );
	if( ret != 0 )
	    kdDebug() << "dbc->c_get: " << db_strerror(ret) << endl;
 
	ret = dbc->c_del( dbc, 0 );
	Q_ASSERT( ret == 0 );
 
	dbc->c_close( dbc );

        return ret == 0;
    } 
};

/*!
    \fn Catalog::Catalog
 */
Catalog::Catalog()
    : d( new Private )
{
}

/*!
    \fn Catalog::~Catalog
 */
Catalog::~Catalog()
{
    close();
    delete( d );
    d = 0;
}

/*!
    \fn Catalog::indexList() const
 */
QValueList<QCString> Catalog::indexList() const
{
    QValueList<QCString> l;
    QMap<QCString, DB*>::Iterator it = d->indexList.begin();
    while( it != d->indexList.end() ){
        l << it.key();
        ++it;
    }

    return l;
}

/*!
    \fn Catalog::hasIndex( const QString& name ) const
 */
bool Catalog::hasIndex( const QCString& name ) const
{
    return d->indexList.contains( name );
}

/*!
    \fn Catalog::addIndex( const QString& name )
 */
void Catalog::addIndex( const QCString& name )
{
    Q_ASSERT( d->dbp != 0 );

    QMap<QCString, DB*>::Iterator it = d->indexList.find( name );
    if( it == d->indexList.end() ){
        DB* dbp = 0;

        int ret;

        if ((ret = db_create(&dbp, 0, 0)) != 0) {
	    kdDebug() << "db_create: " << db_strerror(ret) << endl;
	    return /*false*/;
        }

        if ((ret = dbp->set_flags(dbp, DB_DUP | DB_DUPSORT)) != 0) {
	    dbp->err(dbp, ret, "set_flags: DB_DUP | DB_DUPSORT");
	    dbp->close( dbp, 0 );
	    return;
        }

	QFileInfo fileInfo( d->dbName );
	QString indexName = fileInfo.dirPath(true) + "/" + fileInfo.baseName() + "." + QString(name) + ".idx";

        if ((ret = dbp->open(
	    dbp BDB_OPEN_HACK, QFile::encodeName( indexName ).data(), 0, DB_BTREE, DB_CREATE, 0664)) != 0) {
	    kdDebug() << "db_open: " << db_strerror(ret) << endl;
	    dbp->close( dbp, 0 );
	    return;
        }

        d->indexList[ name ] = dbp;
    }
}

/*!
    \fn Catalog::removeIndex( const QString& name )
 */
void Catalog::removeIndex( const QCString& name )
{
    QMap<QCString, DB*>::Iterator it = d->indexList.find( name );
    if( it != d->indexList.end() ){
	DB* dbp = *it;
        d->indexList.remove( it );
	dbp->close( dbp, 0 );
    }
}

/*!
    \fn Catalog::close()
 */
void Catalog::close()
{
    d->dbName = QString::null;

    QMap<QCString, DB*>::Iterator it = d->indexList.begin();
    while( it != d->indexList.end() ){
        if( it.data() ){
	    it.data()->close( it.data(), 0 );
        }
        ++it;
    }
    d->indexList.clear();

    if( d->dbp != 0 ){
	d->dbp->close( d->dbp, 0 );
	d->dbp = 0;
    }
}

/*!
    \fn Catalog::open( const QString& dbName )
 */
void Catalog::open( const QString& dbName )
{
    Q_ASSERT( d->dbp == 0 );

    d->dbName = dbName;

    int ret;

    if ((ret = db_create(&d->dbp, 0, 0)) != 0) {
	kdDebug() << "db_create: " << db_strerror(ret) << endl;
	return /*false*/;
    }

    if ((ret = d->dbp->set_flags(d->dbp, DB_RECNUM)) != 0) {
	d->dbp->err(d->dbp, ret, "set_flags: DB_RECNUM");
	close();
	return;
    }

    if ((ret = d->dbp->open(
	d->dbp  BDB_OPEN_HACK, d->dbName.local8Bit(), 0, DB_BTREE, DB_CREATE, 0664)) != 0) {
	kdDebug() << "db_open: " << db_strerror(ret) << endl;
	close();
	return;
    }
}

/*!
    \fn Catalog::dbName() const
 */
QString Catalog::dbName() const
{
    return d->dbName;
}

/*!
    \fn Catalog::isValid() const
 */
bool Catalog::isValid() const
{
    return d->dbp != 0;
}

/*!
    \fn Catalog::addItem( Tag& tag )
 */
void Catalog::addItem( Tag& tag )
{
    // TODO: generate a unique ID
    
    QCString id = generateId();
    
    tag.setId( id );
    if( d->addItem(d->dbp, id, tag) ){
	QMap<QCString, DB*>::Iterator it = d->indexList.begin();
	while( it != d->indexList.end() ){
	    if( tag.hasAttribute(it.key()) )
	        d->addItem( it.data(), tag.attribute(it.key()), id );
	    ++it;
	}
    }
}

/*!
    \fn Catalog::removeItemById( const QCString& id )
 */
bool Catalog::removeItemById( const QCString& id )
{
    Tag tag = getItemById( id );
    
    QMap<QCString, DB*>::Iterator it = d->indexList.begin();
    while( it != d->indexList.end() ){
	if( tag.hasAttribute(it.key()) )
	    d->removeItem( it.data(), tag.attribute(it.key()), id );
	++it;
    }

    return d->removeItem( d->dbp, id );
}

/*!
    \fn Catalog::removeItem( const Tag& tag )
 */
bool Catalog::removeItem( const Tag& tag )
{
   return removeItemById( tag.id() );
}

/*!
    \fn Catalog::getItemById( const QString& id )
 */
Tag Catalog::getItemById( const QCString& id )
{
    Q_ASSERT( d->dbp != 0 );

    DBT key, data;
    std::memset( &key, 0, sizeof(key) );
    std::memset( &data, 0, sizeof(data) );

    QByteArray a1;
    {
	QDataStream stream( a1, IO_WriteOnly );
	stream << id;
	key.data = a1.data();
	key.size = a1.size();
    }

    int ret = d->dbp->get( d->dbp, 0, &key, &data, 0 );
    Q_ASSERT( ret == 0 );

    Tag tag;

    if( ret == 0 ){
	QByteArray a;
	a.setRawData( (const char*) data.data, data.size );
	QDataStream stream( a, IO_ReadOnly );
	tag.load( stream );
	a.resetRawData( (const char*) data.data, data.size );
    }

    return tag;
}

/*!
    \fn Catalog::getAllItems()
 */
QValueList<Tag> Catalog::getAllItems()
{
    return d->getAllItems( d->dbp );
}

/*!
    \fn Catalog::sync()
*/
void Catalog::sync()
{
    Q_ASSERT( d->dbp != 0 );
    d->dbp->sync( d->dbp, 0 );

    QMap<QCString, DB*>::Iterator it = d->indexList.begin();
    while( it != d->indexList.end() ){
 	it.data()->sync( it.data(), 0 );
        ++it;
    }
}

/*!
    \fn Catalog::query( const QValueList<QueryArgument>& args )
*/
QValueList<Tag> Catalog::query( const QValueList<QueryArgument>& args )
{
    QValueList<Tag> tags;

    DBT key, data;

    DBC** cursors = new DBC* [ args.size() + 1 ];

    QValueList< QPair<QCString,QVariant> >::ConstIterator it = args.begin();
    int current = 0;
    while( it != args.end() ){
        QCString indexName = (*it).first;
	QVariant value = (*it).second;

        if( d->hasIndex(indexName) ){
            DB* dbp = d->index( indexName );
	    Q_ASSERT( dbp != 0 );

            std::memset( &key, 0, sizeof(key) );
            std::memset( &data, 0, sizeof(data) );

	    QByteArray a1;
	    {
		QDataStream stream( a1, IO_WriteOnly );
		stream << value;
		key.data = a1.data();
		key.size = a1.size();
	    }

	    DBC* cursor = 0;
	    int rtn = dbp->cursor( dbp, 0, &cursor, 0 );
	    Q_ASSERT( rtn == 0 );

	    rtn = cursor->c_get( cursor, &key, &data, DB_SET );
	    if( rtn == DB_NOTFOUND ){
	        // kdDebug() << "!!! not found !!!" << endl;
		rtn = 0;
	    }
	    Q_ASSERT( rtn == 0 );

	    cursors[ current++ ] = cursor;
        }
        ++it;
    }
    cursors[ current ] = 0;

    DBC* join_curs = 0;
    int rtn = d->dbp->join( d->dbp, cursors, &join_curs, 0 );
    Q_ASSERT( rtn == 0 );

    std::memset( &key, 0, sizeof(key) );
    std::memset( &data, 0, sizeof(data) );

    while( join_curs->c_get(join_curs, &key, &data, 0) == 0 ) {

        QByteArray a2;
	{
	    a2.setRawData( (const char*) data.data, data.size );
	    QDataStream s( a2, IO_ReadOnly );
	    Tag tag;
	    tag.load( s );
	    a2.resetRawData( (const char*) data.data, data.size );
	    tags << tag;
	}
    }

    join_curs->c_close( join_curs );
    DBC** c = cursors;
    while( *c != 0 ){
        (*c)->c_close( *c );
        ++c;
    }
    delete[] cursors;

    return tags;
}

void Catalog::removeItems( const QValueList< QueryArgument > & args )
{
    DBT key, data;

    DBC** cursors = new DBC* [ args.size() + 1 ];

    QValueList< QPair<QCString,QVariant> >::ConstIterator it = args.begin();
    int current = 0;
    while( it != args.end() ){
        QCString indexName = (*it).first;
	QVariant value = (*it).second;

        if( d->hasIndex(indexName) ){
            DB* dbp = d->index( indexName );
	    Q_ASSERT( dbp != 0 );

            std::memset( &key, 0, sizeof(key) );
            std::memset( &data, 0, sizeof(data) );

	    QByteArray a1;
	    {
		QDataStream stream( a1, IO_WriteOnly );
		stream << value;
		key.data = a1.data();
		key.size = a1.size();
	    }

	    DBC* cursor = 0;
	    int rtn = dbp->cursor( dbp, 0, &cursor, 0 );
	    Q_ASSERT( rtn == 0 );

	    rtn = cursor->c_get( cursor, &key, &data, DB_SET );
	    if( rtn == DB_NOTFOUND ){
	        // kdDebug() << "!!! not found !!!" << endl;
		rtn = 0;
	    }
	    Q_ASSERT( rtn == 0 );

	    cursors[ current++ ] = cursor;
        }
        ++it;
    }
    cursors[ current ] = 0;


    DBC* join_curs = 0;
    int rtn = d->dbp->join( d->dbp, cursors, &join_curs, 0 );
    Q_ASSERT( rtn == 0 );

    std::memset( &key, 0, sizeof(key) );
    std::memset( &data, 0, sizeof(data) );

    QValueList<QCString> ids;
    while( join_curs->c_get(join_curs, &key, &data, 0) == 0 ) {

        QByteArray a1;
	{
	    a1.setRawData( (const char*) key.data, key.size );
	    QDataStream s( a1, IO_ReadOnly );
	    QCString id;
	    s >> id;
	    ids.append( id );
	    a1.resetRawData( (const char*) key.data, key.size );
	}
    }

    join_curs->c_close( join_curs );
    DBC** c = cursors;
    while( *c != 0 ){
        (*c)->c_close( *c );
        ++c;
    }
    delete[] cursors;

    QValueList<QCString>::Iterator dit = ids.begin();
    while( dit != ids.end() ){
        removeItemById( *dit );
	++dit;
    }
}

QCString Catalog::generateId()
{
    QCString asStr;
    int n = 0;
    for( ;; ){
	++n;
	
	unsigned long l = d->rnd.getLong( 999999 );
	asStr.setNum( l );
	
	if( !hasItem(asStr) ){
	    return asStr;
	}
    } 
}

bool Catalog::hasItem( const QCString & id )
{
    Q_ASSERT( d->dbp != 0 );

    DBT key, data;
    std::memset( &key, 0, sizeof(key) );
    std::memset( &data, 0, sizeof(data) );

    QByteArray a1;
    {
	QDataStream stream( a1, IO_WriteOnly );
	stream << id;
	key.data = a1.data();
	key.size = a1.size();
    }

    int ret = d->dbp->get( d->dbp, 0, &key, &data, 0 );
    return ret == 0;
}

