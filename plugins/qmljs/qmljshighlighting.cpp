/*
    SPDX-FileCopyrightText: 2013 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qmljshighlighting.h"

#include <language/duchain/declaration.h>

using namespace KDevelop;

class HighlightingInstance : public KDevelop::CodeHighlightingInstance
{
public:
    explicit HighlightingInstance(const CodeHighlighting* highlighting)
    : CodeHighlightingInstance(highlighting)
    {}

    bool useRainbowColor(Declaration* dec) const override
    {
        // JS has a function-based prototype OO system, so for now rainbow-color
        // everything that is not a function declaration.
        // In the future we will have to investigate how to handle properties
        // of a function/object
        return dec->kind() == Declaration::Instance &&
               dec->context()->type() != DUContext::Class &&
               dec->context()->type() != DUContext::Enum;
    }
};

QmlJsHighlighting::QmlJsHighlighting(QObject* parent)
: CodeHighlighting(parent)
{

}

CodeHighlightingInstance* QmlJsHighlighting::createInstance() const
{
    return new HighlightingInstance(this);
}

#include "moc_qmljshighlighting.cpp"
