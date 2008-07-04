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

#ifndef TYPEREGISTER_H
#define TYPEREGISTER_H

#include "typesystem.h"
#include "typesystemdata.h"

namespace KDevelop {

class KDEVPLATFORMLANGUAGE_EXPORT AbstractTypeFactory {
  public:
  virtual AbstractType* create(AbstractTypeData* /*data*/) const = 0;
  virtual void callDestructor(AbstractTypeData* data) const = 0;
  virtual void copy(const AbstractTypeData& from, AbstractTypeData& to, bool constant) const = 0;
  virtual uint dynamicSize(const AbstractTypeData& data) const = 0;
  ///Returns the class size(measures the most derived class), but does not include dynamic data.
  
  virtual ~AbstractTypeFactory() {
  }
};

template<class T, class Data>
class KDEVPLATFORMLANGUAGE_EXPORT TypeFactory : public AbstractTypeFactory {
  public:
  AbstractType* create(AbstractTypeData* data) const {
    return new T(*static_cast<typename T::Data*>(data));
  }
  void copy(const AbstractTypeData& from, AbstractTypeData& to, bool constant) const {
    Q_ASSERT(from.typeClassId == T::Identity);
    
    if((bool)from.m_dynamic == (bool)!constant) {
      //We have a problem, "from" and "to" should both be either dynamic or constant. We must copy once more.
      Data* temp = &AbstractType::copyData<Data>(static_cast<const Data&>(from));
      
      new (&to) Data(*temp); //Call the copy constructor to initialize the target
      
      Q_ASSERT((bool)to.m_dynamic == (bool)!constant);
      delete temp;
    }else{
      new (&to) Data(static_cast<const Data&>(from)); //Call the copy constructor to initialize the target
    }
  }
  void callDestructor(AbstractTypeData* data) const {
    Q_ASSERT(data->typeClassId == T::Identity);
    static_cast<Data*>(data)->~Data();
  }
  
  uint dynamicSize(const AbstractTypeData& data) const {
    Q_ASSERT(data.typeClassId == T::Identity);
    return static_cast<const Data&>(data).dynamicSize();
  }
};

class KDEVPLATFORMLANGUAGE_EXPORT TypeSystem {
  public:
    template<class T, class Data>
    void registerTypeClass() {
      Q_ASSERT(T::Identity < 64);
      if(m_factories.size() <= T::Identity) {
        m_factories.resize(T::Identity+1);
        m_dataClassSizes.resize(T::Identity+1);
      }
      
      Q_ASSERT(!m_factories[T::Identity]);
      m_factories[T::Identity] = new TypeFactory<T, Data>();
      m_dataClassSizes[T::Identity] = sizeof(Data);
    }
    
    template<class T, class Data>
    void unregisterTypeClass() {
      Q_ASSERT(m_factories.size() > T::Identity);
      Q_ASSERT(m_factories[T::Identity]);
      delete m_factories[T::Identity];
      m_factories[T::Identity] = 0;
      m_dataClassSizes[T::Identity] = 0;
    }
    
    //Creates an AbstractType for the given data. The returned type must be put into a AbstractType::Ptr immediately.
    //Can return zero, of no type-factory is available for the given data(for example when a language-part is not loaded)
    AbstractType* create(AbstractTypeData* data) const;
    
    ///This just calls the correct constructor on the target. The target must be big enough to hold all the data.
    void copy(const AbstractTypeData& from, AbstractTypeData& to, bool constant) const;
    
    ///Calls the dynamicSize(..) member on the given data, in the most special class. Since we cannot use virtual functions, this is the only way.
    uint dynamicSize(const AbstractTypeData& data) const;
    
    ///Returns the size of the derived class, not including dynamic data.
    ///Returns zero if the class is not known.
    size_t dataClassSize(const AbstractTypeData& data) const;
    
    //Calls the destructor, but does not delete anything. This is needed because the data classes must not contain virtual members.
    void callDestructor(AbstractTypeData* data) const;
    
    static TypeSystem& self();
    
  private:
    QVector<AbstractTypeFactory*> m_factories;
    QVector<size_t> m_dataClassSizes;
};

template<class T, class Data>
struct KDEVPLATFORMLANGUAGE_EXPORT TypeSystemRegistrator {
  TypeSystemRegistrator() {
    TypeSystem::self().registerTypeClass<T, Data>();
  }
  ~TypeSystemRegistrator() {
    TypeSystem::self().unregisterTypeClass<T, Data>();
  }
};

///You must add this into your source-files for every AbstractType based class
///For this to work, the class must have an "Identity" enumerator, that is unique among all types, and should be a very low value.
///The highest allowed identity is 63, so currently we're limited to having 64 different type classes.
#define REGISTER_TYPE(Class) TypeSystemRegistrator<Class, Class ## Data> register ## Class

}

#endif
