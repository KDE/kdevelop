/*
    SPDX-FileCopyrightText: 2011-2014 Sven Brauch <svenbrauch@googlemail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDEVPLATFORM_CONTAINER_TYPES_H
#define KDEVPLATFORM_CONTAINER_TYPES_H

#include "structuretype.h"
#include "typesystemdata.h"
#include "typeutils.h"
#include "../duchainlock.h"

#include <language/languageexport.h>

namespace KDevelop {
class KDEVPLATFORMLANGUAGE_EXPORT ListTypeData
    : public KDevelop::StructureTypeData
{
public:
    ListTypeData()
        : KDevelop::StructureTypeData()
        , m_contentType() { }

    ListTypeData(const ListTypeData& rhs)
        : KDevelop::StructureTypeData(rhs)
        , m_contentType(rhs.m_contentType) { }

    explicit ListTypeData(const KDevelop::StructureTypeData& rhs)
        : KDevelop::StructureTypeData(rhs)
        , m_contentType() { }

    ~ListTypeData() = default;

    ListTypeData& operator=(const ListTypeData& rhs) = delete;

    IndexedType m_contentType;
};

/**
 * @brief Represents a list-like object which can have a content type.
 *
 * Example for Python:
 * @code
 *     # in the file describing the built-in list type
 *     class List:         # (1)
 *         pass
 *
 *     # in the user's code:
 *     a = []              # (2)
 *     a.append(3)         # (3)
 * @endcode
 *
 * This type class can be used to determine the type of a as "list of int" as follows:
 *     (1) When creating the type for the List class,
 *         create a ListType instead of a structure type.
 *         (Your language plugin somehow needs to know
 *         which classes are list-like; Python does this
 *         through special comments for the class)
 *     (2) Set the type of a to the type declared by the
 *         List class, as usual.
 *     (3) Call
 *         \code
 *                static_cast<ListType*>(a->abstractType())->addContentType(int_type)
 *         \endcode
 *         (Your language plugin needs to know which methods
 *         add their argument's content to the type's content;
 *         Python does this through special comments for the method)
 */
class KDEVPLATFORMLANGUAGE_EXPORT ListType
    : public KDevelop::StructureType
{
public:
    using Ptr = TypePtr<ListType>;

    ListType();
    ListType(const ListType& rhs);
    explicit ListType(StructureTypeData& data);

    ListType& operator=(const ListType& rhs) = delete;

    /**
     * @brief Adds @p typeToAdd to the content type of this list.
     *
     * If the list currently has no type, it is set to this type.
     * If @p typeToAdd equals this list's content type, or is a useless type
     * nothing happens.
     * Otherwise, the type of the list becomes an unsure type of the previous
     * and @p typeToAdd.
     *
     * Pass your language's UnsureType as a template parameter, as it will eventually
     * need to be instantiated.
     *
     * @note If the type is already assigned to a declaration, the duchain
     * must be write-locked when this is called.
     *
     * @param typeToAdd The new type the list's contents can possibly be of.
     */
    template <typename LanguageUnsureType>
    void addContentType(const AbstractType::Ptr& typeToAdd)
    {
        auto newContentType = TypeUtils::mergeTypes<LanguageUnsureType>(contentType().abstractType(), typeToAdd);
        d_func_dynamic()->m_contentType = IndexedType(newContentType);
    }

    /**
     * @brief Replaces this list's content type by @p newType.
     */
    void replaceContentType(const AbstractType::Ptr& newType);

    /**
     * @brief Get this lists's content type.
     */
    IndexedType contentType() const;

    /**
     * @brief Return only the container type, not the content type in a string.
     */
    QString containerToString() const;

    /**
     * @brief Formats this type (base type and content type) in a string.
     */
    QString toString() const override;

    AbstractType* clone() const override;
    size_t hash() const override;
    bool equals(const AbstractType* rhs) const override;

    enum { Identity = 58 };

    using Data = ListTypeData;

protected:
    TYPE_DECLARE_DATA(ListType);
};

class KDEVPLATFORMLANGUAGE_EXPORT MapTypeData
    : public ListTypeData
{
public:
    MapTypeData()
        : ListTypeData()
        , m_keyType() { }

    MapTypeData(const MapTypeData& rhs)
        : ListTypeData(rhs)
        , m_keyType(rhs.m_keyType) { }

    explicit MapTypeData(const ListTypeData& rhs)
        : ListTypeData(rhs)
        , m_keyType() { }

    ~MapTypeData() = default;

    MapTypeData& operator=(const ListTypeData& rhs) = delete;

    IndexedType m_keyType;
};

/**
 * @brief Represents a hashmap-like object which can have a key and a content type.
 *
 * @see ListType
 * This works the same as ListType, except that you can also track the object's key type.
 */
class KDEVPLATFORMLANGUAGE_EXPORT MapType
    : public ListType
{
public:
    using Ptr = TypePtr<MapType>;

    MapType();
    MapType(const MapType& rhs);
    explicit MapType(ListTypeData& data);

    MapType& operator=(const MapType& rhs) = delete;

    /**
     * @brief Add @p typeToAdd to this map's key type.
     * Behaves like addContentType, except that it modifies the key type instead.
     */
    template <typename LanguageUnsureType>
    void addKeyType(const AbstractType::Ptr& typeToAdd)
    {
        auto newKeyType = TypeUtils::mergeTypes<LanguageUnsureType>(keyType().abstractType(), typeToAdd);
        DUChainWriteLocker lock;
        d_func_dynamic()->m_keyType = IndexedType(newKeyType);
    }

    /**
     * @brief Set this map's key type to @p newType.
     */
    void replaceKeyType(const AbstractType::Ptr& newType);

    /**
     * @brief Get this map's key type.
     */
    IndexedType keyType() const;

    /**
     * @brief Formats this type (base type and key+content type) in a string.
     */
    QString toString() const override;

    AbstractType* clone() const override;
    size_t hash() const override;
    bool equals(const AbstractType* rhs) const override;

    enum { Identity = 57 };

    using Data = MapTypeData;

protected:
    TYPE_DECLARE_DATA(MapType);
};
} // namespace KDevelop

#endif // KDEVPLATFORM_CONTAINER_TYPES_H
