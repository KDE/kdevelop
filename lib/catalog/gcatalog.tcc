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

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdatastream.h>

#include <config.h>

/*!
    \fn GCatalog<Tp>::GCatalog
 */
template <class Tp>
inline GCatalog<Tp>::GCatalog()
    : d( new _GCatalog_Private<Tp>() )
{
}

/*!
    \fn GCatalog<Tp>::~GCatalog
 */
template <class Tp>
inline GCatalog<Tp>::~GCatalog()
{
    close();
    delete( d );
    d = 0;
}

/*!
    \fn GCatalog<Tp>::indexList() const
 */
template <class Tp>
inline QValueList<QCString> GCatalog<Tp>::indexList() const
{
    QValueList<QCString> l;
    QMap<QCString, DB*>::Iterator it = d->indexList.begin();
    while( it != d->indexList.end() ){
        l << it.key();
        ++it;
    }

    return l;
}

template <class Tp>
bool GCatalog<Tp>::enabled() const
{
    return d->enabled;
}

template <class Tp>
void GCatalog<Tp>::setEnabled( bool enabled )
{
   d->enabled = enabled;
}


/*!
    \fn GCatalog<Tp>::hasIndex( const QString& name ) const
 */
template <class Tp>
inline bool GCatalog<Tp>::hasIndex( const QCString& name ) const
{
    return d->indexList.contains( name );
}

/*!
    \fn GCatalog<Tp>::addIndex( const QString& name )
 */
template <class Tp>
inline void GCatalog<Tp>::addIndex( const QCString& name )
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
    \fn GCatalog<Tp>::removeIndex( const QString& name )
 */
template <class Tp>
inline void GCatalog<Tp>::removeIndex( const QCString& name )
{
    QMap<QCString, DB*>::Iterator it = d->indexList.find( name );
    if( it != d->indexList.end() ){
	DB* dbp = *it;
        d->indexList.remove( it );
	dbp->close( dbp, 0 );
    }
}

/*!
    \fn GCatalog<Tp>::close()
 */
template <class Tp>
inline void GCatalog<Tp>::close()
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
    \fn GCatalog<Tp>::open( const QString& dbName )
 */
template <class Tp>
inline void GCatalog<Tp>::open( const QString& dbName )
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
    \fn GCatalog<Tp>::dbName() const
 */
template <class Tp>
inline QString GCatalog<Tp>::dbName() const
{
    return d->dbName;
}

/*!
    \fn GCatalog<Tp>::isValid() const
 */
template <class Tp>
inline bool GCatalog<Tp>::isValid() const
{
    return d->dbp != 0;
}

/*!
    \fn GCatalog<Tp>::addItem( Tp& tag )
 */
template <class Tp>
inline void GCatalog<Tp>::addItem( Tp& tag )
{
    if( tag.name().isEmpty() )
        return;

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
    \fn GCatalog<Tp>::removeItemById( const QCString& id )
 */
template <class Tp>
inline bool GCatalog<Tp>::removeItemById( const QCString& id )
{
    Tp tag = getItemById( id );

    QMap<QCString, DB*>::Iterator it = d->indexList.begin();
    while( it != d->indexList.end() ){
	if( tag.hasAttribute(it.key()) )
	    d->removeItem( it.data(), tag.attribute(it.key()), id );
	++it;
    }

    return d->removeItem( d->dbp, id );
}

/*!
    \fn GCatalog<Tp>::removeItem( const Tp& tag )
 */
template <class Tp>
inline bool GCatalog<Tp>::removeItem( const Tp& tag )
{
   return removeItemById( tag.id() );
}

/*!
    \fn GCatalog<Tp>::getItemById( const QString& id )
 */
template <class Tp>
inline Tp GCatalog<Tp>::getItemById( const QCString& id )
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

    Tp tag;

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
    \fn GCatalog<Tp>::getAllItems()
 */
template <class Tp>
inline QValueList<Tp> GCatalog<Tp>::getAllItems()
{
    return d->getAllItems( d->dbp );
}

/*!
    \fn GCatalog<Tp>::sync()
*/
template <class Tp>
inline void GCatalog<Tp>::sync()
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
    \fn GCatalog<Tp>::query( const QValueList<QueryArgument>& args )
*/
template <class Tp>
inline QValueList<Tp> GCatalog<Tp>::query( const QValueList<QueryArgument>& args )
{
    QValueList<Tp> tags;

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
	    Tp tag;
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

template <class Tp>
inline void GCatalog<Tp>::removeItems( const QValueList< QueryArgument > & args )
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

template <class Tp>
inline QCString GCatalog<Tp>::generateId()
{
    static int n = 1;
    QCString asStr;
    asStr.sprintf( "%05d", n++ );
    return asStr;
}

template <class Tp>
inline bool GCatalog<Tp>::hasItem( const QCString & id )
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

