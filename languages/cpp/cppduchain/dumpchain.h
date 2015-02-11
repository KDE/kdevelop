/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
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

#ifndef CPP_DUMPCHAIN_H
#define CPP_DUMPCHAIN_H


#include "default_visitor.h"
#include "cppduchainexport.h"

class ParseSession;
namespace KDevelop
{
class DUContext;
}

class CppEditorIntegrator;

namespace Cpp {
class KDEVCPPDUCHAIN_EXPORT DumpChain: protected DefaultVisitor
{
public:
  DumpChain();
  virtual ~DumpChain();

  void dump(AST *node, ParseSession* session = 0);

protected:
  virtual void visit(AST *node) override;

private:
  CppEditorIntegrator* m_editor;
  int indent;
};
}

#endif // DUMPCHAIN_H

