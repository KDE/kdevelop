/*
    SPDX-FileCopyrightText: 2007 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2014 Sven Brauch <svenbrauch@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KDEVPLATFORM_TYPEUTILS_H
#define KDEVPLATFORM_TYPEUTILS_H

#include <language/languageexport.h>
#include "abstracttype.h"
#include "unsuretype.h"
#include "integraltype.h"

#include <type_traits>

namespace KDevelop {
class TopDUContext;
}

namespace TypeUtils {
/**
 * Returns the completely dereferenced and un-aliased type, pointers are also dereferenced(example: ReferenceType(PointerType(int)) -> int)
 * All modifiers are pushed from the aliases into the targets.
 *
 *  !!DU-Chain must be locked!
 * Modifiers of aliases ore references are pushed into the targets.
 * @return return-value will only be zero if type is zero
 */
KDEVPLATFORMLANGUAGE_EXPORT KDevelop::AbstractType::Ptr targetType(const KDevelop::AbstractType::Ptr& type,
                                                                   const KDevelop::TopDUContext* topContext,
                                                                   bool* constant = nullptr);
/**
 * Same as targetType(..), except that it does not un-aliases TypeAliasTypes
 * Modifiers of aliases ore references are pushed into the targets.
 */
KDEVPLATFORMLANGUAGE_EXPORT KDevelop::AbstractType::Ptr targetTypeKeepAliases(const KDevelop::AbstractType::Ptr& type,
                                                                              const KDevelop::TopDUContext* topContext,
                                                                              bool* constant = nullptr);
/**
 * Resolves all type-aliases, returning the effective aliased type
 * All modifiers are pushed from the aliases into the targets.
 */
KDEVPLATFORMLANGUAGE_EXPORT KDevelop::AbstractType::Ptr unAliasedType(const KDevelop::AbstractType::Ptr& type);

/**
 * @brief If @p eventualAlias is an AliasType, return its aliasedType(), otherwise return @p eventualAlias.
 */
KDEVPLATFORMLANGUAGE_EXPORT KDevelop::AbstractType::Ptr resolveAliasType(
    const KDevelop::AbstractType::Ptr& eventualAlias);

/**
 * @brief Check whether the passed type is a null or mixed type.
 * @param type The type to check. Can be null, in which case the function returns false.
 */
KDEVPLATFORMLANGUAGE_EXPORT bool isUsefulType(KDevelop::AbstractType::Ptr type);

/**
 * @brief Merge the second type into the first one
 *
 * If either of @p type or @p newType is null or mixed, return the other one.
 * If one or both of the types is an unsure, or if both types are not null, return
 * an unsure type representing all possible types from both.
 *
 * @param type old type
 * @param newType the type to be added to @p type
 * @return AbstractType::Ptr the merged type, always valid
 *
 * The first argument might be modified, the second one won't be.
 * So if you do something like a = mergeTypes(a, b) make sure you pass "a" as first argument.
 *
 * @warning: If your language has its own specialized UnsureType, make sure to pass it
 * as a template parameter.
 **/
template <typename LanguageUnsureType = KDevelop::UnsureType>
KDevelop::AbstractType::Ptr mergeTypes(KDevelop::AbstractType::Ptr type, const KDevelop::AbstractType::Ptr& newType)
{
    static_assert(std::is_base_of<KDevelop::UnsureType, LanguageUnsureType>::value,
                  "LanguageUnsureType must inherit from KDevelop::UnsureType");

    auto unsure = type.dynamicCast<LanguageUnsureType>();
    auto newUnsure = newType.dynamicCast<LanguageUnsureType>();
    typename LanguageUnsureType::Ptr ret;

    // both types are unsure, so join the list of possible types.
    if (unsure && newUnsure) {
        int len = newUnsure->typesSize();
        for (int i = 0; i < len; i++) {
            unsure->addType(newUnsure->types()[i]);
        }

        ret = unsure;
    }
    // one of them is unsure, use that and add the other one
    else if (unsure) {
        if (isUsefulType(newType)) {
            unsure->addType(newType->indexed());
        }
        ret = unsure;
    } else if (newUnsure) {
        auto createdUnsureType = KDevelop::AbstractType::Ptr(newUnsure->clone()).staticCast<LanguageUnsureType>();
        if (isUsefulType(type)) {
            createdUnsureType->addType(type->indexed());
        }
        ret = createdUnsureType;
    } else {
        unsure = typename LanguageUnsureType::Ptr(new LanguageUnsureType());
        if (isUsefulType(type)) {
            unsure->addType(type->indexed());
        }
        if (isUsefulType(newType)) {
            unsure->addType(newType->indexed());
        }
        if (!unsure->typesSize()) {
            return KDevelop::AbstractType::Ptr(new KDevelop::IntegralType(KDevelop::IntegralType::TypeMixed));
        }
        ret = unsure;
    }
    if (ret->typesSize() == 1) {
        return ret->types()[0].abstractType();
    } else {
        return ret;
    }
}
}

#endif
