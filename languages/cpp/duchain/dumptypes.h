/* This file is part of KDevelop
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#include "typesystem.h"

class DumpTypes : protected TypeVisitor
{
public:
  DumpTypes();
  virtual ~DumpTypes();

  void dump(const AbstractType* type);

protected:
  virtual bool preVisit (const AbstractType * type);
  virtual void postVisit (const AbstractType * type);

  virtual void visit (const IntegralType * type);

  virtual bool visit (const PointerType * type);
  virtual void endVisit (const PointerType * type);

  virtual bool visit (const ReferenceType * type);
  virtual void endVisit (const ReferenceType * type);

  virtual bool visit (const FunctionType * type);
  virtual void endVisit (const FunctionType * type);

  virtual bool visit (const StructureType * type);
  virtual void endVisit (const StructureType * type);

  virtual bool visit (const ArrayType * type);
  virtual void endVisit (const ArrayType * type);

private:
  bool seen(const AbstractType* type);

  class CppEditorIntegrator* m_editor;
  int indent;
  QSet<const AbstractType*> m_encountered;
};

#endif // DUMPTYPES_H

// kate: space-indent on; indent-width 2; replace-tabs on;
