/* This file is part of KDevelop
   Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2006 Matt Rogers <mattr@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kdevbdbcatalogbackend.h"

#include <QFileInfo>
#include <kdebug.h>

#include <cstring>


KDevBDBCatalogBackend::KDevBDBCatalogBackend()
{
    m_db = 0;
}

KDevBDBCatalogBackend::~KDevBDBCatalogBackend()
{

}

void KDevBDBCatalogBackend::open( const QString& dbName )
{
    Q_ASSERT( m_db == 0 );
    int ret;

    ret = db_create( &m_db, 0, 0 );
    if ( ret != 0 )
    {
        kDebug(9000) << k_funcinfo << "db_create: " << db_strerror( ret ) << endl;
        return;
    }

    ret = m_db->set_flags( m_db, DB_RECNUM );
    if ( ret != 0 )
    {
        m_db->err(m_db, ret, "set_flags: DB_RECNUM");
        close();
        return;
    }

    ret = m_db->set_cachesize( m_db, 0, 2 * 1024 * 1024, 0 );
    if ( ret != 0 )
        kDebug(9000) << k_funcinfo << "set_cachesize: " << db_strerror( ret ) << endl;

    ret = m_db->open( m_db, 0, dbName.toLocal8Bit(), 0, DB_BTREE,
                      DB_CREATE, 0664 );
    if ( ret != 0 )
    {
        kDebug(9000) << k_funcinfo << "db_open: " << db_strerror( ret ) << endl;
        close();
        return;
    }
}

void KDevBDBCatalogBackend::close()
{
    QMap<QByteArray, DB*>::Iterator it = m_indexList.begin();
    while ( it != m_indexList.end() )
    {
        if( it.value() )
            it.value()->close( it.value(), 0 );
        ++it;
    }

    m_indexList.clear();

    if ( m_db != 0 )
    {
        m_db->close( m_db, 0 );
        m_db = 0;
    }
}

void KDevBDBCatalogBackend::sync()
{
    Q_ASSERT( m_db != 0 );
    m_db->sync( m_db, 0 );

    QMap<QByteArray, DB*>::Iterator it = m_indexList.begin();
    while( it != m_indexList.end() )
    {
        it.value()->sync( it.value(), 0 );
        ++it;
    }

}

bool KDevBDBCatalogBackend::isOpen()
{
    return ( m_db != 0 );
}

void KDevBDBCatalogBackend::addItem( Tag& tag )
{
    if( tag.name().isEmpty() )
        return;

    QByteArray id = generateId();

    if( addItem( m_db, id, tag ) )
    {
        QMap<QByteArray, DB*>::Iterator it = m_indexList.begin();
        while( it != m_indexList.end() )
        {
            if ( tag.hasAttribute( it.key() ) )
                addItem( it.value(), tag.attribute( it.key() ), id );
            ++it;
        }
    }
}

Tag KDevBDBCatalogBackend::getItemById( int id )
{
    Q_ASSERT( m_db != 0 );

    DBT key, data;
    std::memset( &key, 0, sizeof( key ) );
    std::memset( &data, 0, sizeof( data ) );

    QByteArray a1;
    {
        QDataStream stream( &a1, QIODevice::WriteOnly );
        stream << id;
        key.data = a1.data();
        key.size = a1.size();
    }

    int ret = m_db->get( m_db, 0, &key, &data, 0 );
    Q_ASSERT( ret == 0 );

    Tag tag;

    if( ret == 0 )
    {
        QByteArray a = QByteArray::fromRawData( (const char*) data.data, data.size );
        QDataStream stream( &a, QIODevice::ReadOnly );
        tag.load( stream );
        a.clear();
    }

    return tag;
}

QList<Tag> KDevBDBCatalogBackend::query( const QList<QueryArgument>& args )
{
    QList<Tag> tags;

    DBT key, data;

    DBC** cursors = new DBC*[args.size() + 1];

    QList< QPair<QByteArray,QVariant> >::ConstIterator it = args.begin();
    int current = 0;
    while( it != args.end() )
    {
        QByteArray indexName = (*it).first;
        QVariant value = (*it).second;

        if( hasIndex(indexName) )
        {
            DB* dbp = index( indexName );
            Q_ASSERT( dbp != 0 );

            std::memset( &key, 0, sizeof( key ) );
            std::memset( &data, 0, sizeof( data ) );

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
            if( rtn == DB_NOTFOUND )
                rtn = 0;

            Q_ASSERT( rtn == 0 );

            cursors[current++] = cursor;
        }
        ++it;
    }

    cursors[ current ] = 0;

    DBC* join_curs = 0;
    int rtn = m_db->join( m_db, cursors, &join_curs, 0 );
    Q_ASSERT( rtn == 0 );

    std::memset( &key, 0, sizeof( key ) );
    std::memset( &data, 0, sizeof( data ) );

    while( join_curs->c_get(join_curs, &key, &data, 0) == 0 )
    {
        QByteArray a2 = QByteArray::fromRawData( (const char*) data.data, data.size );
        QDataStream s( &a2, QIODevice::ReadOnly );
        Tag tag;
        tag.load( s );
        a2.clear();
        tags << tag;
    }

    join_curs->c_close( join_curs );
    DBC** c = cursors;
    while( *c != 0 )
    {
        (*c)->c_close( *c );
        ++c;
    }

    delete[] cursors;

    return tags;

}

QList<QByteArray> KDevBDBCatalogBackend::indexList() const
{
    QList<QByteArray> l;
    QMap<QByteArray, DB*>::ConstIterator it = m_indexList.begin();
    while( it != m_indexList.end() )
    {
        l << it.key();
        ++it;
    }

    return l;
}

void KDevBDBCatalogBackend::addIndex( const QByteArray& name )
{
    Q_ASSERT( m_db != 0 );

    QMap<QByteArray, DB*>::Iterator it = m_indexList.find( name );
    if( it == m_indexList.end() )
    {
        DB* dbp = 0;
        const char *fname;

        int ret;

        ret = m_db->get_dbname(m_db, &fname, NULL);
        if ( ret != 0 )
        {
            kDebug() << "get_dbname: " << db_strerror(ret) << endl;
            dbp->close( dbp, 0 );
            return;
        }

        ret = db_create(&dbp, 0, 0);
        if ( ret != 0 )
        {
            kDebug() << "db_create: " << db_strerror(ret) << endl;
            return;
        }

        ret = dbp->set_flags(dbp, DB_DUP | DB_DUPSORT);
        if ( ret != 0 )
        {
            dbp->err( dbp, ret, "set_flags: DB_DUP | DB_DUPSORT" );
            dbp->close( dbp, 0 );
            return;
        }

        QByteArray fileName( fname );
        QFileInfo fileInfo( fileName );
        QString indexName = QString( "%1/%2.%3.idx" )
                            .arg( fileInfo.absolutePath() )
                            .arg( fileInfo.baseName() )
                            .arg( QString( name ) );

        ret = dbp->set_cachesize( dbp, 0, 2 * 1024 * 1024, 0 );
        if( ret != 0 )
            kDebug() << "set_cachesize: " << db_strerror(ret) << endl;

        ret = dbp->open( dbp, 0, QFile::encodeName( indexName ).data(), 0,
                         DB_BTREE, DB_CREATE, 0664 );
        if ( ret != 0 )
        {
            kDebug() << "db_open: " << db_strerror(ret) << endl;
            dbp->close( dbp, 0 );
            return;
        }

        m_indexList[ name ] = dbp;
    }
}

bool KDevBDBCatalogBackend::hasIndex( const QByteArray& name ) const
{
    return m_indexList.contains( name );
}

DB* KDevBDBCatalogBackend::index( const QByteArray& name )
{
    return m_indexList[ name ];
}

bool KDevBDBCatalogBackend::addItem( DB* dbp, const QByteArray& id, const Tag& tag )
{
    Q_ASSERT( dbp != 0 );

    DBT key, data;
    int ret;

    std::memset( &key, 0, sizeof( key ) );
    std::memset( &data, 0, sizeof( data ) );

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

    return ( ret == 0 );
}

bool KDevBDBCatalogBackend::addItem( DB* dbp, const QVariant& id, const QByteArray& v )
{
    Q_ASSERT( dbp != 0 );

    DBT key, data;
    int ret;

    std::memset( &key, 0, sizeof( key ) );
    std::memset( &data, 0, sizeof( data ) );

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

    return ( ret == 0 );
}

QByteArray KDevBDBCatalogBackend::generateId()
{
    static int n = 1;
    QString asStr;
    asStr.sprintf( "%05d", n++ );
    return asStr.toLatin1();
}

//kate: indent-spaces on; indent-width 4; replace-tabs on;
