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

#include "controlflownode.h"

using namespace KDevelop;

ControlFlowNode::ControlFlowNode()
  : m_next(0), m_alternative(0)
{}

ControlFlowNode::Type ControlFlowNode::type() const
{
  Q_ASSERT(!m_alternative || m_next); //If we have alternative, we have next.
  
  if(m_next && m_alternative) return Conditional;
  else if(m_next) return Sequential;
  else return Exit;
}
