/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#ifndef DUMPTYPES_H
#define DUMPTYPES_H

#include <language/duchain/types/typesystem.h>
#include "cppduchainexport.h"

class KDEVCPPDUCHAIN_EXPORT DumpTypes : protected KDevelop::TypeVisitor
{
public:
  DumpTypes();
  virtual ~DumpTypes();

  void dump(const KDevelop::AbstractType* type);

protected:
  virtual bool preVisit (const KDevelop::AbstractType * type);
  virtual void postVisit (const KDevelop::AbstractType *);

  virtual void visit (const KDevelop::IntegralType *);

  virtual bool visit (const KDevelop::AbstractType *);

  virtual bool visit (const KDevelop::PointerType * type);
  virtual void endVisit (const KDevelop::PointerType *);

  virtual bool visit (const KDevelop::ReferenceType * type);
  virtual void endVisit (const KDevelop::ReferenceType *);

  virtual bool visit (const KDevelop::FunctionType * type);
  virtual void endVisit (const KDevelop::FunctionType *);

  virtual bool visit (const KDevelop::StructureType * type);
  virtual void endVisit (const KDevelop::StructureType *);

  virtual bool visit (const KDevelop::ArrayType * type);
  virtual void endVisit (const KDevelop::ArrayType *);

private:
  bool seen(const KDevelop::AbstractType* type);

  int indent;
  QSet<const KDevelop::AbstractType*> m_encountered;
};

#endif // DUMPTYPES_H

