/* This file is part of KDevelop
   Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2006 Matt Rogers <mattr@kde.org>
   Copyright (C) 2006 Jakob Petsovits <jpetso@gmx.at>

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

#include "kdevtdbcatalogbackend.h"

#include <QFileInfo>
#include <kdebug.h>

#include <cstring>
#include <fcntl.h>
#include <errno.h>


KDevTDBCatalogBackend::KDevTDBCatalogBackend()
{
    m_tdb = 0;
}

KDevTDBCatalogBackend::~KDevTDBCatalogBackend()
{

}

void KDevTDBCatalogBackend::open( const QString& dbName )
{
    Q_ASSERT( m_tdb == 0 );

    m_tdb = tdb_open( dbName.toLocal8Bit(), 0 /* hash size, zero is default */,
                      TDB_DEFAULT /* open flags */, O_RDWR | O_CREAT, 0664 );
    if ( !m_tdb )
    {
        kDebug(9000) << k_funcinfo << "tdb_open (" << dbName << "): "
                     << strerror( errno ) << endl;
        return;
    }

    m_dbName = dbName;
}

void KDevTDBCatalogBackend::close()
{
    QMap<QByteArray, TDB_CONTEXT*>::Iterator it = m_indexList.begin();
    while ( it != m_indexList.end() )
    {
        if( it.value() )
            tdb_close( it.value() );
        ++it;
    }

    m_indexList.clear();

    if ( m_tdb != 0 )
    {
        tdb_close( m_tdb );
        m_tdb = 0;
        m_dbName = QString();
    }
}

void KDevTDBCatalogBackend::sync()
{
    // TDB's README says:
    // no tdbm_sync() function. No operations are cached in the library anyway
}

bool KDevTDBCatalogBackend::isOpen()
{
    return ( m_tdb != 0 );
}

void KDevTDBCatalogBackend::addItem( Tag& tag )
{
    if( tag.name().isEmpty() )
        return;

    QByteArray id = generateId();

    if( addItem( m_tdb, id, tag ) )
    {
        QMap<QByteArray, TDB_CONTEXT*>::Iterator it = m_indexList.begin();
        while( it != m_indexList.end() )
        {
            if ( tag.hasAttribute( it.key() ) )
                addItem( it.value(), tag.attribute( it.key() ), id );
            ++it;
        }
    }
}

Tag KDevTDBCatalogBackend::getItemById( int id )
{
    Q_ASSERT( m_tdb != 0 );

    TDB_DATA key, data;
    std::memset( &key, 0, sizeof( key ) );
    std::memset( &data, 0, sizeof( data ) );

    QByteArray a1;
    {
        QDataStream stream( &a1, QIODevice::WriteOnly );
        stream << id;
        key.dptr  = (unsigned char*) a1.data();
        key.dsize = a1.size();
    }

    data = tdb_fetch( m_tdb, key );
    Q_ASSERT( data.dptr != 0 );

    Tag tag;

    if( data.dptr != 0 )
    {
        QByteArray a = QByteArray::fromRawData( (const char*) data.dptr, data.dsize );
        QDataStream stream( &a, QIODevice::ReadOnly );
        tag.load( stream );
        a.clear();
        free( data.dptr );
    }

    return tag;
}

QList<Tag> KDevTDBCatalogBackend::query( const QList<QueryArgument>& args )
{
    QList<Tag> tags;

    TDB_DATA key, data;

    QList<QueryArgument>::ConstIterator it = args.begin();
    while( it != args.end() )
    {
        QByteArray indexName = (*it).first;
        QVariant value = (*it).second;

        if( hasIndex(indexName) )
        {
            TDB_CONTEXT* tdb = index( indexName );
            Q_ASSERT( tdb != 0 );

            std::memset( &key, 0, sizeof( key ) );
            std::memset( &data, 0, sizeof( data ) );

            QByteArray a1;
            {
                QDataStream stream( &a1, QIODevice::WriteOnly );
                stream << value;
                key.dptr  = (unsigned char*) a1.data();
                key.dsize = a1.size();
            }

            data = tdb_fetch( tdb, key );

            if( data.dptr != 0 )
            {
                QByteArray a2 = QByteArray::fromRawData( (const char*) data.dptr, data.dsize );
                QDataStream stream( &a2, QIODevice::ReadOnly );
                Tag tag;

                tag.load( stream );
                a2.clear();
                tags << tag;

                free( data.dptr );
            }
        }
        ++it;
    }

    return tags;
}

QList<QByteArray> KDevTDBCatalogBackend::indexList() const
{
    QList<QByteArray> l;
    QMap<QByteArray, TDB_CONTEXT*>::ConstIterator it = m_indexList.begin();
    while( it != m_indexList.end() )
    {
        l << it.key();
        ++it;
    }

    return l;
}

void KDevTDBCatalogBackend::addIndex( const QByteArray& name )
{
    Q_ASSERT( m_tdb != 0 );

    QMap<QByteArray, TDB_CONTEXT*>::Iterator it = m_indexList.find( name );
    if( it == m_indexList.end() )
    {
        TDB_CONTEXT* tdb = 0;

        QFileInfo fileInfo( m_dbName );
        QString indexName = QString( "%1/%2.%3.idx" )
                .arg( fileInfo.absolutePath() )
                .arg( fileInfo.baseName() )
                .arg( QString( name ) );


        tdb = tdb_open( QFile::encodeName( indexName ).data(),
                        0 /* hash size, zero is default */,
                        TDB_DEFAULT /* open flags */, O_RDWR | O_CREAT, 0664 );
        if ( !tdb )
        {
            kDebug(9000) << k_funcinfo << "tdb_open (" << indexName << "): "
                         << strerror( errno ) << endl;
            return;
        }

        m_indexList[ name ] = tdb;
    }
}

bool KDevTDBCatalogBackend::hasIndex( const QByteArray& name ) const
{
    return m_indexList.contains( name );
}

TDB_CONTEXT* KDevTDBCatalogBackend::index( const QByteArray& name )
{
    return m_indexList[ name ];
}

bool KDevTDBCatalogBackend::addItem( TDB_CONTEXT* tdb, const QByteArray& id, const Tag& tag )
{
    Q_ASSERT( tdb != 0 );

    TDB_DATA key, data;
    int ret;

    std::memset( &key, 0, sizeof( key ) );
    std::memset( &data, 0, sizeof( data ) );

    QByteArray a1;
    {
        QDataStream stream( &a1, QIODevice::WriteOnly );
        stream << id;
        key.dptr  = (unsigned char*) a1.data();
        key.dsize = a1.size();
    }

    QByteArray a2;
    {
        QDataStream stream( &a2, QIODevice::WriteOnly );
        tag.store( stream );
        data.dptr  = (unsigned char*) a2.data();
        data.dsize = a2.size();
    }

    ret = tdb_store( tdb, key, data, TDB_REPLACE );

    return ( ret == 0 );
}

bool KDevTDBCatalogBackend::addItem( TDB_CONTEXT* tdb, const QVariant& id, const QByteArray& v )
{
    Q_ASSERT( tdb != 0 );

    TDB_DATA key, data;
    int ret;

    std::memset( &key, 0, sizeof( key ) );
    std::memset( &data, 0, sizeof( data ) );

    QByteArray a1;
    {
        QDataStream stream( &a1, QIODevice::WriteOnly );
        stream << id;
        key.dptr  = (unsigned char*) a1.data();
        key.dsize = a1.size();
    }

    QByteArray a2;
    {
        QDataStream stream( &a2, QIODevice::WriteOnly );
        stream << v;
        data.dptr  = (unsigned char*) a2.data();
        data.dsize = a2.size();
    }

    ret = tdb_store( tdb, key, data, TDB_REPLACE );

    return ( ret == 0 );
}

QByteArray KDevTDBCatalogBackend::generateId()
{
    static int n = 1;
    QString asStr;
    asStr.sprintf( "%05d", n++ );
    return asStr.toLatin1();
}

//kate: indent-spaces on; indent-width 4; replace-tabs on;
