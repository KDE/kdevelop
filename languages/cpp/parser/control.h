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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef CONTROL_H
#define CONTROL_H

#include "symbol.h"
#include "memorypool.h"
#include "problem.h"

#include <QtCore/QHash>

struct Declarator;
struct Type;

/**
Context which defines a binding of names and types.

In other words, context could be considered as visibility
area of a name.@n
Contexts can be nested so we could find names from the parent
contexts. For example, name "a" can be resolved from the innermost
context by looking into parent context:
@code
{
  int a;
  {
    int b = 2;
    a = 2+b;
  }
}
@endcode
*/
struct Context
{
  Context *parent;

  /**Binds a name to the type in this context.*/
  inline void bind(const NameSymbol *name, Type *type)
  { symbol_table.insert(name, type); }

  /**Resolves a type of the symbol if it is found in this
  or parent context. Returns 0 in case the symbol is not
  bound to a type.*/
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

/**Parser control information.
Provides problem tracking, context(visibility)-aware symbol name tables,
typedef table and other information about the parsing process.*/
class Control
{
public:
  Control();
  ~Control();

  /**@return the number of problems encountered during parsing.*/
  int problemCount() const;
  /**@return the problem by its index.*/
  Problem problem(int index) const;

  /**Adds a problem to the list of problems.*/
  void reportProblem(const Problem &problem);

  inline bool skipFunctionBody() const { return _M_skip_function_body; }
  inline void setSkipFunctionBody(bool skip) { _M_skip_function_body = skip; }

  /**@return current parser's context.*/
  inline Context *currentContext() const
  { return _M_current_context; }

  /**Creates a new context which is nested into the current and
  sets the new one as the current.*/
  void pushContext();
  /**Removes the current context and sets its parent context
  as the current.*/
  void popContext();

  /**@return the type of the symbol. Tries to resolve all
  symbols in this and parent contexts.*/
  Type *lookupType(const NameSymbol *name) const;
  /**Declares a symbol in the current context.*/
  void declare(const NameSymbol *name, Type *type);

  /**Looks for name symbol @p data in the name table and returns it. If a
  name does not exist in the table, it is created.
  @param data buffer to store the name of the symbol.
  @param count length of the name in @p data buffer.*/
  inline const NameSymbol *findOrInsertName(const char *data, size_t count)
  { return _M_name_table.findOrInsert(data, count); }

  void declareTypedef(const NameSymbol *name, Declarator *d);
  bool isTypedef(const NameSymbol *name) const;

private:
  bool _M_skip_function_body;
  Context *_M_current_context;
  NameTable _M_name_table;
  QHash<const NameSymbol*, Declarator*> _M_typedef_table;
  QList<Problem> _M_problems;
};

#endif // CONTROL_H

// kate: space-indent on; indent-width 2; replace-tabs on;
