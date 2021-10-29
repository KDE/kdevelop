/*
    SPDX-FileCopyrightText: 2014 Aleix Pol <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DECLARATIONBUILDER_H
#define DECLARATIONBUILDER_H

#include <cmakelistsparser.h>
#include "contextbuilder.h"
#include <language/duchain/topducontext.h>
#include <language/duchain/builders/abstractdeclarationbuilder.h>

using DeclarationBuilderBase = KDevelop::AbstractDeclarationBuilder<CMakeContentIterator, CMakeFunctionDesc, ContextBuilder>;

class DeclarationBuilder : public DeclarationBuilderBase
{
public:
//     virtual KDevelop::ReferencedTopDUContext build(const KDevelop::IndexedString& url, CMakeFunctionDesc* node, KDevelop::ReferencedTopDUContext updateContext);
    void startVisiting(CMakeContentIterator* node) override;
};

#endif // DECLARATIONBUILDER_H
