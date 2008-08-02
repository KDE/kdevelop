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

#include <kstandarddirs.h>
#include <typeinfo>
#include <QFile>
#include <QByteArray>

#include "topducontextdynamicdata.h"
#include "declaration.h"
#include "declarationdata.h"
#include "ducontext.h"
#include "topducontext.h"
#include "topducontextdata.h"
#include "ducontextdata.h"
#include "duchainregister.h"
#include "repositories/itemrepository.h"

using namespace KDevelop;

QMutex KDevelop::TopDUContextDynamicData::m_temporaryDataMutex(QMutex::Recursive);

///@todo Implement all this using mmap
void saveDUChainItem(QByteArray& data, const DUChainBase& item) {
  
  if(!item.d_func()->classId) {
    //If this triggers, you have probably created an own DUChainBase based class, but haven't called setClassId(this) in the constructor.
    kError() << QString("no class-id set for data attached to a declaration of type %1").arg(typeid(item).name());
    Q_ASSERT(0);
  }
  
  int size = DUChainItemSystem::self().dynamicSize(*item.d_func());
  
  uint pos = data.size();
  
  data.resize(data.size() + size);
  
  DUChainBaseData& target(*((DUChainBaseData*)(data.constData() + pos)));
  DUChainItemSystem::self().copy(*item.d_func(), target, true);
  Q_ASSERT(!target.isDynamic());
}

TopDUContextDynamicData::TopDUContextDynamicData(TopDUContext* topContext) : m_topContext(topContext), m_onDisk(false) {
}

TopDUContextDynamicData::~TopDUContextDynamicData() {
}

TopDUContext* TopDUContextDynamicData::load(uint topContextIndex) {
  QString baseDir = globalItemRepositoryRegistry().path() + "/topcontexts";
  KStandardDirs::makeDir(baseDir);
  
  QFile file(baseDir + "/" + QString("%1").arg(topContextIndex));
  if(file.open(QIODevice::ReadOnly)) {
    
    QVector<uint> contextDataOffsets;
    QVector<uint> declarationDataOffsets;

    uint readValue;
    file.read((char*)&readValue, sizeof(uint));
    contextDataOffsets.resize(readValue);
    file.read((char*)contextDataOffsets.data(), sizeof(uint) * contextDataOffsets.size());

    file.read((char*)&readValue, sizeof(uint));
    declarationDataOffsets.resize(readValue);
    file.read((char*)declarationDataOffsets.data(), sizeof(uint) * declarationDataOffsets.size());
    
    QByteArray data = file.readAll();
    DUChainBaseData* topData = (DUChainBaseData*)data.constData();
    
    TopDUContext* ret = dynamic_cast<TopDUContext*>(DUChainItemSystem::self().create(topData));
    Q_ASSERT(ret);

    TopDUContextDynamicData& target(*ret->m_dynamicData);
    target.m_data = data;
    
    for(int a = 0; a < contextDataOffsets.size(); ++a) {
      if(!contextDataOffsets[a])
        target.m_contexts << 0;
      else
        target.m_contexts << dynamic_cast<DUContext*>(DUChainItemSystem::self().create((DUChainBaseData*)(data.constData() + contextDataOffsets[a])));
    }
    for(int a = 0; a < declarationDataOffsets.size(); ++a) {
      if(!declarationDataOffsets[a])
        target.m_declarations << 0;
      else {
        target.m_declarations << dynamic_cast<Declaration*>(DUChainItemSystem::self().create((DUChainBaseData*)(data.constData() + declarationDataOffsets[a])));
      }
    }
    
    target.m_onDisk = true;
    
    ret->rebuildDynamicData(0, topContextIndex);
    
    return ret;
  }else{
    kWarning() << "Cannot open top-context for reading";
    return 0;
  }
}

bool TopDUContextDynamicData::isOnDisk() const {
  return m_onDisk;
}

void TopDUContextDynamicData::store() {
  QString baseDir = globalItemRepositoryRegistry().path() + "/topcontexts";
  KStandardDirs::makeDir(baseDir);
  
  QFile file(baseDir + "/" + QString("%1").arg(m_topContext->ownIndex()));
  if(file.open(QIODevice::WriteOnly)) {
    file.resize(0);
    
    QByteArray data;

    m_topContext->aboutToSave();
    saveDUChainItem(data, *m_topContext);

    QVector<uint> contextDataOffsets;
    QVector<uint> declarationDataOffsets;
    
    for(int a = 0; a < m_contexts.size(); ++a) {
      if(!m_contexts[a]) {
        contextDataOffsets << 0;
      } else {
        contextDataOffsets << data.size();
        m_contexts[a]->aboutToSave();
        saveDUChainItem(data, *m_contexts[a]);
        
        //Normally the m_inSymbolTable property is initialized with false, but we want to preserve it when saving to disk
//        static_cast<DUContextData*>((DUChainBaseData*)(data.data() + contextDataOffsets.back()))->m_inSymbolTable = m_contexts[a]->d_func()->m_inSymbolTable;
        
        Q_ASSERT(data.size() == contextDataOffsets.back() + DUChainItemSystem::self().dynamicSize(*m_contexts[a]->d_func()));
      }
    }
    
    for(int a = 0; a < m_declarations.size(); ++a) {
      if(!m_declarations[a]) {
        declarationDataOffsets << 0;
      } else {
        declarationDataOffsets << data.size();
        m_declarations[a]->aboutToSave();
        saveDUChainItem(data, *m_declarations[a]);
        
        //Normally the m_inSymbolTable property is initialized with false, but we want to preserve it when saving to disk
//        static_cast<DeclarationData*>((DUChainBaseData*)(data.data() + declarationDataOffsets.back()))->m_inSymbolTable = m_declarations[a]->d_func()->m_inSymbolTable;
        
        Q_ASSERT(data.size() == declarationDataOffsets.back() + DUChainItemSystem::self().dynamicSize(*m_declarations[a]->d_func()));
      }
    }
    
    uint writeValue = contextDataOffsets.size();
    file.write((char*)&writeValue, sizeof(uint));
    file.write((char*)contextDataOffsets.data(), sizeof(uint) * contextDataOffsets.size());

    writeValue = declarationDataOffsets.size();
    file.write((char*)&writeValue, sizeof(uint));
    file.write((char*)declarationDataOffsets.data(), sizeof(uint) * declarationDataOffsets.size());
    
    file.write(data);
    
    m_onDisk = true;
    
  }else{
    kWarning() << "Cannot open top-context for writing";
  }
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
