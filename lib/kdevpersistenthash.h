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

#ifndef KDEVPERSISTENTHASH_H
#define KDEVPERSISTENTHASH_H

#include <QObject>
#include <QReadWriteLock>

#include <kurl.h>
#include <kdebug.h>

#include "kdevexport.h"

/** This class is a stub at the moment.  It works when you have google sparsehash installed.*/

namespace Koncrete
{
struct AST;
}

#define NO_GOOGLE_SPARSEHASH
#ifndef NO_GOOGLE_SPARSEHASH

#include <google/sparse_hash_map>

using google::sparse_hash_map;

struct QStrHash
{
    size_t operator() ( const QString& s ) const
    {
        return HASH_NAMESPACE::hash<const char*>() ( s.toLatin1().constData() );
    }
};

typedef sparse_hash_map<QString, Koncrete::AST*, QStrHash> PHASH;
#else

#include <QHash>

#endif

namespace Koncrete
{
    
class LanguageSupport;

class KDEVPLATFORM_EXPORT PersistentHash: public QObject
{
    Q_OBJECT
public:
    PersistentHash( QObject *parent = 0 );
    virtual ~PersistentHash();

    void insertAST( const KUrl &url, AST* ast );
    AST* retrieveAST( const KUrl &url );

    // FIXME hack, returns the first AST with a matching filename...
    // remove when no longer needed
    AST* retrieveAST( const QString &filename );

    void clearASTs(LanguageSupport* language);

    void load();
    void save();

private:
#ifndef NO_GOOGLE_SPARSEHASH
    PHASH m_astHash;
#else
    QHash<KUrl, AST*> m_astHash;
    QMultiHash<QString, AST*> m_filenameAstHash;
#endif

    mutable QReadWriteLock m_mutex;
};

}
#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
