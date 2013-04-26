/* This file is part of KDevelop
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

#ifndef KDEVPLATFORM_DUCHAINREGISTER_H
#define KDEVPLATFORM_DUCHAINREGISTER_H

#include "duchainbase.h"

namespace KDevelop {
class DUChainBase;
class DUChainBaseData;

///This class is purely internal and doesn't need to be documented. It brings a "fake" type-info
///to classes that don't have type-info in the normal C++ way.
///Never use this directly, use the REGISTER_DUCHAIN_ITEM macro instead.
class KDEVPLATFORMLANGUAGE_EXPORT DUChainBaseFactory {
  public:
  virtual DUChainBase* create(DUChainBaseData* data) const = 0;
  virtual void callDestructor(DUChainBaseData* data) const = 0;
  virtual void freeDynamicData(DUChainBaseData* data) const = 0;
  virtual void copy(const DUChainBaseData& from, DUChainBaseData& to, bool constant) const = 0;
  virtual DUChainBaseData* cloneData(const DUChainBaseData& data) const = 0;
  virtual uint dynamicSize(const DUChainBaseData& data) const = 0;

  virtual ~DUChainBaseFactory() {
  }
};

///Never use this directly, use the REGISTER_DUCHAIN_ITEM macro instead.
template<class T, class Data>
class DUChainItemFactory : public DUChainBaseFactory {
  public:
  DUChainBase* create(DUChainBaseData* data) const {
    return new T(*static_cast<Data*>(data));
  }
  
  void copy(const DUChainBaseData& from, DUChainBaseData& to, bool constant) const {
    Q_ASSERT(from.classId == T::Identity);

    bool previousConstant = DUChainBaseData::shouldCreateConstantData();
    DUChainBaseData::setShouldCreateConstantData(constant);
    
    new (&to) Data(static_cast<const Data&>(from)); //Call the copy constructor to initialize the target
    
    DUChainBaseData::setShouldCreateConstantData(previousConstant);
  }
  
  void callDestructor(DUChainBaseData* data) const {
    Q_ASSERT(data->classId == T::Identity);
    static_cast<Data*>(data)->~Data();
  }

  void freeDynamicData(DUChainBaseData* data) const {
    Q_ASSERT(data->classId == T::Identity);
    static_cast<Data*>(data)->freeDynamicData();
  }

  uint dynamicSize(const DUChainBaseData& data) const {
    Q_ASSERT(data.classId == T::Identity);
    return static_cast<const Data&>(data).dynamicSize();
  }
  
   DUChainBaseData* cloneData(const DUChainBaseData& data) const {
     Q_ASSERT(data.classId == T::Identity);
     return new Data(static_cast<const Data&>(data));
   }
};

/**
 * \short A class which registers data types and creates factories for them.
 *
 * DUChainItemSystem is a global static class which allows you to register new
 * DUChainBase subclasses for creation.
 */
class KDEVPLATFORMLANGUAGE_EXPORT DUChainItemSystem {
  public:
    /**
     * Register a new DUChainBase subclass.
     */
    template<class T, class Data>
    void registerTypeClass() {
      if(m_factories.size() <= T::Identity) {
        m_factories.resize(T::Identity+1);
        m_dataClassSizes.resize(T::Identity+1);
      }

      Q_ASSERT(!m_factories[T::Identity]);
      m_factories[T::Identity] = new DUChainItemFactory<T, Data>();
      m_dataClassSizes[T::Identity] = sizeof(Data);
    }

    /**
     * Unregister an DUChainBase subclass.
     */
    template<class T, class Data>
    void unregisterTypeClass() {
      Q_ASSERT(m_factories.size() > T::Identity);
      Q_ASSERT(m_factories[T::Identity]);
      delete m_factories[T::Identity];
      m_factories[T::Identity] = 0;
      m_dataClassSizes[T::Identity] = 0;
    }

    /**
     * Create an DUChainBase for the given data. The returned type must be put into a DUChainBase::Ptr immediately.
     * Can return null if no type-factory is available for the given data (for example when a language-part is not loaded)
     */
    DUChainBase* create(DUChainBaseData* data) const;

    ///Creates a dynamic copy of the given data
    DUChainBaseData* cloneData(const DUChainBaseData& data) const;
    
    /**
     * This just calls the correct constructor on the target. The target must be big enough to hold all the data.
     * If constant is true, it must be as big as dynamicSize(from).
     */
    void copy(const DUChainBaseData& from, DUChainBaseData& to, bool constant) const;

    ///Calls the dynamicSize(..) member on the given data, in the most special class. Since we cannot use virtual functions, this is the only way.
    uint dynamicSize(const DUChainBaseData& data) const;

    ///Returns the size of the derived class, not including dynamic data.
    ///Returns zero if the class is not known.
    uint dataClassSize(const DUChainBaseData& data) const;

    ///Calls the destructor, but does not delete anything. This is needed because the data classes must not contain virtual members.
    ///This should only be called when a duchain data-pointer is semantically deleted, eg. when it does not persist on disk.
    void callDestructor(DUChainBaseData* data) const;

    ///Does not call the destructor, but frees all special data associated to dynamic data(the appendedlists stuff)
    ///This needs to be called whenever a dynamic duchain data-pointer is being deleted.
    void freeDynamicData(DUChainBaseData* data) const;
    
    /// Access the static DUChainItemSystem instance.
    static DUChainItemSystem& self();

  private:
    QVector<DUChainBaseFactory*> m_factories;
    QVector<uint> m_dataClassSizes;
};

/// Helper class to register an DUChainBase subclass.
///
/// Just use the REGISTER_TYPE(YourTypeClass) macro in your code, and you're done.
template<class T, class Data>
struct DUChainItemRegistrator {
  DUChainItemRegistrator() {
    DUChainItemSystem::self().registerTypeClass<T, Data>();
  }
  ~DUChainItemRegistrator() {
    DUChainItemSystem::self().unregisterTypeClass<T, Data>();
  }
};

///You must add this into your source-files for every DUChainBase based class
///For this to work, the class must have an "Identity" enumerator.
///It should be a unique value, but as small as possible, because a buffer at least as big as that number is created internally.
#define REGISTER_DUCHAIN_ITEM(Class) KDevelop::DUChainItemRegistrator<Class, Class ## Data> register ## Class
#define REGISTER_DUCHAIN_ITEM_WITH_DATA(Class, Data) KDevelop::DUChainItemRegistrator<Class, Data> register ## Class

}

#endif
