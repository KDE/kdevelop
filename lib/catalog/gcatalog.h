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

#ifndef GCATALOG_H
#define GCATALOG_H

#include <qvaluelist.h>
#include <qpair.h>
#include <qvariant.h>

#include <krandomsequence.h>
#include <kdebug.h>

#include <cstring>
#include <cstdlib>
#include <db.h>

template<class Tp>
struct _GCatalog_Private
{
    QString dbName;

    DB* dbp;
    QMap<QCString, DB*> indexList;
    KRandomSequence rnd;

    _GCatalog_Private()
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

    QValueList<Tp> getAllItems( DB* dbp )
    {
	Q_ASSERT( dbp != 0 );

	DBC* cursor;

	int ret = dbp->cursor( dbp, 0, &cursor, 0 );
	Q_ASSERT( cursor != 0 );

	DBT key, data;
	std::memset( &key, 0, sizeof(key) );
	std::memset( &data, 0, sizeof(data) );

	QValueList<Tp> tags;
	QByteArray a2;
	while( (ret = cursor->c_get(cursor, &key, &data, DB_NEXT)) == 0 ){

	    a2.setRawData( (const char*) data.data, data.size );
	    QDataStream s( a2, IO_ReadOnly );

	    Tp tag;
	    tag.load( s );

	    a2.resetRawData( (const char*) data.data, data.size );

	    tags << tag;
	}

	cursor->c_close( cursor );

	return tags;
    }


    bool addItem( DB* dbp, const QCString& id, const Tp& tag )
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

template <class Tp>
class GCatalog
{
public:
    typedef Tp Tag;
    typedef QPair<QCString, QVariant> QueryArgument;

public:
    GCatalog();
    virtual ~GCatalog();

    bool isValid() const;
    QString dbName() const;

    virtual void open( const QString& dbName );
    virtual void close();
    virtual void sync();

    QValueList<QCString> indexList() const;
    bool hasIndex( const QCString& name ) const;
    void addIndex( const QCString& name );
    void removeIndex( const QCString& name );

    void addItem( Tag& tag );
    bool removeItem( const Tag& id );
    bool removeItemById( const QCString& id );
    void removeItems( const QValueList<QueryArgument>& args );

    Tag getItemById( const QCString& id );
    bool hasItem( const QCString& id );
    QValueList<Tag> getAllItems();
    QValueList<Tag> query( const QValueList<QueryArgument>& args );

    QCString generateId();

private:
    _GCatalog_Private<Tp>* d;

private:
    GCatalog( const GCatalog& source );
    void operator = ( const GCatalog& source );
};

#include "gcatalog.tcc"

#endif
