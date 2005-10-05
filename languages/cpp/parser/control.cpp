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

#include "control.h"
#include "lexer.h"

Control::Control()
  : _M_skip_function_body(false),
    _M_current_context(0)

{
  pushContext();

  declareTypedef(findOrInsertName("__builtin_va_list",
		 strlen("__builtin_va_list")), 0);
}

Control::~Control()
{
  popContext();

  Q_ASSERT(_M_current_context == 0);
}

int Control::problemCount() const
{
  return _M_problems.count();
}

Problem Control::problem(int index) const
{
  return _M_problems.at(index);
}

void Control::reportProblem(const Problem &problem)
{
  _M_problems.append(problem);
}

Type *Control::lookupType(const NameSymbol *name) const
{
  Q_ASSERT(_M_current_context != 0);

  return _M_current_context->resolve(name);
}

void Control::declare(const NameSymbol *name, Type *type)
{
  //printf("*** Declare:");
  //printSymbol(name);
  //putchar('\n');
  Q_ASSERT(_M_current_context != 0);

  _M_current_context->bind(name, type);
}

void Control::pushContext()
{
  // printf("+Context\n");
  Context *new_context = new Context;
  new_context->parent = _M_current_context;
  _M_current_context = new_context;
}

void Control::popContext()
{
  // printf("-Context\n");
  Q_ASSERT(_M_current_context != 0);

  Context *old_context = _M_current_context;
  _M_current_context = _M_current_context->parent;

  delete old_context;
}

void Control::declareTypedef(const NameSymbol *name, Declarator *d)
{
  //  printf("declared typedef:");
  //  printSymbol(name);
  //  printf("\n");
  _M_typedef_table.insert(name, d);
}

bool Control::isTypedef(const NameSymbol *name) const
{
  //  printf("is typedef:");
  //  printSymbol(name);
  // printf("= %d\n", (_M_typedef_table.find(name) != _M_typedef_table.end()));

  return _M_typedef_table.contains(name);
}

// kate: space-indent on; indent-width 2; replace-tabs on;
