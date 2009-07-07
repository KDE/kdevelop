/*
   Copyright 2009 Ramón Zarazúa <killerfox512+kde@gmail.com>
   
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

#ifndef CPPDUCHAINCHANGESET_H
#define CPPDUCHAINCHANGESET_H

#include <language/codegen/duchainchangeset.h>


class CppDUChainChangeSet : public KDevelop::DUChainChangeSet
{
  public:
    CppDUChainChangeSet(KDevelop::ReferencedTopDUContext topContext) : DUChainChangeSet(topContext) {}
    
    virtual KDevelop::DUChainRef * newFunction();
    virtual KDevelop::DUChainRef * newClass();
    virtual KDevelop::DUChainRef * newDeclaration();
};

#endif // CPPDUCHAINCHANGESET_H
