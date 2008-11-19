/*
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

//When uncommented, the reason for needed updates is printed
#define DEBUG_NEEDSUPDATE

namespace KDevelop {

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
    *item = m_data;
  }
  
  bool equals(const FileModificationPair* item) const {
    return *item == m_data;
  }
};

typedef KDevelop::ItemRepository<FileModificationPair, FileModificationPairRequest, KDevelop::NoDynamicData, false> FileModificationPairRepository;

static FileModificationPairRepository& fileModificationPairRepository() {
  static FileModificationPairRepository rep("file modification repository");
  return rep;
}

struct FileModificationSetRepository : public Utils::BasicSetRepository {
  FileModificationSetRepository() : Utils::BasicSetRepository("file modification sets", true) {
  }
  virtual void itemRemovedFromSets(uint index) {
    kDebug() << "item removed from file-modification repository:" << index;
    fileModificationPairRepository().deleteItem(index);
  }
};

FileModificationSetRepository fileModificationSetRepository;

ModificationRevisionSet::ModificationRevisionSet(unsigned int index) : m_index(index) {

}

void ModificationRevisionSet::clear() {
  if(m_index) {
    Utils::Set oldModificationTimes = Utils::Set(m_index, &fileModificationSetRepository);
    oldModificationTimes.staticUnref();
    m_index = 0;
  }
}

void ModificationRevisionSet::addModificationRevision(const IndexedString& url, const KDevelop::ModificationRevision& revision) {
  if(m_index == 0) {
    Utils::Set set = fileModificationSetRepository.createSet(fileModificationPairRepository().index(FileModificationPair(url, revision)));
    set.staticRef();
    m_index = set.setIndex();
  }else{
    Utils::Set oldModificationTimes = Utils::Set(m_index, &fileModificationSetRepository);
    Utils::Set newModificationTimes = oldModificationTimes;
    
    Utils::Set tempSet = fileModificationSetRepository.createSet(fileModificationPairRepository().index(FileModificationPair(url, revision)));
    tempSet.staticRef();
    
    newModificationTimes += tempSet;
    newModificationTimes.staticRef();
    oldModificationTimes.staticUnref();
    tempSet.staticUnref();
    
    m_index = newModificationTimes.setIndex();
  }
}

bool ModificationRevisionSet::removeModificationRevision(const IndexedString& url, const KDevelop::ModificationRevision& revision) {
  if(!m_index)
    return false;
  
  Utils::Set oldModificationTimes = Utils::Set(m_index, &fileModificationSetRepository);
  Utils::Set newModificationTimes = oldModificationTimes;
  
  Utils::Set tempSet = fileModificationSetRepository.createSet(fileModificationPairRepository().index(FileModificationPair(url, revision)));
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

bool ModificationRevisionSet::needsUpdate() const {

  Utils::Set set(m_index, &fileModificationSetRepository);
  
  ///@todo Do caching for complete sets!
  Utils::Set::Iterator it = set.iterator();
  while(it) {
    const FileModificationPair* data = fileModificationPairRepository().itemFromIndex(*it);
    ModificationRevision revision = KDevelop::ModificationRevision::revisionForFile( data->file );
    if( revision != data->revision ) {
#ifdef DEBUG_NEEDSUPDATE
      kDebug( 9007 ) << "dependency" << data->file.str() << "has changed, stored stamp:" << data->revision << "new time:" << revision ;
#endif
      
      return true;
    }

    ++it;
  }
  
  return false;
}

ModificationRevisionSet& ModificationRevisionSet::operator+=(const ModificationRevisionSet& rhs) {
  Utils::Set oldModificationTimes = Utils::Set(m_index, &fileModificationSetRepository);
  Utils::Set otherModificationTimes = Utils::Set(rhs.m_index, &fileModificationSetRepository);
  
  Utils::Set newModificationTimes = oldModificationTimes;
  
  newModificationTimes += otherModificationTimes;
  newModificationTimes.staticRef();
  oldModificationTimes.staticUnref();
  
  m_index = newModificationTimes.setIndex();
  
  return *this;
}

ModificationRevisionSet& ModificationRevisionSet::operator-=(const ModificationRevisionSet& rhs) {
  Utils::Set oldModificationTimes = Utils::Set(m_index, &fileModificationSetRepository);
  Utils::Set otherModificationTimes = Utils::Set(rhs.m_index, &fileModificationSetRepository);
  
  Utils::Set newModificationTimes = oldModificationTimes;
  
  newModificationTimes -= otherModificationTimes;
  newModificationTimes.staticRef();
  oldModificationTimes.staticUnref();
  
  m_index = newModificationTimes.setIndex();
  
  return *this;
}

}
