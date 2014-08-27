/***************************************************************************
   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef KDEVPLATFORM_INDEXED_STRING_H
#define KDEVPLATFORM_INDEXED_STRING_H

//krazy:excludeall=dpointer,inline

#include <QtCore/QMetaType>

#include "referencecounting.h"

#include "serializationexport.h"

class KUrl;

namespace KDevelop {

/**
 * This string does "disk reference-counting", which means that reference-counts are maintainted,
 * but only when the string is in a disk-stored location. The file referencecounting.h is used
 * to manage this condition.
 *
 * Whenever reference-counting is enabled for a range that contains the IndexedString, it will
 * manipulate the reference-counts.
 *
 * The duchain storage mechanisms automatically are about correctly managing that condition,
 * so you don't need to care, and can just use this class in every duchain data type without
 * restrictions.
 *
 * @warning Do not use IndexedString after QCoreApplication::aboutToQuit() has been emitted,
 * items that are not disk-referenced will be invalid at that point.
 *
 * @note Empty strings have an index of zero.
 *
 * @note Strings of length one are not put into the repository, but are encoded directly within
 * the index: They are encoded like @c 0xffff00bb where @c bb is the byte of the character.
 */
class KDEVPLATFORMSERIALIZATION_EXPORT IndexedString {
 public:
  IndexedString();
  /**
   * @param str must be a utf8 encoded string, does not need to be 0-terminated.
   * @param length must be its length in bytes.
   * @param hash must be a hash as constructed with the here defined hash functions.
   *             If it is zero, it will be computed.
   */
  explicit IndexedString( const char* str, unsigned short length, unsigned int hash = 0 );

  /**
   * Needs a zero terminated string. When the information is already available,
   * try using the other constructor.
   *
   * WARNING There is a UTF8-related issue when attempting to retrieve the string
   * using str from an IndexedString built from this constructor
   */
  explicit IndexedString( const char* str );

  explicit IndexedString( char c );

  /**
   * When the information is already available, try using the other constructor.
   *
   * @note This is expensive.
   */
  explicit IndexedString( const QString& str );

  /**
   * When the information is already available, try using the other constructor.
   *
   * @note This is expensive.
   */
  explicit IndexedString( const QByteArray& str );

  IndexedString( IndexedString&& o ) Q_DECL_NOEXCEPT
    : m_index(o.m_index)
  {
    o.m_index = 0;
  }

  /**
   * Returns a not reference-counted IndexedString that represents the given index.
   *
   * @warning It is dangerous dealing with indices directly, because it may break
   *          the reference counting logic. Never store pure indices to disk.
   */
  static IndexedString fromIndex( unsigned int index ) {
    IndexedString ret;
    ret.m_index = index;
    return ret;
  }

  /**
   * @warning This is relatively expensive: needs a mutex lock, hash lookups, and eventual loading,
   *       so avoid it when possible.
   */
  static int lengthFromIndex(unsigned int index);

  IndexedString( const IndexedString& );

  ~IndexedString();

  /**
   * Creates an indexed string from a KUrl, this is expensive.
   */
  explicit IndexedString( const KUrl& url );

  /**
   * Re-construct a KUrl from this indexed string, the result can be used with the
   * KUrl-using constructor.
   *
   * @note This is expensive.
   */
  KUrl toUrl() const;

  inline unsigned int hash() const {
    return m_index;
  }

  /**
   * The string is uniquely identified by this index. You can use it for comparison.
   *
   * @warning It is dangerous dealing with indices directly, because it may break the
   *          reference counting logic. never store pure indices to disk
   */
  inline unsigned int index() const {
    return m_index;
  }

  bool isEmpty() const {
    return m_index == 0;
  }

  /**
   * @note This is relatively expensive: needs a mutex lock, hash lookups, and eventual loading,
   * so avoid it when possible.
   */
  int length() const;

  /**
   * Returns the underlying c string, in utf-8 encoding.
   *
   * @warning The string is not 0-terminated, consider length()!
   */
  const char* c_str() const;

  /**
   * Convenience function, avoid using it, it's relatively expensive
   */
  QString str() const;

  /**
   * Convenience function, avoid using it, it's relatively expensive (less expensive then str() though)
   */
  QByteArray byteArray() const;

  IndexedString& operator=(const IndexedString&);

  IndexedString& operator=(IndexedString&& o) Q_DECL_NOEXCEPT
  {
    m_index = o.m_index;
    o.m_index = 0;
    return *this;
  }

  /**
   * Fast index-based comparison
   */
  bool operator == ( const IndexedString& rhs ) const {
    return m_index == rhs.m_index;
  }

  /**
   * Fast index-based comparison
   */
  bool operator != ( const IndexedString& rhs ) const {
    return m_index != rhs.m_index;
  }

  /**
   * Does not compare alphabetically, uses the index for ordering.
   */
  bool operator < ( const IndexedString& rhs ) const {
    return m_index < rhs.m_index;
  }

  /**
   * Use this to construct a hash-value on-the-fly
   *
   * To read it, just use the hash member, and when a new string is started, call @c clear().
   *
   * This needs very fast performance(per character operation), so it must stay inlined.
   */
  struct RunningHash {
    enum {
      HashInitialValue = 5381
    };

    RunningHash() : hash(HashInitialValue) { //We initialize the hash with zero, because we want empty strings to create a zero hash(invalid)
    }
    inline void append(const char c) {
      hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    inline void clear() {
      hash = HashInitialValue;
    }
    unsigned int hash;
  };

  static unsigned int hashString(const char* str, unsigned short length);

  /**
   * Optimized function that only computes the index of a string
   * removes the overhead of the IndexedString ref counting
   */
  static uint indexForString(const char* str, unsigned short length, uint hash = 0);
  static uint indexForString(const QString& str, uint hash = 0);

 private:
   explicit IndexedString(bool);
   uint m_index;
};

// the following function would need to be exported in case you'd remove the inline keyword.
inline uint qHash( const KDevelop::IndexedString& str ) {
  return str.index();
}

}


/**
 * kDebug() stream operator.  Writes the string to the debug output.
 */
KDEVPLATFORMSERIALIZATION_EXPORT QDebug operator<<(QDebug s, const KDevelop::IndexedString& string);

Q_DECLARE_METATYPE(KDevelop::IndexedString);
Q_DECLARE_TYPEINFO(KDevelop::IndexedString, Q_MOVABLE_TYPE);

#endif
