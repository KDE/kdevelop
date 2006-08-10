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

#include <kurl.h>
#include <kdebug.h>

#include "kdevexport.h"

/** This class is a stub at the moment.  It works when you have google sparsehash installed.*/

// #include <google/sparse_hash_map>

// using google::sparse_hash_map;

struct KDevAST;

// struct QStrHash
// {
//     size_t operator() ( const QString& s ) const
//     {
//         return HASH_NAMESPACE::hash<const char*>() ( s.toLatin1().constData() );
//     }
// };

// typedef sparse_hash_map<QString, KDevAST*, QStrHash> PHASH;

class KDEVINTERFACES_EXPORT KDevPersistentHash: public QObject
{
    Q_OBJECT
public:
    KDevPersistentHash( QObject *parent = 0 );
    virtual ~KDevPersistentHash();

    void insert( const KUrl &url, KDevAST* ast );
    void load();
    void save();

// private:
//     PHASH m_hash;
};

#endif

// kate: space-indent on; indent-width 4; tab-width 4; replace-tabs on
