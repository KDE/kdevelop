/* This  is part of KDevelop
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

#include "duchainbase.h"

namespace KDevelop
{

class DUChainBasePrivate
{
public:
  DUChainBasePrivate() {
    m_modelRow = 0;
    m_encountered = 0;
  }
  int m_modelRow;
  unsigned int m_encountered;
};


DUChainBase::DUChainBase(KTextEditor::Range* range)
  : KDevelop::DocumentRangeObject(range)
  , d(new DUChainBasePrivate)
{
}

DUChainBase::~DUChainBase()
{
  delete d;
}

int DUChainBase::modelRow() const
{
  return d->m_modelRow;
}

void DUChainBase::setModelRow(int row)
{
  d->m_modelRow = row;
}

unsigned int DUChainBase::lastEncountered() const
{
  return d->m_encountered;
}

void DUChainBase::setEncountered(unsigned int encountered)
{
  d->m_encountered = encountered;
}

TopDUContext* DUChainBase::topContext() const
{
  return 0;
}
}

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
