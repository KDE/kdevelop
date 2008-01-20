/* 
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

/**
 * This files contains common helpers for using the du-chain in the context of c++
 * */

#ifndef CPPDUCHAIN_H
#define CPPDUCHAIN_H

#include <QList>
#include <QPair>
#include "cppduchainexport.h"

namespace KTextEditor {
  class Cursor;
}

namespace KDevelop {
  class Declaration;
  class DUContext;
  class TopDUContext;
  class QualifiedIdentifier;
  class SimpleCursor;
}

namespace Cpp {
/**
 * Searches context, and if the identifier is not found there, in its base-classes, as it should be in c++.
 * @param topContext should be the top-context from where the search starts. This is needed to resolve template-parameters.
 * Du-chain must be locked before.
 * */
KDEVCPPDUCHAIN_EXPORT QList<KDevelop::Declaration*> findLocalDeclarations( KDevelop::DUContext* context, const KDevelop::QualifiedIdentifier& identifier, const KDevelop::TopDUContext* topContext );

/**
 * Searches for declarations on the same level, either locally within the context, or namespace. \param identifier that will be declared in a given \param context
 * Tries to follow the C++ rules, that decide where a type may have been forward-declared.
 * */
QList<KDevelop::Declaration*> findDeclarationsSameLevel(KDevelop::DUContext* context, const KDevelop::QualifiedIdentifier& identifier, const KDevelop::SimpleCursor& position);

/**
 * Takes and returns a list of declarations together with inheritance-depth.
 * Since in c++ one declaration with a name in one depth overloads deeper
 * declarations, they are hidden here.
 * */
KDEVCPPDUCHAIN_EXPORT QList< QPair<KDevelop::Declaration*, int> > hideOverloadedDeclarations( const QList< QPair<KDevelop::Declaration*, int> >& declarations );
}

#endif
