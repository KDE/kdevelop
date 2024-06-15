/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef __FUNCTIONTYPE_H__
#define __FUNCTIONTYPE_H__

#include "duchainexport.h"

#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/identifiedtype.h>
#include <language/duchain/types/typepointer.h>
#include <language/duchain/types/typesystemdata.h>

namespace QmlJS {

using FunctionTypeBase = KDevelop::MergeIdentifiedType<KDevelop::FunctionType>;

/**
 * Function type bound to a function declaration, so that the name of its parameters
 * can be displayed when needed
 */
class KDEVQMLJSDUCHAIN_EXPORT FunctionType : public FunctionTypeBase
{
public:
    using Ptr = KDevelop::TypePtr<FunctionType>;

    FunctionType();
    FunctionType(const FunctionType& rhs);
    explicit FunctionType(Data& data);
    ~FunctionType() override;

    KDevelop::AbstractType* clone() const override;
    QString toString() const override;
    size_t hash() const override;

    enum {
        Identity = 30
    };

    using Data = FunctionTypeBase::Data;
};

}

#endif
