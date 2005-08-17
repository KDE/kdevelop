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
//Added by qt3to4:
#include <QList>
#include <QByteArray>

#include <krandomsequence.h>
#include <kdebug.h>


#include <cstring>
#include <cstdlib>
#include <db.h>

#include <config.h>

struct _Catalog_Private
{
    QString dbName;

    DB* dbp;
    QMap<QByteArray, DB*> indexList;
    KRandomSequence rnd;
    bool enabled;

    _Catalog_Private()
	: dbp( 0 ), enabled( true )
    {
    }

    bool hasIndex( const QByteArray& name ) const
    {
        return indexList.contains( name );
    }

    DB* index( const QByteArray& name )
    {
        return indexList[ name ];
    }

    bool addItem( DB* dbp, const QByteArray& id, const Tag& tag )
    {
	Q_ASSERT( dbp != 0 );

	DBT key, data;
	int ret;

	std::memset( &key, 0, sizeof(key) );
	std::memset( &data, 0, sizeof(data) );

	QByteArray a1;
	{
	    QDataStream stream( &a1, QIODevice::WriteOnly );
	    stream << id;
	    key.data = a1.data();
	    key.size = a1.size();
	}

	QByteArray a2;
	{
	    QDataStream stream( &a2, QIODevice::WriteOnly );
	    tag.store( stream );
	    data.data = a2.data();
	    data.size = a2.size();
	}

	ret = dbp->put( dbp, 0, &key, &data, 0 );

	return ret == 0;
    }

    bool addItem( DB* dbp, const QVariant& id, const QByteArray& v )
    {
	Q_ASSERT( dbp != 0 );

	DBT key, data;
	int ret;

	std::memset( &key, 0, sizeof(key) );
	std::memset( &data, 0, sizeof(data) );

	QByteArray a1;
	{
	    QDataStream stream( &a1, QIODevice::WriteOnly );
	    stream << id;
	    key.data = a1.data();
	    key.size = a1.size();
	}

	QByteArray a2;
	{
	    QDataStream stream( &a2, QIODevice::WriteOnly );
	    stream << v;
	    data.data = a2.data();
	    data.size = a2.size();
	}

	ret = dbp->put( dbp, 0, &key, &data, 0 );

	return ret == 0;
    }

};


/*!
    \fn  Catalog::Catalog
 */
 Catalog::Catalog()
    : d( new _Catalog_Private() )
{
}

/*!
    \fn  Catalog::~Catalog
 */
 Catalog::~Catalog()
{
    close();
    delete( d );
    d = 0;
}

/*!
    \fn  Catalog::indexList() const
 */
 QList<QByteArray>  Catalog::indexList() const
{
    QList<QByteArray> l;
    QMap<QByteArray, DB*>::Iterator it = d->indexList.begin();
    while( it != d->indexList.end() ){
        l << it.key();
        ++it;
    }

    return l;
}

bool  Catalog::enabled() const
{
    return d->enabled;
}

void  Catalog::setEnabled( bool isEnabled )
{
   d->enabled = isEnabled;
}

/*!
    \fn  Catalog::addIndex( const QString& name )
    @todo document these functions
 */
 void  Catalog::addIndex( const QByteArray& name )
{
    Q_ASSERT( d->dbp != 0 );

    QMap<QByteArray, DB*>::Iterator it = d->indexList.find( name );
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

	if( (ret = dbp->set_cachesize( dbp, 0, 2 * 1024 * 1024, 0 )) != 0 ){
	    kdDebug() << "set_cachesize: " << db_strerror(ret) << endl;
	}

        if ((ret = dbp->open(
	    dbp, QFile::encodeName( indexName ).data(), 0, DB_BTREE, DB_CREATE, 0664)) != 0) {
	    kdDebug() << "db_open: " << db_strerror(ret) << endl;
	    dbp->close( dbp, 0 );
	    return;
        }

        d->indexList[ name ] = dbp;
    }
}

/*!
    \fn  Catalog::close()
 */
 
 void  Catalog::close()
{
    d->dbName = QString::null;

    QMap<QByteArray, DB*>::Iterator it = d->indexList.begin();
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
    \fn  Catalog::open( const QString& dbName )
 */
 
 void  Catalog::open( const QString& dbName )
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

    if( (ret = d->dbp->set_cachesize( d->dbp, 0, 2 * 1024 * 1024, 0 )) != 0 ){
	kdDebug() << "set_cachesize: " << db_strerror(ret) << endl;
    }

    if ((ret = d->dbp->open(
	d->dbp, d->dbName.local8Bit(), 0, DB_BTREE, DB_CREATE, 0664)) != 0) {
	kdDebug() << "db_open: " << db_strerror(ret) << endl;
	close();
	return;
    }
}

/*!
    \fn  Catalog::dbName() const
 */
 
 QString  Catalog::dbName() const
{
    return d->dbName;
}

/*!
    \fn  Catalog::isValid() const
 */
 
 bool  Catalog::isValid() const
{
    return d->dbp != 0;
}

/*!
    \fn  Catalog::addItem( Tag& tag )
 */
 
 void  Catalog::addItem( Tag& tag )
{
    if( tag.name().isEmpty() )
        return;

    QByteArray id = generateId();

    tag.setId( id );
    if( d->addItem(d->dbp, id, tag) ){
	QMap<QByteArray, DB*>::Iterator it = d->indexList.begin();
	while( it != d->indexList.end() ){
	    if( tag.hasAttribute(it.key()) )
	        d->addItem( it.data(), tag.attribute(it.key()), id );
	    ++it;
	}
    }
}

/*!
    \fn  Catalog::getItemById( const QString& id )
 */
 
 Tag  Catalog::getItemById( const QByteArray& id )
{
    Q_ASSERT( d->dbp != 0 );

    DBT key, data;
    std::memset( &key, 0, sizeof(key) );
    std::memset( &data, 0, sizeof(data) );

    QByteArray a1;
    {
	QDataStream stream( &a1, QIODevice::WriteOnly );
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
	QDataStream stream( &a, QIODevice::ReadOnly );
	tag.load( stream );
	a.resetRawData( (const char*) data.data, data.size );
    }

    return tag;
}

/*!
    \fn  Catalog::sync()
*/
 
 void  Catalog::sync()
{
    Q_ASSERT( d->dbp != 0 );
    d->dbp->sync( d->dbp, 0 );

    QMap<QByteArray, DB*>::Iterator it = d->indexList.begin();
    while( it != d->indexList.end() ){
 	it.data()->sync( it.data(), 0 );
        ++it;
    }
}

/*!
    \fn  Catalog::query( const QValueList<QueryArgument>& args )
*/
 
 QList<Tag>  Catalog::query( const QList<QueryArgument>& args )
{
    QList<Tag> tags;

    DBT key, data;

    DBC** cursors = new DBC* [ args.size() + 1 ];

    QList< QPair<QByteArray,QVariant> >::ConstIterator it = args.begin();
    int current = 0;
    while( it != args.end() ){
        QByteArray indexName = (*it).first;
	QVariant value = (*it).second;

        if( d->hasIndex(indexName) ){
            DB* dbp = d->index( indexName );
	    Q_ASSERT( dbp != 0 );

            std::memset( &key, 0, sizeof(key) );
            std::memset( &data, 0, sizeof(data) );

	    QByteArray a1;
	    {
		QDataStream stream( &a1, QIODevice::WriteOnly );
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
	    QDataStream s( &a2, QIODevice::ReadOnly );
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

 QByteArray Catalog::generateId()
{
    static int n = 1;
    static char buffer[1024];
    qsnprintf(buffer, 1024, "%05d", n++ );
    return buffer;
}

