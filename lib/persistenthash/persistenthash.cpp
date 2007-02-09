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

#include "persistenthash.h"

#include <iostream>
#include <fstream>

#include <QFile>
#include <QDataStream>
#include <QReadLocker>
#include <QWriteLocker>
#include <QtDesigner/QExtensionFactory>

#include "kdevast.h"
#include "icore.h"
#include "kdevlanguagesupport.h"

namespace KDevelop
{

struct PersistentHashPrivate
{
public:
#ifndef NO_GOOGLE_SPARSEHASH
    PHASH m_astHash;
#else
    QHash<KUrl, AST*> m_astHash;
    QMultiHash<QString, AST*> m_filenameAstHash;
#endif

    mutable QReadWriteLock m_mutex;
};

PersistentHash::PersistentHash( KInstance* instance, QObject *parent)
        : IPlugin( instance, parent ), d(new PersistentHashPrivate)
{
#ifndef NO_GOOGLE_SPARSEHASH
    d->m_astHash.set_deleted_key( NULL );
#endif
}

PersistentHash::~PersistentHash()
{
#ifndef NO_GOOGLE_SPARSEHASH
#else
    QHashIterator<KUrl, AST*> it = d->m_astHash;
    while (it.hasNext()) {
        it.next();
        it.value()->release();
    }
#endif
    delete d;
}

void PersistentHash::insertAST( const KUrl &url, AST *ast )
{
    QWriteLocker lock(&d->m_mutex);

#ifndef NO_GOOGLE_SPARSEHASH
    d->m_astHash[ url.url() ] = ast;
#else
    if (d->m_astHash.contains(url)) {
        d->m_astHash[url]->release();
        for (QMultiHash<QString, AST*>::Iterator it = d->m_filenameAstHash.find(url.fileName()); it != d->m_filenameAstHash.end() && it.key() == url.fileName(); ++it)
            if (it.value() == ast) {
                d->m_filenameAstHash.erase(it);
                break;
            }
    }

    d->m_astHash.insert(url, ast);
    d->m_filenameAstHash.insert(url.fileName(), ast);
#endif
}

AST * PersistentHash::retrieveAST(const KUrl & url)
{
    QReadLocker lock(&d->m_mutex);

#ifndef NO_GOOGLE_SPARSEHASH
    return d->m_astHash[ url.url() ];
#else
    if (d->m_astHash.contains(url))
        return d->m_astHash[ url ];
    return 0;
#endif
}

void PersistentHash::clearASTs(LanguageSupport* language)
{
#ifndef NO_GOOGLE_SPARSEHASH
#else
    QMutableHashIterator<KUrl, AST*> it = d->m_astHash;
    while (it.hasNext()) {
        it.next();
        if (it.value()->language == language) {
            it.value()->release();
            it.remove();
        }
    }
#endif
}

AST* PersistentHash::retrieveAST( const QString &filename )
{
    QReadLocker lock(&d->m_mutex);

#ifndef NO_GOOGLE_SPARSEHASH
    return d->m_astHash[ url.url() ];
#else
    if (d->m_filenameAstHash.contains(filename))
        return d->m_filenameAstHash.values(filename).first();

    return 0;
#endif
}

void PersistentHash::load()
{
    QWriteLocker lock(&d->m_mutex);

#ifndef NO_GOOGLE_SPARSEHASH
    kDebug() << k_funcinfo << endl;
    QString fileName = "/home/kde/trunk/KDE/kdevelop/.kdev4/pcstest";
    FILE *f = fopen( fileName.toLatin1().data(), "r" );
    if ( f )
    {
        d->m_astHash.read_metadata( f );
    }

    fpos_t pos;
    if ( fgetpos( f, &pos ) )
    {
        kDebug() << "WARNING couldn't skip metadata!! ABORT" << endl;
        return;
    }

    QFile file;
    if ( file.open( f, QIODevice::ReadOnly ) )
    {
        file.seek( pos.__pos );

        QDataStream in( &file );

        PHASH::iterator it = d->m_astHash.begin();
        for ( ; it != d->m_astHash.end(); ++it )
        {
            QString str;
            qint32 ast;
            in >> str >> ast;
            new (const_cast<QString*>(&it->first)) QString(str);
            new (&it->second) int(ast);
        }

        file.close();
    }

    PHASH::iterator it = d->m_astHash.begin();
    for ( ; it != d->m_astHash.end(); ++it )
    {
        kDebug() << "key = " << it->first << " value = " << it->second << endl;
    }
#endif
}

void PersistentHash::save()
{
    QReadLocker lock(&d->m_mutex);

#ifndef NO_GOOGLE_SPARSEHASH
    kDebug() << k_funcinfo << endl;
    QString fileName = "/home/kde/trunk/KDE/kdevelop/.kdev4/pcstest";
    FILE *f = fopen( fileName.toLatin1().data(), "w" );
    if ( f )
    {
        d->m_astHash.write_metadata( f );
    }
    fclose(f);

    std::ofstream out( fileName.toLatin1().data(), std::ios::binary | std::ios::app );
    if (out.is_open())
    {
        PHASH::iterator it = d->m_astHash.begin();
        for ( ; it != d->m_astHash.end(); ++it )
        {
            char *s = it->first.toLatin1().data();
            out.write( reinterpret_cast<char*>(s), sizeof(s) );
            Core::activeLanguage()->write( it->second, out );
        }

        out.flush();
        out.close();
    }
#endif
}

QStringList PersistentHash::extensions()
{
    return QStringList() << "IPersistentHash";
}

KDEV_USE_EXTENSION_INTERFACE( IPersistentHash, PersistentHash )

}
#include "persistenthash.moc"

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
