/*
   This  is part of KDevelop
   Copyright 2007 Bernd Buschinski <b.buschinski@web.de>

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

#include "duchainpointer.h"

namespace KDevelop
{

DUChainBase* DUChainPointerData::base()
{
  return m_base;
}

DUChainBase* DUChainPointerData::base() const
{
  return m_base;
}

DUChainPointerData::DUChainPointerData()
  : m_base(0)
{
}

DUChainPointerData::~DUChainPointerData()
{
}

DUChainPointerData::DUChainPointerData( DUChainBase* base )
  : m_base(base)
{
}

} //KDevelop
