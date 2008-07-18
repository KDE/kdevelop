/* This  is part of KDevelop
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

#include "topducontextdynamicdata.h"
#include "declaration.h"
#include "ducontext.h"
#include "ducontextdata.h"

using namespace KDevelop;

QMutex KDevelop::TopDUContextDynamicData::m_temporaryDataMutex(QMutex::Recursive);

TopDUContextDynamicData::TopDUContextDynamicData(TopDUContext* topContext) : m_topContext(topContext) {
}

TopDUContextDynamicData::~TopDUContextDynamicData() {
}

void TopDUContextDynamicData::store() const {
  
}

uint TopDUContextDynamicData::allocateDeclarationIndex(Declaration* decl, bool temporary) {
  if(!temporary) {
    m_declarations.append(decl);
    return m_declarations.size();
  }else{
    QMutexLocker lock(&m_temporaryDataMutex);
    m_temporaryDeclarations.append(decl);
    return 0xffffffff - m_temporaryDeclarations.size();
  }
}

Declaration* TopDUContextDynamicData::getDeclarationForIndex(uint index) const {
  if(index < (0xffffffff/2)) {
    if(index == 0 || index > m_declarations.size())
      return 0;
    else
      return m_declarations[index-1];
  }else{
    QMutexLocker lock(&m_temporaryDataMutex);
    index = 0xffffffff - index;
    if(index == 0 || index > m_temporaryDeclarations.size())
      return 0;
    else
      return m_temporaryDeclarations[index-1];
  }
}

void TopDUContextDynamicData::clearDeclarationIndex(Declaration* decl) {
  uint index = decl->m_indexInTopContext;
  if(index < (0xffffffff/2)) {
    if(index == 0 || index > m_declarations.size())
      return;
    else {
      Q_ASSERT(m_declarations[index-1] == decl);
      m_declarations[index-1] = 0;
    }
  }else{
    QMutexLocker lock(&m_temporaryDataMutex);
    index = 0xffffffff - index;
    if(index == 0 || index > m_temporaryDeclarations.size())
      return;
    else {
      Q_ASSERT(m_temporaryDeclarations[index-1] == decl);
      m_temporaryDeclarations[index-1] = 0;
    }
  }
}

uint TopDUContextDynamicData::allocateContextIndex(DUContext* decl, bool temporary) {
  if(!temporary) {
    m_contexts.append(decl);
    return m_contexts.size();
  }else{
    QMutexLocker lock(&m_temporaryDataMutex);
    m_temporaryContexts.append(decl);
    return 0xffffffff - m_temporaryContexts.size();
  }
}

DUContext* TopDUContextDynamicData::getContextForIndex(uint index) const {
  if(index < (0xffffffff/2)) {
    if(index == 0 || index > m_contexts.size())
      return 0;
    else
      return m_contexts[index-1];
  }else{
    QMutexLocker lock(&m_temporaryDataMutex);
    index = 0xffffffff - index;
    if(index == 0 || index > m_temporaryContexts.size())
      return 0;
    else
      return m_temporaryContexts[index-1];
  }
}

void TopDUContextDynamicData::clearContextIndex(DUContext* decl) {
  uint index = decl->m_dynamicData->m_indexInTopContext;
  if(index < (0xffffffff/2)) {
    if(index == 0 || index > m_contexts.size())
      return;
    else {
      Q_ASSERT(m_contexts[index-1] == decl);
      m_contexts[index-1] = 0;
    }
  }else{
    QMutexLocker lock(&m_temporaryDataMutex);
    index = 0xffffffff - index;
    if(index == 0 || index > m_temporaryContexts.size())
      return;
    else {
      Q_ASSERT(m_temporaryContexts[index-1] == decl);
      m_temporaryContexts[index-1] = 0;
    }
  }
}
