/* This file is part of KDevelop
Copyright (C) 2006 Adam Treat <treat@kde.org>
Copyright (C) 2007 Andreas Pakulat <apaku@gmx.de>

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

#ifndef IPERSISTENTHASH_H
#define IPERSISTENTHASH_H

#include "iextension.h"
#include <QtDesigner/QAbstractExtensionFactory>
#include "kdevexport.h"

class KUrl;

namespace KDevelop
{
struct AST;

class LanguageSupport;

class KDEVPLATFORM_EXPORT IPersistentHash
{
public:
    virtual ~IPersistentHash() {}

    virtual void insertAST( const KUrl &url, AST* ast ) = 0;
    virtual AST* retrieveAST( const KUrl &url ) = 0;

    // FIXME hack, returns the first AST with a matching filename...
    // remove when no longer needed
    virtual AST* retrieveAST( const QString &filename ) = 0;

    virtual void clearASTs(LanguageSupport* language) = 0;

    virtual void load() = 0;
    virtual void save() = 0;

};

}

KDEV_DECLARE_EXTENSION_INTERFACE( KDevelop, IPersistentHash, "org.kdevelop.IPersistentHash" )

Q_DECLARE_INTERFACE( KDevelop::IPersistentHash, "org.kdevelop.IPersistentHash"  )

#endif

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
