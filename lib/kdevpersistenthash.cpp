/* This file is part of KDevelop
Copyright (C) 2006 Adam Treat <treat@kde.org>

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

#include "kdevpersistenthash.h"

#include <iostream>
#include <fstream>

#include <QFile>
#include <QDataStream>

#include "kdevast.h"
#include "kdevcore.h"
#include "kdevlanguagesupport.h"

KDevPersistentHash::KDevPersistentHash( QObject *parent )
        : QObject( parent )
{
//     m_hash.set_deleted_key( NULL );
}

KDevPersistentHash::~KDevPersistentHash()
{}

void KDevPersistentHash::insert( const KUrl &url, KDevAST *ast )
{
    Q_UNUSED( url );
    Q_UNUSED( ast );
//     if ( !url.isValid() )
//         return ;

//     m_hash[ url.url() ] = ast;
}

void KDevPersistentHash::load()
{
//     kDebug() << k_funcinfo << endl;
//     QString fileName = "/home/kde/trunk/KDE/kdevelop/.kdev4/pcstest";
//     FILE *f = fopen( fileName.toLatin1().data(), "r" );
//     if ( f )
//     {
//         m_hash.read_metadata( f );
//     }
//
//     fpos_t pos;
//     if ( fgetpos( f, &pos ) )
//     {
//         kDebug() << "WARNING couldn't skip metadata!! ABORT" << endl;
//         return;
//     }
//
//     QFile file;
//     if ( file.open( f, QIODevice::ReadOnly ) )
//     {
//         file.seek( pos.__pos );
//
//         QDataStream in( &file );
//
//         PHASH::iterator it = m_hash.begin();
//         for ( ; it != m_hash.end(); ++it )
//         {
//             QString str;
//             qint32 ast;
//             in >> str >> ast;
//             new (const_cast<QString*>(&it->first)) QString(str);
//             new (&it->second) int(ast);
//         }
//
//         file.close();
//     }
//
//     PHASH::iterator it = m_hash.begin();
//     for ( ; it != m_hash.end(); ++it )
//     {
//         kDebug() << "key = " << it->first << " value = " << it->second << endl;
//     }
}

void KDevPersistentHash::save()
{
//     kDebug() << k_funcinfo << endl;
//     QString fileName = "/home/kde/trunk/KDE/kdevelop/.kdev4/pcstest";
//     FILE *f = fopen( fileName.toLatin1().data(), "w" );
//     if ( f )
//     {
//         m_hash.write_metadata( f );
//     }
//     fclose(f);
//
//     std::ofstream out( fileName.toLatin1().data(), std::ios::binary | std::ios::app );
//     if (out.is_open())
//     {
//         PHASH::iterator it = m_hash.begin();
//         for ( ; it != m_hash.end(); ++it )
//         {
//             char *s = it->first.toLatin1().data();
//             out.write( reinterpret_cast<char*>(s), sizeof(s) );
//             KDevCore::activeLanguage()->write( it->second, out );
//         }
//
//         out.flush();
//         out.close();
//     }
}

#include "kdevpersistenthash.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
