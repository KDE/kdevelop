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
#include <ktexteditor/smartinterface.h>
#include <editor/editorintegrator.h>
#include <editor/hashedstring.h>
#include "indexedstring.h"
#include <hash_map>

namespace KDevelop {

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

struct FileModificationCache {
  QDateTime m_readTime;
  QDateTime m_modificationTime;
};
#ifdef Q_CC_MSVC
    typedef stdext::hash_map<KDevelop::IndexedString, FileModificationCache, IndexedStringHash> FileModificationMap;
#else    
    typedef __gnu_cxx::hash_map<KDevelop::IndexedString, FileModificationCache, IndexedStringHash> FileModificationMap;
#endif
  
QDateTime fileModificationTimeCached( const IndexedString& fileName ) {
  static FileModificationMap fileModificationCache;
  static QDateTime currentDateTime = QDateTime::currentDateTime();

  FileModificationMap::const_iterator it = fileModificationCache.find( fileName );
  if( it != fileModificationCache.end() ) {
    ///Use the cache for 10 seconds
    if( (*it).second.m_readTime.secsTo( currentDateTime ) < 10 ) {
      return (*it).second.m_modificationTime;
    }
  }

  ///@todo support non-local files
  QFileInfo fileInfo( fileName.str() );
  fileModificationCache[fileName].m_readTime = QDateTime::currentDateTime();
  fileModificationCache[fileName].m_modificationTime = fileInfo.lastModified();
  return fileInfo.lastModified();

}

ModificationRevision ModificationRevision::revisionForFile(const IndexedString& url) {

  ModificationRevision ret(fileModificationTimeCached(url));

  ///@todo prevent this conversion from IndexedString to HashedString
  KTextEditor::Document* doc = EditorIntegrator::documentForUrl(HashedString(url.str()));
  if( doc ) {
    KTextEditor::SmartInterface* smart =   dynamic_cast<KTextEditor::SmartInterface*>(doc);
    if( smart )
      ret.revision = smart->currentRevision();
  }
  
  return ret;
}

ModificationRevision::ModificationRevision( const QDateTime& modTime , int revision_ ) : modificationTime(modTime), revision(revision_) {
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
  return QString("%1 (rev %2)").arg(modificationTime.time().toString()).arg(revision);
}

}

kdbgstream& operator<< (kdbgstream& s, const KDevelop::ModificationRevision& rev) {
  s << rev.toString();
  return s;
}
