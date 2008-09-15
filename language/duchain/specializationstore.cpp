/* This file is part of KDevelop
    Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "specializationstore.h"
#include "declarationid.h"

namespace KDevelop {

SpecializationStore::SpecializationStore() {
}

SpecializationStore::~SpecializationStore() {
}

SpecializationStore& SpecializationStore::self() {
  static SpecializationStore store;
  return store;
}

void SpecializationStore::set(DeclarationId declaration, uint specialization) {
  Q_ASSERT(specialization >> 16);
  m_specializations[declaration] = specialization;
}

uint SpecializationStore::get(DeclarationId declaration) {
  QHash<DeclarationId, uint>::const_iterator it = m_specializations.find(declaration);
  if(it != m_specializations.end())
    return *it;
  else
    return 0;
}

void SpecializationStore::clear(DeclarationId declaration) {
  QHash<DeclarationId, uint>::iterator it = m_specializations.find(declaration);
  if(it != m_specializations.end())
    m_specializations.erase(it);
}

void SpecializationStore::clear() {
  m_specializations.clear();
}

}
