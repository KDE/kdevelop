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

#include "modificationrevision.h"

#include <QString>
#include <QFileInfo>

#include <ktexteditor/document.h>

#if defined(Q_CC_MSVC)
#include <hash_map>
using namespace stdext;
#elif defined GXX_LT_4_3
#include <ext/hash_map>
using namespace __gnu_cxx;
#else // C++0X
// TODO: Replace hash_map with unordered map when support for G++ < 4.3 has 
//       ended. This class was added as a temporary workaround, to get rid of
//       hash_map related warnings for g++ >= 4.3.
#include <unordered_map>
template<class _Key, class _Tp,
       class _Hash = std::hash<_Key>,
       class _Pred = std::equal_to<_Key>,
       class _Alloc = std::allocator<std::pair<const _Key, _Tp> > >
class  hash_map : public std::unordered_map<_Key, _Tp, _Hash, _Pred, _Alloc> { };
#endif


#include "../duchain/indexedstring.h"
#include "modificationrevisionset.h"
#include <sys/time.h>

///@todo Listen to filesystem changes (together with the project manager) and call fileModificationCache().clear(...) when a file has changed

namespace KDevelop {

const int cacheModificationTimesForSeconds = 30;

struct IndexedStringHash {
  uint operator() (const KDevelop::IndexedString& str) const {
    return str.hash();
  }
  
  #ifdef Q_CC_MSVC
  bool operator() (const KDevelop::IndexedString& lhs, const KDevelop::IndexedString& rhs) const {
    return lhs < rhs;
  }
    enum
        {   // parameters for hash table
        bucket_size = 4,    // 0 < bucket_size
        min_buckets = 8};   // min_buckets = 2 ^^ N, 0 < N
  #endif
};

QMutex fileModificationTimeCacheMutex(QMutex::Recursive);

struct FileModificationCache {
  timeval m_readTime;
  QDateTime m_modificationTime;
};

typedef hash_map<KDevelop::IndexedString, FileModificationCache, IndexedStringHash> FileModificationMap;

FileModificationMap& fileModificationCache() {
  static FileModificationMap cache;
  return cache;
}

typedef hash_map<KDevelop::IndexedString, int, IndexedStringHash> OpenDocumentRevisionsMap;

OpenDocumentRevisionsMap& openDocumentsRevisionMap() {
  static OpenDocumentRevisionsMap map;
  return map;
}

QDateTime fileModificationTimeCached( const IndexedString& fileName ) {
  
  timeval currentTime;
  gettimeofday(&currentTime, 0);
  
  FileModificationMap::const_iterator it = fileModificationCache().find( fileName );
  if( it != fileModificationCache().end() ) {
    ///Use the cache for X seconds
    timeval  age;
    timersub(&currentTime, &(*it).second.m_readTime, &age);
    if( age.tv_sec < cacheModificationTimesForSeconds )
      return (*it).second.m_modificationTime;
  }

  QFileInfo fileInfo( fileName.str() );
  fileModificationCache()[fileName].m_readTime = currentTime;
  fileModificationCache()[fileName].m_modificationTime = fileInfo.lastModified();
  return fileInfo.lastModified();
}

void ModificationRevision::clearModificationCache(const IndexedString& fileName) {
  ModificationRevisionSet::clearCache(); ///@todo Make the cache management more clever (don't clear the whole)
  
  QMutexLocker lock(&fileModificationTimeCacheMutex);

  FileModificationMap::iterator it = fileModificationCache().find(fileName);
  if(it != fileModificationCache().end())
    fileModificationCache().erase(it);
}

ModificationRevision ModificationRevision::revisionForFile(const IndexedString& url) {

  QMutexLocker lock(&fileModificationTimeCacheMutex);
  
  ModificationRevision ret(fileModificationTimeCached(url));

  OpenDocumentRevisionsMap::const_iterator it = openDocumentsRevisionMap().find(url);
  if(it != openDocumentsRevisionMap().end())
    ret.revision = (*it).second;
  
  return ret;
}

void ModificationRevision::clearEditorRevisionForFile(const KDevelop::IndexedString& url)
{
  ModificationRevisionSet::clearCache(); ///@todo Make the cache management more clever (don't clear the whole)
  
  QMutexLocker lock(&fileModificationTimeCacheMutex);
  if(openDocumentsRevisionMap().find(url) != openDocumentsRevisionMap().end())
    openDocumentsRevisionMap().erase(url);
}

void ModificationRevision::setEditorRevisionForFile(const KDevelop::IndexedString& url, int revision)
{
  ModificationRevisionSet::clearCache(); ///@todo Make the cache management more clever (don't clear the whole)
  
  QMutexLocker lock(&fileModificationTimeCacheMutex);
  openDocumentsRevisionMap().erase(url);
  openDocumentsRevisionMap().insert(std::make_pair(url, revision));
  Q_ASSERT(revisionForFile(url).revision == revision);
}

ModificationRevision::ModificationRevision( const QDateTime& modTime , int revision_ ) : modificationTime(modTime.toTime_t()), revision(revision_) {
}

bool ModificationRevision::operator <( const ModificationRevision& rhs ) const {
  return modificationTime < rhs.modificationTime || (modificationTime == rhs.modificationTime && revision < rhs.revision);
}

bool ModificationRevision::operator ==( const ModificationRevision& rhs ) const {
  return modificationTime == rhs.modificationTime && revision == rhs.revision;
}

bool ModificationRevision::operator !=( const ModificationRevision& rhs ) const {
  return modificationTime != rhs.modificationTime || revision != rhs.revision;
}

QString ModificationRevision::toString() const {
  return QString("%1 (rev %2)").arg(QDateTime::fromTime_t(modificationTime).time().toString()).arg(revision);
}

}

kdbgstream& operator<< (kdbgstream& s, const KDevelop::ModificationRevision& rev) {
  s << rev.toString();
  return s;
}
