/*
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

#include "./modificationrevisionset.h"
#include "../duchain/repositories/itemrepository.h"
#include <duchain/indexedstring.h>
#include <util/setrepository.h>
#include <time.h>

//When uncommented, the reason for needed updates is printed
// #define DEBUG_NEEDSUPDATE
#include <sys/time.h>

namespace KDevelop {

QMutex modificationRevisionSetMutex(QMutex::Recursive);
  
struct FileModificationPair {
  KDevelop::IndexedString file;
  KDevelop::ModificationRevision revision;

  FileModificationPair() {
  }
  
  FileModificationPair(KDevelop::IndexedString _file, KDevelop::ModificationRevision _revision) : file(_file), revision(_revision) {
  }
  
  unsigned int hash() const {
    return ((file.hash() + revision.modificationTime) * 17 + revision.revision) * 73;
  }
  
  unsigned short int itemSize() const {
    return sizeof(FileModificationPair);
  }
  
  bool operator==(const FileModificationPair& rhs) const {
    return file == rhs.file && revision == rhs.revision;
  }
};

struct FileModificationPairRequest {

  FileModificationPairRequest(const FileModificationPair& data) : m_data(data) {
  }
  
  const FileModificationPair& m_data;
  
  enum {
    AverageSize = sizeof(FileModificationPair)
  };

  unsigned int hash() const {
    return m_data.hash();
  }
  
  size_t itemSize() const {
      return m_data.itemSize();
  }

  void createItem(FileModificationPair* item) const {
    new (item)  FileModificationPair(m_data);
  }
  
  bool equals(const FileModificationPair* item) const {
    return *item == m_data;
  }
  
  static void destroy(FileModificationPair* item, KDevelop::AbstractItemRepository&) {
    item->~FileModificationPair();
  }
  
  static bool persistent(const FileModificationPair* /*item*/) {
    return true; //Reference-counting is done implicitly using the set-repository
  }
};

typedef KDevelop::ItemRepository<FileModificationPair, FileModificationPairRequest, true, false> FileModificationPairRepository;

static FileModificationPairRepository& fileModificationPairRepository() {
  static FileModificationPairRepository rep("file modification repository");
  rep.setMutex(&modificationRevisionSetMutex);
  return rep;
}

void initModificationRevisionSetRepository() {
  fileModificationPairRepository();
}

QHash<uint, std::pair<timeval, bool> > needsUpdateCache;

void ModificationRevisionSet::clearCache() {
  QMutexLocker lock(&modificationRevisionSetMutex);
  ///@todo More intelligent clearing. We actually need to watch the directory for changes, and if there are changes, clear the cache.
  needsUpdateCache.clear();
}

struct FileModificationSetRepository : public Utils::BasicSetRepository {
  FileModificationSetRepository() : Utils::BasicSetRepository("file modification sets", &globalItemRepositoryRegistry(), true) {
  }
  virtual void itemRemovedFromSets(uint index);
};

//FileModificationSetRepository fileModificationSetRepository;

struct FileModificationSetRepositoryRepresenter {
  static FileModificationSetRepository& repository() {
    static FileModificationSetRepository fileModificationSetRepository;
    return fileModificationSetRepository;
  }
};


ModificationRevisionSet::ModificationRevisionSet(unsigned int index) : m_index(index) {
}

uint ModificationRevisionSet::size() const {
  Utils::Set set = Utils::Set(m_index, &FileModificationSetRepositoryRepresenter::repository());
  return set.count();
}

void ModificationRevisionSet::clear() {
  QMutexLocker lock(&modificationRevisionSetMutex);
  
  if(m_index) {
    Utils::Set oldModificationTimes = Utils::Set(m_index, &FileModificationSetRepositoryRepresenter::repository());
    oldModificationTimes.staticUnref();
    m_index = 0;
  }
}

void ModificationRevisionSet::addModificationRevision(const IndexedString& url, const KDevelop::ModificationRevision& revision) {
  QMutexLocker lock(&modificationRevisionSetMutex);
  
  if(m_index == 0) {
    Utils::Set set = FileModificationSetRepositoryRepresenter::repository().createSet(fileModificationPairRepository().index(FileModificationPair(url, revision)));
    set.staticRef();
    m_index = set.setIndex();
  }else{
    Utils::Set oldModificationTimes = Utils::Set(m_index, &FileModificationSetRepositoryRepresenter::repository());
    Utils::Set newModificationTimes = oldModificationTimes;
    
    Utils::Set tempSet = FileModificationSetRepositoryRepresenter::repository().createSet(fileModificationPairRepository().index(FileModificationPair(url, revision)));
    tempSet.staticRef();
    
    newModificationTimes += tempSet;
    newModificationTimes.staticRef();
    oldModificationTimes.staticUnref();
    tempSet.staticUnref();
    
    m_index = newModificationTimes.setIndex();
  }
}

bool ModificationRevisionSet::removeModificationRevision(const IndexedString& url, const KDevelop::ModificationRevision& revision) {
  QMutexLocker lock(&modificationRevisionSetMutex);
  
  if(!m_index)
    return false;

  Utils::Set oldModificationTimes = Utils::Set(m_index, &FileModificationSetRepositoryRepresenter::repository());
  Utils::Set newModificationTimes = oldModificationTimes;
  
  Utils::Set tempSet = FileModificationSetRepositoryRepresenter::repository().createSet(fileModificationPairRepository().index(FileModificationPair(url, revision)));
  tempSet.staticRef();
  
  newModificationTimes -= tempSet;
  newModificationTimes.staticRef();
  oldModificationTimes.staticUnref();
  tempSet.staticUnref();
  
  m_index = newModificationTimes.setIndex();
  return m_index != oldModificationTimes.setIndex();
}

// const QMap<IndexedString, KDevelop::ModificationRevision> ModificationRevisionSet::allModificationTimes() const {
//   QMap<IndexedString, KDevelop::ModificationRevision> ret;
//   Utils::Set::Iterator it = m_allModificationTimes.iterator();
//   while(it) {
//     const FileModificationPair* data = fileModificationPairRepository().itemFromIndex(*it);
//     ret[data->file] = data->revision;
//     ++it;
//   }
//   return ret;
// }

 typedef Utils::VirtualSetNode<uint, Utils::IdentityConversion<uint>, FileModificationSetRepositoryRepresenter> ModificationRevisionSetNode;
// static bool (const Utils::SetNodeData* node) {
//   ModificationRevisionSetNode
//   if(!node)
//     return false;
// }

static bool nodeNeedsUpdate(uint index) {
  QMutexLocker lock(&modificationRevisionSetMutex);
  
  if(!index)
    return false;
  
  timeval currentTime;
  gettimeofday(&currentTime, 0);
  
  QHash<uint, std::pair<timeval, bool> >::const_iterator cached = needsUpdateCache.constFind(index);
  if(cached != needsUpdateCache.constEnd()) {
    
    timeval  age;
    timersub(&currentTime, &(*cached).first, &age);
    
    if( age.tv_sec < cacheModificationTimesForSeconds )
    {
      return cached->second;
    }
  }
  
  bool result = false;
  
  const Utils::SetNodeData* nodeData = FileModificationSetRepositoryRepresenter::repository().nodeFromIndex(index);
  if(nodeData->contiguous()) {
    //Do  the actual checking
    for(unsigned int a = nodeData->start(); a < nodeData->end(); ++a) {
      const FileModificationPair* data = fileModificationPairRepository().itemFromIndex(a);
      ModificationRevision revision = KDevelop::ModificationRevision::revisionForFile( data->file );
      if( revision != data->revision ) {
        result = true;
        break;
      }
    }
  }else{
    result = nodeNeedsUpdate(nodeData->leftNode()) || nodeNeedsUpdate(nodeData->rightNode());
  }
  
  needsUpdateCache.insert(index, std::make_pair(currentTime, result));
  
  return result;
}


QString ModificationRevisionSet::toString() const
{
  QMutexLocker lock(&modificationRevisionSetMutex);
  QString ret = "["; // krazy:exclude=doublequote_chars
  Utils::Set set(m_index, &FileModificationSetRepositoryRepresenter::repository());
  Utils::Set::Iterator it = set.iterator();
  bool first = true;
  while(it) {
    if(!first)
      ret += ", ";
    first = false;
    
    const FileModificationPair* data = fileModificationPairRepository().itemFromIndex(*it);
    ret += data->file.str() + ':' + data->revision.toString();
    ++it;
  }

  ret += ']';
  return ret;
}

bool ModificationRevisionSet::needsUpdate() const {
  QMutexLocker lock(&modificationRevisionSetMutex);
  
  #ifdef DEBUG_NEEDSUPDATE
  Utils::Set set(m_index, &FileModificationSetRepositoryRepresenter::repository());
  Utils::Set::Iterator it = set.iterator();
  while(it) {
    const FileModificationPair* data = fileModificationPairRepository().itemFromIndex(*it);
    ModificationRevision revision = KDevelop::ModificationRevision::revisionForFile( data->file );
    if( revision != data->revision ) {
       kDebug() << "dependency" << data->file.str() << "has changed, stored stamp:" << data->revision << "new time:" << revision ;
       return true;
    }
    ++it;
  }
  return false;
  #else
  return nodeNeedsUpdate(m_index);
  #endif
}

ModificationRevisionSet& ModificationRevisionSet::operator+=(const ModificationRevisionSet& rhs) {
  QMutexLocker lock(&modificationRevisionSetMutex);
  
  Utils::Set oldModificationTimes = Utils::Set(m_index, &FileModificationSetRepositoryRepresenter::repository());
  Utils::Set otherModificationTimes = Utils::Set(rhs.m_index, &FileModificationSetRepositoryRepresenter::repository());
  
  Utils::Set newModificationTimes = oldModificationTimes;
  
  newModificationTimes += otherModificationTimes;
  newModificationTimes.staticRef();
  oldModificationTimes.staticUnref();
  
  m_index = newModificationTimes.setIndex();
  
  return *this;
}

ModificationRevisionSet& ModificationRevisionSet::operator-=(const ModificationRevisionSet& rhs) {
  QMutexLocker lock(&modificationRevisionSetMutex);
  
  Utils::Set oldModificationTimes = Utils::Set(m_index, &FileModificationSetRepositoryRepresenter::repository());
  Utils::Set otherModificationTimes = Utils::Set(rhs.m_index, &FileModificationSetRepositoryRepresenter::repository());
  
  Utils::Set newModificationTimes = oldModificationTimes;
  
  newModificationTimes -= otherModificationTimes;
  newModificationTimes.staticRef();
  oldModificationTimes.staticUnref();
  
  m_index = newModificationTimes.setIndex();

  return *this;
}

void FileModificationSetRepository::itemRemovedFromSets(uint index)
{
    fileModificationPairRepository().deleteItem(index);
    needsUpdateCache.remove(index);
}

}
