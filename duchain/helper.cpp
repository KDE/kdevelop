/*************************************************************************************
 *  Copyright (C) 2013 by Andrea Scarpino <scarpino@kde.org>                         *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "helper.h"

#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/types/unsuretype.h>
#include <language/duchain/types/integraltype.h>

namespace QmlJS
{
using namespace KDevelop;

DeclarationPointer getDeclaration(const QualifiedIdentifier& id, const DUContextPointer& context)
{
    DUChainReadLocker lock;
    if (context) {
        foreach (Declaration* dec, context->findDeclarations(id)) {
            return DeclarationPointer(dec);
        }
    }
    return DeclarationPointer();
}

KDevelop::AbstractType::Ptr mergeTypes(KDevelop::AbstractType::Ptr type,
                                       const KDevelop::AbstractType::Ptr newType)
{
    Q_ASSERT(newType);

    UnsureType::Ptr type_unsure = UnsureType::Ptr::dynamicCast(type);
    IntegralType::Ptr type_integral = IntegralType::Ptr::dynamicCast(type);
    UnsureType::Ptr newtype_unsure = UnsureType::Ptr::dynamicCast(newType);
    IntegralType::Ptr newtype_integral = IntegralType::Ptr::dynamicCast(newType);

    if (!type) {
        // No previous type available
        return newType;
    } if (newtype_integral && newtype_integral->dataType() == IntegralType::TypeMixed) {
        // Do not add a mixed type to another one, as mixed types have no value
        return type;
    } else if (type_integral && type_integral->dataType() == IntegralType::TypeMixed) {
        // If the original type was mixed, replace it with the new one
        return newType;
    } else if (type_unsure && newtype_unsure) {
        // both types are unsure, so join the list of possible types.
        int len = newtype_unsure->typesSize();

        for (int i = 0; i < len; i++) {
            type_unsure->addType(newtype_unsure->types()[i]);
        }

        return AbstractType::Ptr::staticCast(type_unsure);
    } else if (type_unsure) {
        // Add the other type to the current unsure type
        type_unsure->addType(newType->indexed());

        return AbstractType::Ptr::staticCast(type_unsure);
    } else if (newtype_unsure) {
        // The type that cannot be modified is unsure, copy it and add the first
        // type to it.
        UnsureType::Ptr ret = UnsureType::Ptr(static_cast<UnsureType *>(newtype_unsure->clone()));

        ret->addType(type->indexed());

        return AbstractType::Ptr::staticCast(ret);
    } else if (type->equals(newType.constData())) {
        // The two types are the same, no need to merge anything
        // (this comparison is more expensive than checking pointers, and is
        // therefore performed last)
        return type;
    } else {
        // The types are different and not unsure nor mixed. Simply join them
        UnsureType::Ptr ret(new UnsureType);

        ret->addType(type->indexed());
        ret->addType(newType->indexed());

        return AbstractType::Ptr::staticCast(ret);
    }
}


} // End of namespace QmlJS
