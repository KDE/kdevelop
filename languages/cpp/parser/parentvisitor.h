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

#ifndef PARENTVISITOR_H
#define PARENTVISITOR_H

#include "default_visitor.h"

class ParseSession;

/**
 * Sets the parent node of each AST node. The root node won't have a parent set.
 */
class ParentVisitor : public DefaultVisitor
{
  public:
    ParentVisitor(ParseSession* session);
    virtual void visit(AST* node) override;

  private:
    AST *m_currentParent;
    ParseSession* m_session;
};

#endif // PARENTVISITOR_H
