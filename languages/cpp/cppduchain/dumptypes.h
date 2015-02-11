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
  virtual bool preVisit (const KDevelop::AbstractType * type) override;
  virtual void postVisit (const KDevelop::AbstractType *) override;

  virtual void visit (const KDevelop::IntegralType *) override;

  virtual bool visit (const KDevelop::AbstractType *) override;

  virtual bool visit (const KDevelop::PointerType * type) override;
  virtual void endVisit (const KDevelop::PointerType *) override;

  virtual bool visit (const KDevelop::ReferenceType * type) override;
  virtual void endVisit (const KDevelop::ReferenceType *) override;

  virtual bool visit (const KDevelop::FunctionType * type) override;
  virtual void endVisit (const KDevelop::FunctionType *) override;

  virtual bool visit (const KDevelop::StructureType * type) override;
  virtual void endVisit (const KDevelop::StructureType *) override;

  virtual bool visit (const KDevelop::ArrayType * type) override;
  virtual void endVisit (const KDevelop::ArrayType *) override;

private:
  bool seen(const KDevelop::AbstractType* type);

  int indent;
  QSet<const KDevelop::AbstractType*> m_encountered;
};

#endif // DUMPTYPES_H

