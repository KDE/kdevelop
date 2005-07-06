/* This file is part of KDevelop
    Copyright (C) 2005 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "control.h"
#include "lexer.h"

Control::Control()
  : current_context(0),
    _M_skipFunctionBody(false)
{
  pushContext();

  declareTypedef(findOrInsertName("__builtin_va_list",
		 strlen("__builtin_va_list")), 0);
}

Control::~Control()
{
  popContext();

  assert(current_context == 0);
}

Type *Control::lookupType(const NameSymbol *name) const
{
  assert(current_context != 0);

  return current_context->resolve(name);
}

void Control::declare(const NameSymbol *name, Type *type)
{
  //printf("*** Declare:");
  //printSymbol(name);
  //putchar('\n');
  assert(current_context != 0);

  current_context->bind(name, type);
}

void Control::pushContext()
{
  // printf("+Context\n");
  Context *new_context = new Context;
  new_context->parent = current_context;
  current_context = new_context;
}

void Control::popContext()
{
  // printf("-Context\n");
  assert(current_context != 0);

  Context *old_context = current_context;
  current_context = current_context->parent;

  delete old_context;
}

void Control::declareTypedef(const NameSymbol *name, Declarator *d)
{
  //  printf("declared typedef:");
  //  printSymbol(name);
  //  printf("\n");
  stl_typedef_table.insert(std::make_pair(name, d));
}

bool Control::isTypedef(const NameSymbol *name) const
{
  //  printf("is typedef:");
  //  printSymbol(name);
  // printf("= %d\n", (stl_typedef_table.find(name) != stl_typedef_table.end()));

  return stl_typedef_table.find(name) != stl_typedef_table.end();
}
