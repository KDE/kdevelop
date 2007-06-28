/* 
   Copyright (C) 2007 David Nolden <user@host.de>
   (where user = david.nolden.kdevelop, host = art-master)

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

namespace KDevelop {
  class Declaration;
  class DUContext;
  class QualifiedIdentifier;
};

namespace Cpp {
/**
 * Searches context, and if the identifier is not found there, in it's base-classes, as it should be in c++.
 * Du-chain must be locked before.
 * */
QList<KDevelop::Declaration*> findLocalDeclarations( KDevelop::DUContext* context, const KDevelop::QualifiedIdentifier& identifier );

}

#endif
