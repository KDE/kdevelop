/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#ifndef DUCHAINBASE_H
#define DUCHAINBASE_H

#include "language/editor/documentrangeobject.h"
#include "language/editor/hashedstring.h"
#include "language/languageexport.h"
#include "appendedlist.h"

namespace KDevelop
{

class TopDUContext;
class DUChainBase;
class DUChainPointerData;
class IndexedString;

#define DUCHAIN_DECLARE_DATA(Class) \
    inline class Class##Data* d_func_dynamic() { makeDynamic(); return reinterpret_cast<Class##Data *>(d_ptr); } \
    inline const class Class##Data* d_func() const { return reinterpret_cast<const Class##Data *>(d_ptr); }

#define DUCHAIN_D(Class) const Class##Data * const d = d_func()
#define DUCHAIN_D_DYNAMIC(Class) Class##Data * const d = d_func_dynamic()

struct DUChainBaseData : public DocumentRangeObjectData {
    DUChainBaseData() : classId(0) {
    }
    DUChainBaseData(const DUChainBaseData& rhs) : DocumentRangeObjectData(rhs), classId(rhs.classId) {
    }
    
    uint classId;

    /**
    * Internal setup for the data structure.
    *
    * This must be called from actual class that belongs to this data(not parent classes), and the class must have the
    * "Identity" enumerator with a unique identity. Do NOT call this in copy-constructors!
    */
    template<class T>
    void setClassId(T*) {
      classId = T::Identity;
    }
      
    uint classSize() const;

    APPENDED_LISTS_STUB(DocumentRangeObjectData);
};

/**
 * Base class for definition-use chain objects.
 *
 * This class provides a thread safe pointer type to reference duchain objects
 * while the DUChain mutex is not held (\see DUChainPointer)
 */

class KDEVPLATFORMLANGUAGE_EXPORT DUChainBase : public KDevelop::DocumentRangeObject
{
public:
  /**
   * Constructor.
   *
   * \param url url of the document where this occurred
   * \param range range of the alias declaration's identifier
   */
  DUChainBase(const SimpleRange& range);
  /// Destructor
  virtual ~DUChainBase();

  /**
   * Determine the top context to which this object belongs.
   */
  virtual TopDUContext* topContext() const;

  /**
   * Returns a special pointer that can be used to track the existence of a du-chain object across locking-cycles.
   * @see DUChainPointerData
   * */
  const KSharedPtr<DUChainPointerData>& weakPointer() const;

  IndexedString url() const;
  
  enum {
    Identity = 1
  };
  
  void makeDynamic() {
  }
  
  DUChainBase( DUChainBaseData& dd );
  
protected:
  /**
   * Creates a duchain object that uses the data of the given one, and will not delete it on destruction.
   * The data will be shared, and this object must be deleted before the given one is.
   */
  DUChainBase( DUChainBase& rhs );

  /**
    * Constructor for copy constructors in subclasses.
    *
    * \param dd data to use.
    * \param url document url in which this object is located.
    * \param range text range which this object covers.
    */
  DUChainBase( DUChainBaseData& dd, const SimpleRange& range );

private:
  DUCHAIN_DECLARE_DATA(DUChainBase)
  mutable KSharedPtr<DUChainPointerData> m_ptr;
};
}

#endif // DUCHAINBASE_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
