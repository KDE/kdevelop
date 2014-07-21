/*
 * This file is part of qmljs, the QML/JS language support plugin for KDevelop
 * Copyright (c) 2014 Denis Steckelmacher <steckdenis@yahoo.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef __FUNCTIONTYPE_H__
#define __FUNCTIONTYPE_H__

#include "duchainexport.h"

#include <language/duchain/types/functiontype.h>
#include <language/duchain/types/identifiedtype.h>
#include <language/duchain/types/typepointer.h>
#include <language/duchain/types/typesystemdata.h>

namespace QmlJS {

typedef KDevelop::MergeIdentifiedType<KDevelop::FunctionType> FunctionTypeBase;

/**
 * Function type bound to a function declaration, so that the name of its parameters
 * can be displayed when needed
 */
class KDEVQMLJSDUCHAIN_EXPORT FunctionType : public FunctionTypeBase
{
public:
    typedef TypePtr<FunctionType> Ptr;

    FunctionType();
    FunctionType(const FunctionType& rhs);
    FunctionType(Data& data);
    virtual ~FunctionType();

    virtual AbstractType* clone() const;
    virtual QString toString() const;
    virtual uint hash() const;

    enum {
        Identity = 30
    };

    typedef FunctionTypeBase::Data Data;
};

}

#endif