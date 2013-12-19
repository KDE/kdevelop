/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>
    Copyright 2007/2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef KDEVPLATFORM_DUCHAINBASE_H
#define KDEVPLATFORM_DUCHAINBASE_H

#include "../languageexport.h"
#include "appendedlist.h"
#include "duchainpointer.h"
#include <language/editor/persistentmovingrange.h>

namespace KDevelop
{
class PersistentMovingRange;
class SimpleCursor;
class SimpleRange;
class DUContext;
class TopDUContext;
class DUChainBase;
class IndexedString;

///Use this to declare the data functions in your DUChainBase based class. @warning Behind this macro, the access will be "public".
#define DUCHAIN_DECLARE_DATA(Class) \
    inline class Class##Data* d_func_dynamic() { makeDynamic(); return reinterpret_cast<Class##Data *>(d_ptr); } \
    inline const class Class##Data* d_func() const { return reinterpret_cast<const Class##Data *>(d_ptr); } \
    public: typedef Class ## Data Data; private:

#define DUCHAIN_D(Class) const Class##Data * const d = d_func()
#define DUCHAIN_D_DYNAMIC(Class) Class##Data * const d = d_func_dynamic()

///@note When a data-item is stored on disk, no destructors of contained items will be called while destruction.
///DUChainBase assumes that each item that has constant data, is stored on disk.
///However the destructor is called even on constant items, when they have been replaced with a dynamic item.
///This tries to keep constructor/destructor count consistency persistently, which allows doing static reference-counting
///using contained classes in their constructor/destructors(For example the class Utils::StorableSet).
///This means that the data of all items that are stored to disk _MUST_ be made constant before their destruction.
///This also means that every item that is "semantically" deleted, _MUST_ have dynamic data before its destruction.
///This also means that DUChainBaseData based items should never be cloned using memcpy, but rather always using the copy-constructor,
///even if both sides are constant.
class KDEVPLATFORMLANGUAGE_EXPORT DUChainBaseData {
public:

    DUChainBaseData() : classId(0) {
      initializeAppendedLists();
    }
    
    DUChainBaseData(const DUChainBaseData& rhs) : m_range(rhs.m_range), classId(rhs.classId) {
      initializeAppendedLists();
    }
    
    ~DUChainBaseData() {
      freeAppendedLists();
    }
    
    RangeInRevision m_range;
    
    APPENDED_LISTS_STUB(DUChainBaseData)
    
    uint classId;

    bool isDynamic() const {
      return m_dynamic;
    }
    
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

    ///This is called whenever the data-object is being deleted memory-wise, but not semantically(Which means it stays on disk)
    ///Implementations of parent-classes must always be called
    void freeDynamicData() {
    }
    
    ///Used to decide whether a constructed item should create constant data.
    ///The default is "false", so dynamic data is created by default.
    ///This is stored thread-locally.
    static bool& shouldCreateConstantData();
    
    ///Returns whether initialized objects should be created as dynamic objects
    static bool appendedListDynamicDefault() {
      return !shouldCreateConstantData();
    }
};

/**
 * Base class for definition-use chain objects.
 *
 * This class provides a thread safe pointer type to reference duchain objects
 * while the DUChain mutex is not held (\see DUChainPointer)
 */

class KDEVPLATFORMLANGUAGE_EXPORT DUChainBase
{
public:
  /**
   * Constructor.
   *
   * \param url url of the document where this occurred
   * \param range range of the alias declaration's identifier
   */
  DUChainBase(const RangeInRevision& range);
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

  virtual IndexedString url() const;
  
  enum {
    Identity = 1
  };
  
  ///After this was called, the data-pointer is dynamic. It is cloned if needed.
  void makeDynamic();
  
  DUChainBase( DUChainBaseData& dd );
  
  ///This must only be used to change the storage-location or storage-kind(dynamic/constant) of the data, but
  ///the data must always be equal!
  virtual void setData(DUChainBaseData*, bool constructorCalled = true);
  
  ///Returns the range assigned to this object, in the document revision when this document was last parsed.
  RangeInRevision range() const;

  ///Changes the range assigned to this object, in the document revision when this document is parsed.
  void setRange(const RangeInRevision& range);
  
  ///Returns the range assigned to this object, transformed into the current revision of the document.
  ///@warning This must only be called from the foreground thread, or with the foreground lock acquired.
  SimpleRange rangeInCurrentRevision() const;

  ///Returns the range assigned to this object, transformed into the current revision of the document.
  ///The returned object is unique at each call, so you can use it and change it in whatever way you want.
  ///@warning This must only be called from the foreground thread, or with the foreground lock acquired.
  PersistentMovingRange::Ptr createRangeMoving() const;
  
  ///Transforms the given cursor in the current document revision to its according position
  ///in the parsed document containing this duchain object. The resulting cursor will be directly comparable to the non-translated
  ///range() members in the duchain, but only for one duchain locking cycle.
  ///@warning This must only be called from the foreground thread, or with the foreground lock acquired.
  CursorInRevision transformToLocalRevision(const SimpleCursor& cursor) const;

  ///Transforms the given range in the current document revision to its according position
  ///in the parsed document containing this duchain object. The resulting cursor will be directly comparable to the non-translated
  ///range() members in the duchain, but only for one duchain locking cycle.
  ///@warning This must only be called from the foreground thread, or with the foreground lock acquired.
  RangeInRevision transformToLocalRevision(const SimpleRange& range) const;

  SimpleCursor transformFromLocalRevision(const CursorInRevision& cursor) const;
  
  SimpleRange transformFromLocalRevision(const RangeInRevision& range) const;

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
  DUChainBase( DUChainBaseData& dd, const RangeInRevision& range );

  ///Called after loading to rebuild the dynamic data. If this is a context, this should recursively work on all sub-contexts.
  virtual void rebuildDynamicData(DUContext* parent, uint ownIndex);
  
  /// Data pointer that is shared across all the inheritance hierarchy
  DUChainBaseData* d_ptr;
private:
  
  mutable KSharedPtr<DUChainPointerData> m_ptr;
public:
  DUCHAIN_DECLARE_DATA(DUChainBase)
};
}

#endif // KDEVPLATFORM_DUCHAINBASE_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
