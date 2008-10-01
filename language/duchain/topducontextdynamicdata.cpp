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

void saveDUChainItem(QList<ArrayWithPosition>& data, DUChainBase& item, uint& totalDataOffset) {
  
  if(!item.d_func()->classId) {
    //If this triggers, you have probably created an own DUChainBase based class, but haven't called setClassId(this) in the constructor.
    kError() << QString("no class-id set for data attached to a declaration of type %1").arg(typeid(item).name());
    Q_ASSERT(0);
  }
  
  int size = DUChainItemSystem::self().dynamicSize(*item.d_func());
  
  if(data.back().first.size() - int(data.back().second) < size)
      //Create a new data item
      data.append( qMakePair(QByteArray(size > 10000 ? size : 10000, 0), 0u) );
  
  uint pos = data.back().second;
  data.back().second += size;
  totalDataOffset += size;
  
  DUChainBaseData& target(*((DUChainBaseData*)(data.back().first.constData() + pos)));
  DUChainItemSystem::self().copy(*item.d_func(), target, true);
  Q_ASSERT(!target.isDynamic());
  item.setData(&target);
}

TopDUContextDynamicData::TopDUContextDynamicData(TopDUContext* topContext) : m_topContext(topContext), m_onDisk(false), m_fastDeclarations(0), m_fastDeclarationsSize(0), m_fastContexts(0), m_fastContextsSize(0) {
}

TopDUContextDynamicData::~TopDUContextDynamicData() {
}

IndexedString TopDUContextDynamicData::loadUrl(uint topContextIndex) {
  QString baseDir = globalItemRepositoryRegistry().path() + "/topcontexts";
  QString fileName = baseDir + "/" + QString("%1").arg(topContextIndex);
  QFile file(fileName);
  if(file.open(QIODevice::ReadOnly)) {

    uint readValue;
    file.read((char*)&readValue, sizeof(uint));
    file.seek(sizeof(ItemDataInfo)*readValue + file.pos());

    file.read((char*)&readValue, sizeof(uint));
    file.seek(sizeof(ItemDataInfo)*readValue + file.pos());
    
    QByteArray data = file.read(sizeof(TopDUContextData));
    const TopDUContextData* topData = (const TopDUContextData*)data.constData();
    Q_ASSERT(topData->m_url.isEmpty() || topData->m_url.index() >> 16);
    return topData->m_url;
  }
  
  return IndexedString();
}

TopDUContext* TopDUContextDynamicData::load(uint topContextIndex) {
  QString baseDir = globalItemRepositoryRegistry().path() + "/topcontexts";
  KStandardDirs::makeDir(baseDir);
  
  QString fileName = baseDir + "/" + QString("%1").arg(topContextIndex);
  QFile file(fileName);
  if(file.open(QIODevice::ReadOnly)) {
    
    QVector<ItemDataInfo> contextDataOffsets;
    QVector<ItemDataInfo> declarationDataOffsets;

    uint readValue;
    file.read((char*)&readValue, sizeof(uint));
    contextDataOffsets.resize(readValue);
    file.read((char*)contextDataOffsets.data(), sizeof(ItemDataInfo) * contextDataOffsets.size());

    file.read((char*)&readValue, sizeof(uint));
    declarationDataOffsets.resize(readValue);
    file.read((char*)declarationDataOffsets.data(), sizeof(ItemDataInfo) * declarationDataOffsets.size());
    
    QByteArray data = file.readAll();
    DUChainBaseData* topData = (DUChainBaseData*)data.constData();
    
    TopDUContext* ret = dynamic_cast<TopDUContext*>(DUChainItemSystem::self().create(topData));
    Q_ASSERT(ret); //If this triggers, the top-context type is not registered. Most probably it means that the specific language-support is not loaded.

    TopDUContextDynamicData& target(*ret->m_dynamicData);
    target.m_data.clear();
    target.m_data.append(qMakePair(data, (uint)data.size()));
    target.m_contextDataOffsets = contextDataOffsets;
    target.m_declarationDataOffsets = declarationDataOffsets;
    
    //Fill with zeroes for now, will be initialized on-demand
    target.m_contexts.resize(contextDataOffsets.size());
    target.m_fastContexts = target.m_contexts.data();
    target.m_fastContextsSize = target.m_contexts.size();
    
    target.m_declarations.resize(declarationDataOffsets.size());
    target.m_fastDeclarations = target.m_declarations.data();
    target.m_fastDeclarationsSize = target.m_declarations.size();
    
    target.m_onDisk = true;
    
    ret->rebuildDynamicData(0, topContextIndex);
    
    return ret;
  }else{
    kWarning() << "Cannot open top-context for reading:" << fileName;
    return 0;
  }
}

bool TopDUContextDynamicData::isOnDisk() const {
  return m_onDisk;
}

void TopDUContextDynamicData::store() {
  if(m_onDisk) {
    //Check if something has changed. If nothing has changed, don't store to disk.
    bool someThingChanged = false;
    if(m_topContext->d_ptr->m_dynamic)
      someThingChanged = true;
    
    for(int a = 0; a < m_fastContextsSize; ++a) {
      if(m_fastContexts[a] && m_fastContexts[a]->d_ptr->m_dynamic)
        someThingChanged = true;
      
      if(someThingChanged)
        break;
    }

    for(int a = 0; a < m_fastDeclarationsSize; ++a) {
      if(m_fastDeclarations[a] && m_fastDeclarations[a]->d_ptr->m_dynamic)
        someThingChanged = true;
      
      if(someThingChanged)
        break;
    }
    if(!someThingChanged)
      return;
  }
  
  QString baseDir = globalItemRepositoryRegistry().path() + "/topcontexts";
  KStandardDirs::makeDir(baseDir);
  
  QFile file(baseDir + "/" + QString("%1").arg(m_topContext->ownIndex()));
  if(file.open(QIODevice::WriteOnly)) {
    file.resize(0);
    QList<ArrayWithPosition> oldDatas = m_data; //Keep the old data alive until everything is stored into a new data structure
    
    //Load all lazy declarations/contexts
    for(int a = 0; a < m_fastContextsSize; ++a)
      getContextForIndex(a+1); //Load the context
    for(int a = 0; a < m_fastDeclarationsSize; ++a)
      getDeclarationForIndex(a+1); //Load the declaration
    
    //We don't need these structures any more, since we have loaded all the declarations/contexts, and m_data
    //will be reset which these structures pointed into
    m_contextDataOffsets.clear();
    m_declarationDataOffsets.clear();
    
    m_data.clear();

    uint newDataSize = 0;
    foreach(ArrayWithPosition array, oldDatas)
        newDataSize += array.second;
    
    m_data.append( qMakePair(QByteArray(newDataSize, 0), (uint)0) );
    
    m_topContext->aboutToSave();
    
    uint currentDataOffset = 0;
    
    saveDUChainItem(m_data, *m_topContext, currentDataOffset);

    QVector<ItemDataInfo> contextDataOffsets;
    QVector<ItemDataInfo> declarationDataOffsets;
    
    for(int a = 0; a < m_fastContextsSize; ++a) {
      if(!m_fastContexts[a]) {
        contextDataOffsets << ItemDataInfo();
      } else {
        contextDataOffsets << ItemDataInfo(currentDataOffset, LocalIndexedDUContext(m_fastContexts[a]->parentContext()).localIndex());
        m_fastContexts[a]->aboutToSave();
        saveDUChainItem(m_data, *m_fastContexts[a], currentDataOffset);
        
        //Normally the m_inSymbolTable property is initialized with false, but we want to preserve it when saving to disk
//        static_cast<DUContextData*>((DUChainBaseData*)(data.data() + contextDataOffsets.back()))->m_inSymbolTable = m_fastContexts[a]->d_func()->m_inSymbolTable;
        
        //Q_ASSERT(data.size() == contextDataOffsets.back() + DUChainItemSystem::self().dynamicSize(*m_fastContexts[a]->d_func()));
      }
    }
    
    for(int a = 0; a < m_fastDeclarationsSize; ++a) {
      if(!m_fastDeclarations[a]) {
        declarationDataOffsets << ItemDataInfo();
      } else {
        declarationDataOffsets << ItemDataInfo(currentDataOffset, LocalIndexedDUContext(m_fastDeclarations[a]->context()).localIndex());
        m_fastDeclarations[a]->aboutToSave();
        saveDUChainItem(m_data, *m_fastDeclarations[a], currentDataOffset);
        
        //Normally the m_inSymbolTable property is initialized with false, but we want to preserve it when saving to disk
//        static_cast<DeclarationData*>((DUChainBaseData*)(data.data() + declarationDataOffsets.back()))->m_inSymbolTable = m_declarations[a]->d_func()->m_inSymbolTable;
        
        //Q_ASSERT(data.size() == declarationDataOffsets.back() + DUChainItemSystem::self().dynamicSize(*m_declarations[a]->d_func()));
      }
    }
    
    uint writeValue = contextDataOffsets.size();
    file.write((char*)&writeValue, sizeof(uint));
    file.write((char*)contextDataOffsets.data(), sizeof(ItemDataInfo) * contextDataOffsets.size());

    writeValue = declarationDataOffsets.size();
    file.write((char*)&writeValue, sizeof(uint));
    file.write((char*)declarationDataOffsets.data(), sizeof(ItemDataInfo) * declarationDataOffsets.size());
    
    foreach(const ArrayWithPosition& pos, m_data)
      file.write(pos.first.constData(), pos.second);
    
    m_onDisk = true;
    
  }else{
    kWarning() << "Cannot open top-context for writing";
  }
}

uint TopDUContextDynamicData::allocateDeclarationIndex(Declaration* decl, bool temporary) {
  if(!temporary) {
    m_declarations.append(decl);
    m_fastDeclarations = m_declarations.data();
    m_fastDeclarationsSize = m_declarations.size();
    return m_fastDeclarationsSize;
  }else{
    QMutexLocker lock(&m_temporaryDataMutex);
    m_temporaryDeclarations.append(decl);
    return 0xffffffff - m_temporaryDeclarations.size();
  }
}

bool TopDUContextDynamicData::isDeclarationForIndexLoaded(uint index) const {
  if(index < (0xffffffff/2)) {
    if(index == 0 || index > uint(m_fastDeclarationsSize))
      return false;
    return (bool)m_fastDeclarations[index-1];
  }else{
    return true;
  }
}

bool TopDUContextDynamicData::isContextForIndexLoaded(uint index) const {
  if(index < (0xffffffff/2)) {
    if(index == 0)
      return true;
    if(index > uint(m_fastContextsSize))
      return false;
    return (bool)m_fastContexts[index-1];
  }else{
    return true;
  }
}

Declaration* TopDUContextDynamicData::getDeclarationForIndex(uint index) const {
  if(index < (0xffffffff/2)) {
    if(index == 0 || index > uint(m_fastDeclarationsSize))
      return 0;
    else {
      uint realIndex = index-1;
      if(!m_fastDeclarations[realIndex] && realIndex < (uint)m_declarationDataOffsets.size() && m_declarationDataOffsets[realIndex].dataOffset) {
        m_fastDeclarations[realIndex] = dynamic_cast<Declaration*>(DUChainItemSystem::self().create((DUChainBaseData*)(m_data.first().first.constData() + m_declarationDataOffsets[realIndex].dataOffset)));
        if(!m_fastDeclarations[realIndex]) {
          //When this happens, the declaration has not been registered correctly.
          //We can stop here, because else we will get crashes later.
          kError() << "Failed to load declaration with identity" << ((DUChainBaseData*)(m_data.first().first.constData() + m_declarationDataOffsets[realIndex].dataOffset))->classId;
          Q_ASSERT(0);
        }else{
          DUContext* context = getContextForIndex(m_declarationDataOffsets[realIndex].parentContext);
          Q_ASSERT(context); //If this triggers, the context has been deleted without deleting its contained declarations
          m_fastDeclarations[realIndex]->rebuildDynamicData(context, index);
        }
      }
      
      return m_fastDeclarations[realIndex];
    }
  }else{
    QMutexLocker lock(&m_temporaryDataMutex);
    index = 0xffffffff - index;
    if(index == 0 || index > uint(m_temporaryDeclarations.size()))
      return 0;
    else
      return m_temporaryDeclarations[index-1];
  }
}

void TopDUContextDynamicData::clearDeclarationIndex(Declaration* decl) {
  uint index = decl->m_indexInTopContext;
  if(index < (0xffffffff/2)) {
    if(index == 0 || index > uint(m_fastDeclarationsSize))
      return;
    else {
      Q_ASSERT(m_fastDeclarations[index-1] == decl);
      m_fastDeclarations[index-1] = 0;
      
      if(index-1 < (uint)m_declarationDataOffsets.size())
        m_declarationDataOffsets[index-1] = ItemDataInfo();
    }
  }else{
    QMutexLocker lock(&m_temporaryDataMutex);
    index = 0xffffffff - index;
    if(index == 0 || index > uint(m_temporaryDeclarations.size()))
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
    m_fastContexts = m_contexts.data();
    m_fastContextsSize = m_contexts.size();
    return m_fastContextsSize;
  }else{
    QMutexLocker lock(&m_temporaryDataMutex);
    m_temporaryContexts.append(decl);
    return 0xffffffff - m_temporaryContexts.size();
  }
}

DUContext* TopDUContextDynamicData::getContextForIndex(uint index) const {
  
  if(index < (0xffffffff/2)) {
    if(index == 0)
      return m_topContext;
    if(index > uint(m_fastContextsSize))
      return 0;
    else {
      uint realIndex = index-1;
      const DUContext** fastContextsPos = m_fastContexts + realIndex;
      if(*fastContextsPos) //Shortcut, because this is the most common case
        return *fastContextsPos;
      
      if(!*fastContextsPos && realIndex < (uint)m_contextDataOffsets.size() && m_contextDataOffsets[realIndex].dataOffset) {
        //Construct the context, and eventuall its parent first
        *fastContextsPos = dynamic_cast<DUContext*>(DUChainItemSystem::self().create((DUChainBaseData*)(m_data.first().first.constData() + m_contextDataOffsets[realIndex].dataOffset)));
        if(!*fastContextsPos) {
          //When this happens, the declaration has not been registered correctly.
          //We can stop here, because else we will get crashes later.
          kError() << "Failed to load declaration with identity" << ((DUChainBaseData*)(m_data.first().first.constData() + m_contextDataOffsets[realIndex].dataOffset))->classId;
          Q_ASSERT(0);
        }else{
          (*fastContextsPos)->rebuildDynamicData(getContextForIndex(m_contextDataOffsets[realIndex].parentContext), index);
        }
      }
      
      return *fastContextsPos;
    }
  }else{
    QMutexLocker lock(&m_temporaryDataMutex);
    index = 0xffffffff - index;
    if(index == 0 || index > uint(m_temporaryContexts.size()))
      return 0;
    else
      return m_temporaryContexts[index-1];
  }
}

void TopDUContextDynamicData::clearContextIndex(DUContext* decl) {
  uint index = decl->m_dynamicData->m_indexInTopContext;
  if(index < (0xffffffff/2)) {
    
    if(index == 0 || index > uint(m_fastContextsSize))
      return;
    else {
      Q_ASSERT(m_fastContexts[index-1] == decl);
      m_fastContexts[index-1] = 0;
      
      if(index-1 < (uint)m_contextDataOffsets.size())
        m_contextDataOffsets[index-1] = ItemDataInfo();
    }
  }else{
    QMutexLocker lock(&m_temporaryDataMutex);
    index = 0xffffffff - index;
    if(index == 0 || index > uint(m_temporaryContexts.size()))
      return;
    else {
      Q_ASSERT(m_temporaryContexts[index-1] == decl);
      m_temporaryContexts[index-1] = 0;
    }
  }
}
