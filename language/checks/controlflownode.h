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

#ifndef CONTROLFLOWNODE_H
#define CONTROLFLOWNODE_H
#include <language/editor/rangeinrevision.h>
#include <language/languageexport.h>

namespace KDevelop
{

class KDEVPLATFORMLANGUAGE_EXPORT ControlFlowNode
{
  public:
    enum Type { Conditional, Sequential, Exit };
    ControlFlowNode();
    
    Type type() const;
    
    void setStartCursor(const CursorInRevision& cursor) {m_nodeRange.start = cursor; }
    void setEndCursor(const CursorInRevision& cursor) {m_nodeRange.end = cursor; }
    
    KDevelop::RangeInRevision m_nodeRange;
    KDevelop::RangeInRevision m_conditionRange;
    
    ControlFlowNode* m_next;
    ControlFlowNode* m_alternative;
};

}
#endif // FLUXNODE_H
