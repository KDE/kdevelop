/*
    SPDX-FileCopyrightText: 2015 Sergey Kalinichev <kalinichev.so.0@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "clanghighlighting.h"

#include "duchain/macrodefinition.h"

#include <language/duchain/topducontext.h>

using namespace KDevelop;

class ClangHighlighting::Instance : public KDevelop::CodeHighlightingInstance
{
public:
    explicit Instance(const KDevelop::CodeHighlighting* highlighting);

    CodeHighlightingType typeForDeclaration(KDevelop::Declaration* dec, KDevelop::DUContext* context) const override
    {
        if (auto macro = dynamic_cast<MacroDefinition*>(dec)) {
            if (macro->isFunctionLike()) {
                return CodeHighlightingType::MacroFunctionLike;
            }
        }

        return CodeHighlightingInstance::typeForDeclaration(dec, context);
    }

    bool useRainbowColor(KDevelop::Declaration* dec) const override
    {
        return dec->context()->type() == DUContext::Function || dec->context()->type() == DUContext::Other;
    }

};

ClangHighlighting::Instance::Instance(const KDevelop::CodeHighlighting* highlighting)
    : KDevelop::CodeHighlightingInstance(highlighting)
{
}

ClangHighlighting::ClangHighlighting(QObject* parent)
    : KDevelop::CodeHighlighting(parent)
{
}

KDevelop::CodeHighlightingInstance* ClangHighlighting::createInstance() const
{
    return new Instance(this);
}

#include "moc_clanghighlighting.cpp"
