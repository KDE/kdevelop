/* This file is part of KDevelop
    Copyright (C) 2006 Roberto Raggi <roberto@kdevelop.org>

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

#ifndef DUBUILDER_H
#define DUBUILDER_H

#include "default_visitor.h"

class DUBuilder: protected DefaultVisitor
{
public:
  DUBuilder ();
  virtual ~DUBuilder ();

  void operator () (AST *node);

protected:
  virtual void visitNamespace (NamespaceAST *);
  virtual void visitClassSpecifier (ClassSpecifierAST *);

  inline bool inNamespace (bool f) {
    bool was = in_namespace;
    in_namespace = f;
    return was;
  }

  inline bool inClass (bool f) {
    bool was = in_class;
    in_class = f;
    return was;
  }

private:
  union {
    struct {
      uint in_namespace: 1;
      uint in_class: 1;
    };

    uint _M_flags;
  };
};

#endif // DUBUILDER_H

// kate: indent-width 2;
