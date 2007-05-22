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

#ifndef FORWARDDECLARATION_H
#define FORWARDDECLARATION_H

#include "declaration.h"

/**
 * Represents a forward declaration
 */
class ForwardDeclaration : public Declaration
{
public:
  ForwardDeclaration(KTextEditor::Range* range, Scope scope, DUContext* context);
  virtual ~ForwardDeclaration();

  virtual bool isForwardDeclaration() const;

  Declaration* resolved() const;
  void setResolved(Declaration* declaration);

private:
  Declaration* m_resolvedDeclaration;
};

#endif // DECLARATION_H

// kate: indent-width 2;
