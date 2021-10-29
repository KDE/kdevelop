/*
    SPDX-FileCopyrightText: 2014 Denis Steckelmacher <steckdenis@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef __DECLARATIONNAVIGATIONCONTEXT_H__
#define __DECLARATIONNAVIGATIONCONTEXT_H__

#include <language/duchain/navigation/abstractdeclarationnavigationcontext.h>
#include <language/duchain/types/identifiedtype.h>

namespace QmlJS {

class DeclarationNavigationContext : public KDevelop::AbstractDeclarationNavigationContext
{
    Q_OBJECT
public:
    using KDevelop::AbstractDeclarationNavigationContext::AbstractDeclarationNavigationContext;

protected:
    void htmlIdentifiedType(KDevelop::AbstractType::Ptr type, const KDevelop::IdentifiedType* idType) override;
    void eventuallyMakeTypeLinks(KDevelop::AbstractType::Ptr type) override;
};

}

#endif
