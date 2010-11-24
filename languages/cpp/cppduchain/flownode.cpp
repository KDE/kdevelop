/* This file is part of KDevelop
    Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

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

#include "flownode.h"

using namespace KDevelop;

FlowNode::FlowNode(const KDevelop::RangeInRevision& range, FlowNode* next, FlowNode* alternative, const KDevelop::RangeInRevision& condition)
  : m_nodeRange(range), m_conditionRange(condition), m_next(next), m_alternative(alternative)
{
  Q_ASSERT(!m_alternative || m_next); //If we have alternative, we have next.
}

FlowNode FlowNode::createConditional(const RangeInRevision& r, FlowNode* n, FlowNode* a, const RangeInRevision& c) { return FlowNode(r, n, a, c); }
FlowNode FlowNode::createExit(const RangeInRevision& r) { return FlowNode(r); }
FlowNode FlowNode::createSequential(const RangeInRevision& r, FlowNode* next) { return FlowNode(r, next); }

FlowNode::Type FlowNode::type() const
{
  if(m_next && m_alternative) return Conditional;
  else if(m_next) return Sequential;
  else return Exit;
}
