/* This file is part of KDevelop
    Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef CONTROL_H
#define CONTROL_H

#include "symbol.h"
#include "memorypool.h"

#include <QtCore/QHash>

struct Declarator;
struct Type;

struct Context
{
  Context *parent;

  inline void bind(const NameSymbol *name, Type *type)
  { symbol_table.insert(name, type); }

  inline Type *resolve(const NameSymbol *name) const
  {
    if (Type *type = symbol_table.value(name))
      return type;
    else if (parent)
      return parent->resolve(name);

    return 0;
  }

  typedef QHash<const NameSymbol*, Type*> symbol_table_t;

  symbol_table_t symbol_table;
};

class Control
{
public:
  Control();
  ~Control();

  inline bool skipFunctionBody() const { return _M_skipFunctionBody; }
  inline void setSkipFunctionBody(bool skip) { _M_skipFunctionBody = skip; }

  Context *current_context;

  inline Context *currentContext() const
  { return current_context; }

  void pushContext();
  void popContext();

  Type *lookupType(const NameSymbol *name) const;
  void declare(const NameSymbol *name, Type *type);

  inline const NameSymbol *findOrInsertName(const char *data, size_t count)
  { return name_table.findOrInsert(data, count); }

  void declareTypedef(const NameSymbol *name, Declarator *d);
  bool isTypedef(const NameSymbol *name) const;

private:
  NameTable name_table;
  QHash<const NameSymbol*, Declarator*> stl_typedef_table;
  bool _M_skipFunctionBody;
};

#endif // CONTROL_H

// kate: space-indent on; indent-width 2; replace-tabs on;
