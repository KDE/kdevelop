/*
   (c) 2009 Bertjan Broeksema <b.broeksema@home.nl>

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

#include "parentvisitor.h"

#include <util/pushvalue.h>

ParentVisitor::ParentVisitor() : m_currentParent(0)
{ }

void ParentVisitor::visit(AST* node)
{
  if (node)
    node->parent = m_currentParent;

  PushValue<AST*> pushCurrentParent(m_currentParent, node);
  DefaultVisitor::visit(node);
}
