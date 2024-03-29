/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>

    SPDX-License-Identifier: LGPL-2.0-only
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
class KDEVPLATFORMLANGUAGE_EXPORT DUChainBaseFactory
{
public:
    virtual DUChainBase* create(DUChainBaseData* data) const = 0;
    virtual void callDestructor(DUChainBaseData* data) const = 0;
    virtual void freeDynamicData(DUChainBaseData* data) const = 0;
    virtual void deleteDynamicData(DUChainBaseData* data) const = 0;
    virtual void copy(const DUChainBaseData& from, DUChainBaseData& to, bool constant) const = 0;
    virtual DUChainBaseData* cloneData(const DUChainBaseData& data) const = 0;
    virtual uint dynamicSize(const DUChainBaseData& data) const = 0;

    virtual ~DUChainBaseFactory()
    {
    }
};

///Never use this directly, use the REGISTER_DUCHAIN_ITEM macro instead.
template <class T, class Data>
class DUChainItemFactory
    : public DUChainBaseFactory
{
public:
    DUChainBase* create(DUChainBaseData* data) const override
    {
        return new T(*static_cast<Data*>(data));
    }

    void copy(const DUChainBaseData& from, DUChainBaseData& to, bool constant) const override
    {
        Q_ASSERT(from.classId == T::Identity);

        bool& isConstant = DUChainBaseData::shouldCreateConstantData();
        const bool previousConstant = isConstant;
        if (previousConstant != constant) {
            isConstant = constant;
        }

        new (&to) Data(static_cast<const Data&>(from)); //Call the copy constructor to initialize the target

        if (previousConstant != constant) {
            isConstant = previousConstant;
        }
    }

    void callDestructor(DUChainBaseData* data) const override
    {
        Q_ASSERT(data->classId == T::Identity);
        static_cast<Data*>(data)->~Data();
    }

    void freeDynamicData(DUChainBaseData* data) const override
    {
        Q_ASSERT(data->classId == T::Identity);
        static_cast<Data*>(data)->freeDynamicData();
    }

    void deleteDynamicData(DUChainBaseData* data) const override
    {
        Q_ASSERT(data->classId == T::Identity);
        delete static_cast<Data*>(data);
    }

    uint dynamicSize(const DUChainBaseData& data) const override
    {
        Q_ASSERT(data.classId == T::Identity);
        return static_cast<const Data&>(data).dynamicSize();
    }

    DUChainBaseData* cloneData(const DUChainBaseData& data) const override
    {
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
class KDEVPLATFORMLANGUAGE_EXPORT DUChainItemSystem
{
public:
    /**
     * Register a new DUChainBase subclass.
     */
    template <class T, class Data>
    void registerTypeClass()
    {
        if (m_factories.size() <= T::Identity) {
            m_factories.resize(T::Identity + 1);
            m_dataClassSizes.resize(T::Identity + 1);
        }

        Q_ASSERT_X(!m_factories[T::Identity], Q_FUNC_INFO, "This identity is already registered");
        m_factories[T::Identity] = new DUChainItemFactory<T, Data>();
        m_dataClassSizes[T::Identity] = sizeof(Data);
    }

    /**
     * Unregister an DUChainBase subclass.
     */
    template <class T, class Data>
    void unregisterTypeClass()
    {
        Q_ASSERT(m_factories.size() > T::Identity);
        Q_ASSERT(m_factories[T::Identity]);
        delete m_factories[T::Identity];
        m_factories[T::Identity] = nullptr;
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

    /// Call delete on @p data
    void deleteDynamicData(DUChainBaseData* data) const;

    /// Access the static DUChainItemSystem instance.
    static DUChainItemSystem& self();

private:
    Q_DISABLE_COPY(DUChainItemSystem)
    DUChainItemSystem() = default;
    ~DUChainItemSystem();

    QVector<DUChainBaseFactory*> m_factories;
    QVector<uint> m_dataClassSizes;
};

template <typename T>
struct DUChainType {};

/// Use this in the header to declare DUChainType<YourTypeClass>
#define DUCHAIN_DECLARE_TYPE(Type) \
    namespace KDevelop { \
    template <> struct DUChainType<Type> { \
        static void registerType(); \
        static void unregisterType(); \
    }; \
    }
/// Use this in the source file to define functions in DUChainType<YourTypeClass>
#define DUCHAIN_DEFINE_TYPE_WITH_DATA(Type, Data) \
    void KDevelop::DUChainType<Type>::registerType() { DUChainItemSystem::self().registerTypeClass<Type, Data>(); } \
    void KDevelop::DUChainType<Type>::unregisterType() { DUChainItemSystem::self().unregisterTypeClass<Type, Data>(); }
#define DUCHAIN_DEFINE_TYPE(Type) \
    DUCHAIN_DEFINE_TYPE_WITH_DATA(Type, Type ## Data)

/// Register @p T to DUChainItemSystem
template <typename T>
void duchainRegisterType() { DUChainType<T>::registerType(); }
/// Unregister @p T to DUChainItemSystem
template <typename T>
void duchainUnregisterType() { DUChainType<T>::unregisterType(); }

/// Helper class to register an DUChainBase subclass.
///
/// Just use the REGISTER_TYPE(YourTypeClass) macro in your code, and you're done.
template <class T, class Data>
struct DUChainItemRegistrator
{
    DUChainItemRegistrator()
    {
        DUChainItemSystem::self().registerTypeClass<T, Data>();
    }
    ~DUChainItemRegistrator()
    {
        DUChainItemSystem::self().unregisterTypeClass<T, Data>();
    }
private:
    Q_DISABLE_COPY(DUChainItemRegistrator)
};

///You must add this into your source-files for every DUChainBase based class
///For this to work, the class must have an "Identity" enumerator.
///It should be a unique value, but as small as possible, because a buffer at least as big as that number is created internally.
#define REGISTER_DUCHAIN_ITEM(Class) KDevelop::DUChainItemRegistrator<Class, Class ## Data> register ## Class
#define REGISTER_DUCHAIN_ITEM_WITH_DATA(Class, Data) KDevelop::DUChainItemRegistrator<Class, Data> register ## Class
}

#endif
