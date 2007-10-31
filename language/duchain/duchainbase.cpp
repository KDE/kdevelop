/* This  is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#include "duchainbase.h"
#include "duchainpointer.h"

namespace KDevelop
{


DUChainBase::DUChainBase(KTextEditor::Range* range)
  : KDevelop::DocumentRangeObject(range), m_ptr( 0L )
{
}

DUChainBase::~DUChainBase()
{
  if (m_ptr)
    m_ptr->m_base = 0;
}

TopDUContext* DUChainBase::topContext() const
{
  return 0;
}

const KSharedPtr<DUChainPointerData>& DUChainBase::weakPointer() const
{
  if (!m_ptr) {
    m_ptr = new DUChainPointerData(const_cast<DUChainBase*>(this));
    m_ptr->m_base = const_cast<DUChainBase*>(this);
  }

  return m_ptr;
}

}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
