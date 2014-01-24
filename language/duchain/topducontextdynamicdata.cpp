/* This  is part of KDevelop

   Copyright 2014 Milian Wolff <mail@milianw.de>
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

#include <kstandarddirs.h>
#include <typeinfo>
#include <QtCore/QFile>
#include <QtCore/QByteArray>

#include "declaration.h"
#include "declarationdata.h"
#include "ducontext.h"
#include "topducontext.h"
#include "topducontextdata.h"
#include "ducontextdata.h"
#include "ducontextdynamicdata.h"
#include "duchainregister.h"
#include "repositories/itemrepository.h"

//#define DEBUG_DATA_INFO

//This might be problematic on some systems, because really many mmaps are created
#define USE_MMAP
using namespace KDevelop;

QMutex KDevelop::TopDUContextDynamicData::m_temporaryDataMutex(QMutex::Recursive);

namespace {

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

  DUChainBaseData& target(*(reinterpret_cast<DUChainBaseData*>(data.back().first.data() + pos)));

  if(item.d_func()->isDynamic()) {
    //Change from dynamic data to constant data

    enableDUChainReferenceCounting(data.back().first.data(), data.back().first.size());
    DUChainItemSystem::self().copy(*item.d_func(), target, true);
    Q_ASSERT(!target.isDynamic());
    item.setData(&target);
    disableDUChainReferenceCounting(data.back().first.data());
  }else{
    //Just copy the data into another place, expensive copy constructors are not needed
    memcpy(&target, item.d_func(), size);
    item.setData(&target, false);
  }
  Q_ASSERT(item.d_func() == &target);

  Q_ASSERT(!item.d_func()->isDynamic());
}

}

const char* KDevelop::TopDUContextDynamicData::pointerInData(const QList<ArrayWithPosition>& data, uint totalOffset) {
  
  for(int a = 0; a < data.size(); ++a) {
    if(totalOffset < data[a].second)
      return data[a].first.constData() + totalOffset;
    totalOffset -= data[a].second;
  }
  Q_ASSERT(0); //Offset doesn't exist in the data
  return 0;
}

const char* KDevelop::TopDUContextDynamicData::pointerInData(uint totalOffset) const {
  Q_ASSERT(!m_mappedData || m_data.isEmpty());
  
  if(m_mappedData && m_mappedDataSize)
    return (char*)m_mappedData + totalOffset;
  
  for(int a = 0; a < m_data.size(); ++a) {
    if(totalOffset < m_data[a].second)
      return m_data[a].first.constData() + totalOffset;
    totalOffset -= m_data[a].second;
  }
  Q_ASSERT(0); //Offset doesn't exist in the data
  return 0;
}

void TopDUContextDynamicData::verifyDataInfo(const ItemDataInfo& info, const QList<ArrayWithPosition>& data) {
  Q_UNUSED(info);
  Q_UNUSED(data);
#ifdef DEBUG_DATA_INFO
  DUChainBaseData* item = (DUChainBaseData*)(pointerInData(data, info.dataOffset));
  int size = DUChainItemSystem::self().dynamicSize(*item);
  Q_ASSERT(size);
#endif
}

TopDUContextDynamicData::ItemDataInfo TopDUContextDynamicData::writeDataInfo(const ItemDataInfo& info, const DUChainBaseData* data, uint& totalDataOffset) {
  ItemDataInfo ret(info);
  Q_ASSERT(info.dataOffset);
  int size = DUChainItemSystem::self().dynamicSize(*data);
  Q_ASSERT(size);

  if(m_data.back().first.size() - int(m_data.back().second) < size)
      //Create a new m_data item
      m_data.append( qMakePair(QByteArray(size > 10000 ? size : 10000, 0), 0u) );

  ret.dataOffset = totalDataOffset;

  uint pos = m_data.back().second;
  m_data.back().second += size;
  totalDataOffset += size;

  DUChainBaseData& target(*reinterpret_cast<DUChainBaseData*>(m_data.back().first.data() + pos));
  memcpy(&target, data, size);

  verifyDataInfo(ret, m_data);

  return ret;
}

TopDUContextDynamicData::TopDUContextDynamicData(TopDUContext* topContext)
  : m_deleting(false)
  , m_topContext(topContext)
  , m_onDisk(false)
  , m_dataLoaded(true)
  , m_mappedFile(0)
  , m_mappedData(0)
  , m_mappedDataSize(0)
  , m_itemRetrievalForbidden(false)
{
}

void KDevelop::TopDUContextDynamicData::clearContextsAndDeclarations() {
  //Due to template specialization it's possible that a declaration is not reachable through the normal context structure.
  //For that reason we have to check here, and delete all remaining declarations.
  qDeleteAll(m_temporaryContexts);
  qDeleteAll(m_temporaryDeclarations);
  qDeleteAll(m_contexts);
  qDeleteAll(m_declarations);
  //NOTE: not clearing, is called oly from the dtor anyways
}

TopDUContextDynamicData::~TopDUContextDynamicData() {
  clearContextsAndDeclarations();
  unmap();
}

void KDevelop::TopDUContextDynamicData::unmap() {
  delete m_mappedFile;
  m_mappedFile = 0;
  m_mappedData = 0;
  m_mappedDataSize = 0;
}

QList<IndexedDUContext> TopDUContextDynamicData::loadImporters(uint topContextIndex) {
  QList<IndexedDUContext> ret;

  QString baseDir = globalItemRepositoryRegistry().path() + "/topcontexts";
  QString fileName = baseDir + '/' + QString("%1").arg(topContextIndex);
  QFile file(fileName);
  if(file.open(QIODevice::ReadOnly)) {
     uint readValue;
     file.read((char*)&readValue, sizeof(uint));
     //now readValue is filled with the top-context data size

     //We only read the most needed stuff, not the whole top-context data
     QByteArray data = file.read(readValue);
     const TopDUContextData* topData = reinterpret_cast<const TopDUContextData*>(data.constData());
     FOREACH_FUNCTION(const IndexedDUContext& importer, topData->m_importers)
      ret << importer;
  }

  return ret;
}

QList<IndexedDUContext> TopDUContextDynamicData::loadImports(uint topContextIndex) {
  QList<IndexedDUContext> ret;

  QString baseDir = globalItemRepositoryRegistry().path() + "/topcontexts";
  QString fileName = baseDir + '/' + QString("%1").arg(topContextIndex);
  QFile file(fileName);
  if(file.open(QIODevice::ReadOnly)) {
     uint readValue;
     file.read((char*)&readValue, sizeof(uint));
     //now readValue is filled with the top-context data size

     //We only read the most needed stuff, not the whole top-context data
     QByteArray data = file.read(readValue);
     const TopDUContextData* topData = reinterpret_cast<const TopDUContextData*>(data.constData());
     FOREACH_FUNCTION(const DUContext::Import& import, topData->m_importedContexts)
      ret << import.indexedContext();
  }

  return ret;
}

bool TopDUContextDynamicData::fileExists(uint topContextIndex)
{
  QString baseDir = globalItemRepositoryRegistry().path() + "/topcontexts";
  QString fileName = baseDir + '/' + QString("%1").arg(topContextIndex);
  QFile file(fileName);
  return file.exists();
}

IndexedString TopDUContextDynamicData::loadUrl(uint topContextIndex) {

  QString baseDir = globalItemRepositoryRegistry().path() + "/topcontexts";
  QString fileName = baseDir + '/' + QString("%1").arg(topContextIndex);
  QFile file(fileName);
  if(file.open(QIODevice::ReadOnly)) {
     uint readValue;
     file.read((char*)&readValue, sizeof(uint));
     //now readValue is filled with the top-context data size

     //We only read the most needed stuff, not the whole top-context data
     QByteArray data = file.read(sizeof(TopDUContextData));
     const TopDUContextData* topData = reinterpret_cast<const TopDUContextData*>(data.constData());
     Q_ASSERT(topData->m_url.isEmpty() || topData->m_url.index() >> 16);
     return topData->m_url;
  }

  return IndexedString();
}

void TopDUContextDynamicData::loadData() const {
  //This function has to be protected by an additional mutex, since it can be triggered from multiple threads at the same time
  static QMutex mutex;
  QMutexLocker lock(&mutex);
  if(m_dataLoaded)
    return;

  Q_ASSERT(!m_dataLoaded);
  Q_ASSERT(m_data.isEmpty());
  Q_ASSERT(m_contextDataOffsets.isEmpty());
  Q_ASSERT(m_declarations.isEmpty());

  QString baseDir = globalItemRepositoryRegistry().path() + "/topcontexts";
  KStandardDirs::makeDir(baseDir);

  QString fileName = baseDir + '/' + QString("%1").arg(m_topContext->ownIndex());
  QFile* file = new QFile(fileName);
  bool open = file->open(QIODevice::ReadOnly);
  Q_UNUSED(open);
  Q_ASSERT(open);
  Q_ASSERT(file->size());

  //Skip the offsets, we're already read them
  //Skip top-context data
  uint readValue;
  file->read((char*)&readValue, sizeof(uint));
  file->seek(readValue + file->pos());

  file->read((char*)&readValue, sizeof(uint));
  m_contextDataOffsets.resize(readValue);
  file->read((char*)m_contextDataOffsets.data(), sizeof(ItemDataInfo) * m_contextDataOffsets.size());

  file->read((char*)&readValue, sizeof(uint));
  m_declarationDataOffsets.resize(readValue);
  file->read((char*)m_declarationDataOffsets.data(), sizeof(ItemDataInfo) * m_declarationDataOffsets.size());

#ifdef USE_MMAP
  
  m_mappedData = file->map(file->pos(), file->size() - file->pos());
  if(m_mappedData) {
    m_mappedFile = file;
    m_mappedDataSize = file->size() - file->pos();
    file->close(); //Close the file, so there is less open file descriptors(May be problematic)
  }else{
    kDebug() << "Failed to map" << fileName;
  }
  
#endif
  
  if(!m_mappedFile) {
    QByteArray data = file->readAll();
    m_data.append(qMakePair(data, (uint)data.size()));
    delete file;
  }

  //Fill with zeroes for now, will be initialized on-demand
  m_contexts.resize(m_contextDataOffsets.size());

  m_declarations.resize(m_declarationDataOffsets.size());

  m_dataLoaded = true;
  
}

TopDUContext* TopDUContextDynamicData::load(uint topContextIndex) {
  QString baseDir = globalItemRepositoryRegistry().path() + "/topcontexts";
  KStandardDirs::makeDir(baseDir);

  QString fileName = baseDir + '/' + QString("%1").arg(topContextIndex);
  QFile file(fileName);
  if(file.open(QIODevice::ReadOnly)) {
    if(file.size() == 0) {
      kWarning() << "Top-context file is empty" << fileName;
      return 0;
    }
    QVector<ItemDataInfo> contextDataOffsets;
    QVector<ItemDataInfo> declarationDataOffsets;

    uint readValue;
    file.read((char*)&readValue, sizeof(uint));
    //now readValue is filled with the top-context data size
    QByteArray topContextData = file.read(readValue);

    DUChainBaseData* topData = reinterpret_cast<DUChainBaseData*>(topContextData.data());
/*    IndexedString language = static_cast<TopDUContextData*>(topData)->m_language;
    if(!language.isEmpty()) {*/
      ///@todo Load the language if it isn't loaded yet, problem: We're possibly not in the foreground thread!
//     }
    TopDUContext* ret = dynamic_cast<TopDUContext*>(DUChainItemSystem::self().create(topData));
    if(!ret) {
      kWarning() << "Cannot load a top-context, the requered language-support is probably not loaded";
      return 0;
    }

    //Disable the updating flag on loading. Sometimes it may be set when the context was saved while it was updated
    ret->setFlags( (TopDUContext::Flags) (ret->flags() & (~TopDUContext::UpdatingContext)) );

    TopDUContextDynamicData& target(*ret->m_dynamicData);

//     kDebug() << "loaded" << ret->url().str() << ret->ownIndex() << "import-count:" << ret->importedParentContexts().size() << ret->d_func()->m_importedContextsSize();

    target.m_data.clear();
    target.m_dataLoaded = false;
    target.m_onDisk = true;
    ret->rebuildDynamicData(0, topContextIndex);
    target.m_topContextData.append(qMakePair(topContextData, (uint)0));

//     kDebug() << "loaded" << ret->url().str() << ret->ownIndex() << "import-count:" << ret->importedParentContexts().size() << ret->d_func()->m_importedContextsSize();

    return ret;
  }else{
    return 0;
  }
}

bool TopDUContextDynamicData::isOnDisk() const {
  return m_onDisk;
}

void TopDUContextDynamicData::deleteOnDisk() {
  if(!isOnDisk())
    return;
  kDebug() << "deleting" << m_topContext->ownIndex() << m_topContext->url().str();

  if(!m_dataLoaded)
    loadData();

  for(int a = 0; a < m_contexts.size(); ++a)
    if(m_contexts[a])
      m_contexts[a]->makeDynamic();

  for(int a = 0; a < m_declarations.size(); ++a)
    if(m_declarations[a])
      m_declarations[a]->makeDynamic();

  m_topContext->makeDynamic();

  m_onDisk = false;

  bool successfullyRemoved = QFile::remove(filePath());
  Q_UNUSED(successfullyRemoved);
  Q_ASSERT(successfullyRemoved);
  kDebug() << "deletion ready";
}

QString KDevelop::TopDUContextDynamicData::filePath() const {
  QString baseDir = globalItemRepositoryRegistry().path() + "/topcontexts";
  KStandardDirs::makeDir(baseDir);
  return baseDir + '/' + QString("%1").arg(m_topContext->ownIndex());
}

static uint indexForParentContext(DUContext* context)
{
  return LocalIndexedDUContext(context->parentContext()).localIndex();
}

static uint indexForParentContext(Declaration* declaration)
{
  return LocalIndexedDUContext(declaration->context()).localIndex();
}

void validateItem(const DUChainBase* const item, const uchar* const mappedData, const size_t mappedDataSize)
{
  Q_ASSERT(!item->d_func()->isDynamic());
  if (mappedData) {
    Q_ASSERT(((size_t)item->d_func()) < ((size_t)mappedData)
          || ((size_t)item->d_func()) > ((size_t)mappedData) + mappedDataSize);
  }
}

template<class Item>
void TopDUContextDynamicData::storeData(QVector<ItemDataInfo>& offsets, const QVector<Item*>& items,
                                        uint& currentDataOffset, const QList<ArrayWithPosition>& oldData)
{
  auto const oldOffsets = offsets;
  offsets.clear();
  for (int a = 0; a < items.size(); ++a) {
    auto item = items[a];
    if (!item) {
      if (oldOffsets.size() > a && oldOffsets[a].dataOffset) {
        //Directly copy the old data range into the new data
        const DUChainBaseData* data = nullptr;
        if (m_mappedData) {
          data = reinterpret_cast<const DUChainBaseData*>(m_mappedData + oldOffsets[a].dataOffset);
        } else {
          data = reinterpret_cast<const DUChainBaseData*>(pointerInData(oldData, oldOffsets[a].dataOffset));
        }
        offsets << writeDataInfo(oldOffsets[a], data, currentDataOffset);
      } else {
        offsets << ItemDataInfo();
      }
    } else {
      offsets << ItemDataInfo(currentDataOffset, indexForParentContext(item));
      saveDUChainItem(m_data, *item, currentDataOffset);
      verifyDataInfo(offsets.back(), m_data);

      validateItem(item, m_mappedData, m_mappedDataSize);
    }
  }

#ifndef QT_NO_DEBUG
  for (auto item : items) {
    if (item) {
      validateItem(item, m_mappedData, m_mappedDataSize);
    }
  }
#endif
}

template<typename T>
bool TopDUContextDynamicData::itemsHaveChanged(const QVector<T*>& items) const
{
  for (auto item : items) {
    if (item && item->d_ptr->m_dynamic) {
      return true;
    }
  }
  return false;
}

bool TopDUContextDynamicData::hasChanged() const
{
  return !m_onDisk || m_topContext->d_ptr->m_dynamic || itemsHaveChanged(m_contexts) || itemsHaveChanged(m_declarations);
}

void TopDUContextDynamicData::store() {
//   kDebug() << "storing" << m_topContext->url().str() << m_topContext->ownIndex() << "import-count:" << m_topContext->importedParentContexts().size();

  //Check if something has changed. If nothing has changed, don't store to disk.
  bool contentDataChanged = hasChanged();
  if (!contentDataChanged) {
    return;
  }

    ///@todo Save the meta-data into a repository, and only the actual content data into a file.
    ///      This will make saving+loading more efficient, and will reduce the disk-usage.
    ///      Then we also won't need to load the data if only the meta-data changed.
  if(!m_dataLoaded)
    loadData();
  
  ///If the data is mapped, and we re-write the file, we must make sure that the data is copied out of the map,
  ///even if only metadata is changed.
  ///@todo If we split up data and metadata, we don't need to do this
  if(m_mappedData)
    contentDataChanged = true;

  m_topContext->makeDynamic();
  m_topContextData.clear();
  Q_ASSERT(m_topContext->d_func()->m_ownIndex == m_topContext->ownIndex());

  uint topContextDataSize = DUChainItemSystem::self().dynamicSize(*m_topContext->d_func());
  m_topContextData.append( qMakePair(QByteArray(DUChainItemSystem::self().dynamicSize(*m_topContext->d_func()), topContextDataSize), (uint)0) );
  uint actualTopContextDataSize = 0;

  if (contentDataChanged) {
    //We don't need these structures any more, since we have loaded all the declarations/contexts, and m_data
    //will be reset which these structures pointed into
    //Load all lazy declarations/contexts

    const auto oldData = m_data; //Keep the old data alive until everything is stored into a new data structure

    m_data.clear();

    uint newDataSize = 0;
    foreach(const ArrayWithPosition &array, oldData)
        newDataSize += array.second;

    if(newDataSize < 10000)
      newDataSize = 10000;

    //We always put 1 byte to the front, so we don't have zero data-offsets, since those are used for "invalid".
    uint currentDataOffset = 1;
    m_data.append( qMakePair(QByteArray(newDataSize, 0), currentDataOffset) );

    m_itemRetrievalForbidden = true;

    storeData(m_contextDataOffsets, m_contexts, currentDataOffset, oldData);
    storeData(m_declarationDataOffsets, m_declarations, currentDataOffset, oldData);

    m_itemRetrievalForbidden = false;
  }

    saveDUChainItem(m_topContextData, *m_topContext, actualTopContextDataSize);
    Q_ASSERT(actualTopContextDataSize == topContextDataSize);
    Q_ASSERT(m_topContextData.size() == 1);
    Q_ASSERT(!m_topContext->d_func()->isDynamic());

    unmap();
    
    QFile file(filePath());
    if(file.open(QIODevice::WriteOnly)) {

      file.resize(0);
      
      file.write((char*)&topContextDataSize, sizeof(uint));
      foreach(const ArrayWithPosition& pos, m_topContextData)
        file.write(pos.first.constData(), pos.second);

      uint writeValue = m_contextDataOffsets.size();
      file.write((char*)&writeValue, sizeof(uint));
      file.write((char*)m_contextDataOffsets.data(), sizeof(ItemDataInfo) * m_contextDataOffsets.size());

      writeValue = m_declarationDataOffsets.size();
      file.write((char*)&writeValue, sizeof(uint));
      file.write((char*)m_declarationDataOffsets.data(), sizeof(ItemDataInfo) * m_declarationDataOffsets.size());

      foreach(const ArrayWithPosition& pos, m_data)
        file.write(pos.first.constData(), pos.second);

      m_onDisk = true;

      if (file.size() == 0) {
        kWarning() << "Saving zero size top ducontext data";
      }
      file.close();
    } else {
      kWarning() << "Cannot open top-context for writing";
    }
//   kDebug() << "stored" << m_topContext->url().str() << m_topContext->ownIndex() << "import-count:" << m_topContext->importedParentContexts().size();
}

uint TopDUContextDynamicData::allocateDeclarationIndex(Declaration* decl, bool temporary) {
  if(!m_dataLoaded)
    loadData();
  if(!temporary) {
    m_declarations.append(decl);
    return m_declarations.size();
  }else{
    QMutexLocker lock(&m_temporaryDataMutex);
    m_temporaryDeclarations.append(decl);
    return 0x0fffffff - m_temporaryDeclarations.size(); //We always keep the highest bit at zero
  }
}

bool TopDUContextDynamicData::isDeclarationForIndexLoaded(uint index) const {
  if(!m_dataLoaded)
    return false;
  if(index < (0x0fffffff/2)) {
    if(index == 0 || index > uint(m_declarations.size()))
      return false;
    return (bool)m_declarations[index-1];
  }else{
    return true;
  }
}

bool TopDUContextDynamicData::isContextForIndexLoaded(uint index) const {
  if(!m_dataLoaded)
    return false;
  if(index < (0x0fffffff/2)) {
    if(index == 0)
      return true;
    if(index > uint(m_contexts.size()))
      return false;
    return (bool)m_contexts[index-1];
  }else{
    return true;
  }
}

Declaration* TopDUContextDynamicData::getDeclarationForIndex(uint index) const {
  if(!m_dataLoaded)
    loadData();

  if(index < (0x0fffffff/2)) {
    if(index == 0 || index > uint(m_declarations.size())) {
      kWarning() << "declaration index out of bounds:" << index << "count:" << m_declarations.size();
      return 0;
    }
    else {
      uint realIndex = index-1;
      if(!m_declarations[realIndex] && realIndex < (uint)m_declarationDataOffsets.size() && m_declarationDataOffsets[realIndex].dataOffset) {
        
        Q_ASSERT(!m_itemRetrievalForbidden);
        
        const DUChainBaseData* data = reinterpret_cast<const DUChainBaseData*>(
          pointerInData(m_declarationDataOffsets[realIndex].dataOffset));
        ///FIXME: ugly, remove const_cast
        m_declarations[realIndex] = static_cast<Declaration*>(DUChainItemSystem::self().create(
          const_cast<DUChainBaseData*>(data)));
        if(!m_declarations[realIndex]) {
          //When this happens, the declaration has not been registered correctly.
          //We can stop here, because else we will get crashes later.
          kError() << "Failed to load declaration with identity" << data->classId;
          Q_ASSERT(0);
        }else{
          DUContext* context = getContextForIndex(m_declarationDataOffsets[realIndex].parentContext);
          Q_ASSERT(context); //If this triggers, the context has been deleted without deleting its contained declarations
          m_declarations[realIndex]->rebuildDynamicData(context, index);
        }
      }

      return m_declarations[realIndex];
    }
  }else{
    QMutexLocker lock(&m_temporaryDataMutex);
    index = 0x0fffffff - index; //We always keep the highest bit at zero
    if(index == 0 || index > uint(m_temporaryDeclarations.size()))
      return 0;
    else
      return m_temporaryDeclarations[index-1];
  }
}

void TopDUContextDynamicData::clearDeclarationIndex(Declaration* decl) {
  if(!m_dataLoaded)
    loadData();

  uint index = decl->m_indexInTopContext;
  if(index < (0x0fffffff/2)) {
    if(index == 0 || index > uint(m_declarations.size()))
      return;
    else {
      Q_ASSERT(m_declarations[index-1] == decl);
      m_declarations[index-1] = 0;

      if(index-1 < (uint)m_declarationDataOffsets.size())
        m_declarationDataOffsets[index-1] = ItemDataInfo();
    }
  }else{
    QMutexLocker lock(&m_temporaryDataMutex);
    index = 0x0fffffff - index; //We always keep the highest bit at zero
    if(index == 0 || index > uint(m_temporaryDeclarations.size()))
      return;
    else {
      Q_ASSERT(m_temporaryDeclarations[index-1] == decl);
      m_temporaryDeclarations[index-1] = 0;
    }
  }
}

uint TopDUContextDynamicData::allocateContextIndex(DUContext* decl, bool temporary) {
  if(!m_dataLoaded)
    loadData();

  if(!temporary) {
    m_contexts.append(decl);
    return m_contexts.size();
  }else{
    QMutexLocker lock(&m_temporaryDataMutex);
    m_temporaryContexts.append(decl);
    return 0x0fffffff - m_temporaryContexts.size(); //We always keep the highest bit at zero
  }
}

bool TopDUContextDynamicData::isTemporaryContextIndex(uint index) const {
  return !(index < (0x0fffffff/2));
}

bool TopDUContextDynamicData::isTemporaryDeclarationIndex(uint index) const {
  return !(index < (0x0fffffff/2));
}

DUContext* TopDUContextDynamicData::getContextForIndex(uint index) const {

  if(!m_dataLoaded)
    loadData();

  if(index < (0x0fffffff/2)) {
    if(index == 0)
      return m_topContext;
    if(index > uint(m_contexts.size())) {
      kWarning() << "declaration index out of bounds:" << index << "count:" << m_declarations.size();
      return 0;
    } else {
      uint realIndex = index-1;
      DUContext** fastContextsPos = (m_contexts.data() + realIndex);
      if(*fastContextsPos) //Shortcut, because this is the most common case
        return *fastContextsPos;

      if(!*fastContextsPos && realIndex < (uint)m_contextDataOffsets.size() && m_contextDataOffsets[realIndex].dataOffset) {

        Q_ASSERT(!m_itemRetrievalForbidden);
        
        //Construct the context, and eventuall its parent first
        ///TODO: ugly, remove const_cast
        const DUChainBaseData* data = reinterpret_cast<const DUChainBaseData*>(
          pointerInData(m_contextDataOffsets[realIndex].dataOffset));
        *fastContextsPos = dynamic_cast<DUContext*>(DUChainItemSystem::self().create(const_cast<DUChainBaseData*>(data)));
        if(!*fastContextsPos) {
          //When this happens, the declaration has not been registered correctly.
          //We can stop here, because else we will get crashes later.
          kError() << "Failed to load declaration with identity" << data->classId;
        }else{
          (*fastContextsPos)->rebuildDynamicData(getContextForIndex(m_contextDataOffsets[realIndex].parentContext), index);
        }
      }

      return *fastContextsPos;
    }
  }else{
    QMutexLocker lock(&m_temporaryDataMutex);
    index = 0x0fffffff - index; //We always keep the highest bit at zero
    if(index == 0 || index > uint(m_temporaryContexts.size()))
      return 0;
    else
      return m_temporaryContexts[index-1];
  }
}

void TopDUContextDynamicData::clearContextIndex(DUContext* decl) {

  if(!m_dataLoaded)
    loadData();

  uint index = decl->m_dynamicData->m_indexInTopContext;
  if(index < (0x0fffffff/2)) {

    if(index == 0 || index > uint(m_contexts.size()))
      return;
    else {
      Q_ASSERT(m_contexts[index-1] == decl);
      m_contexts[index-1] = 0;

      if(index-1 < (uint)m_contextDataOffsets.size())
        m_contextDataOffsets[index-1] = ItemDataInfo();
    }
  }else{
    QMutexLocker lock(&m_temporaryDataMutex);
    index = 0x0fffffff - index; //We always keep the highest bit at zero
    if(index == 0 || index > uint(m_temporaryContexts.size()))
      return;
    else {
      Q_ASSERT(m_temporaryContexts[index-1] == decl);
      m_temporaryContexts[index-1] = 0;
    }
  }
}
