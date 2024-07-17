/*
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "configurablecolors.h"

#include "colorcache.h"

#include <debug.h>

#include <KTextEditor/View>
#include <KSyntaxHighlighting/Theme>

#define ifDebug(x)

namespace KDevelop {
ConfigurableHighlightingColors::ConfigurableHighlightingColors()
{
    reset(nullptr, nullptr);
}

ConfigurableHighlightingColors::~ConfigurableHighlightingColors() = default;

KTextEditor::Attribute::Ptr ConfigurableHighlightingColors::attribute(CodeHighlightingType type) const
{
    return m_attributes[type];
}

void ConfigurableHighlightingColors::reset(ColorCache* cache, KTextEditor::View* view)
{
    m_attributes.clear();
    auto createAttribute = [&](CodeHighlightingType type) {
        KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
        m_attributes[type] = a;
        return a;
    };
    auto addColor = [&](CodeHighlightingType type, QRgb color_) {
        auto a = createAttribute(type);
        auto color = QColor::fromRgb(color_);
        a->setForeground(cache ? cache->blendGlobalColor(color) : color);
        ifDebug(qCDebug(LANGUAGE) << #type << "color: " << #color_ << "->" << a->foreground().color().name());
    };
    // TODO: The set of colors doesn't work very well. Many colors simply too dark (even on the maximum "Global colorization intensity" they hardly distinguishable from grey) and look alike.
    addColor(CodeHighlightingType::Class, 0x005912); // Dark green
    addColor(CodeHighlightingType::TypeAlias, 0x35938d);
    addColor(CodeHighlightingType::Enum, 0x6c101e); // Dark red
    addColor(CodeHighlightingType::Enumerator, 0x862a38); // Greyish red
    addColor(CodeHighlightingType::Function, 0x21005A); // Navy blue
    addColor(CodeHighlightingType::MemberVariable, 0x443069); // Dark Burple (blue/purple)
    addColor(CodeHighlightingType::LocalClassMember, 0xae7d00); // Light orange
    addColor(CodeHighlightingType::LocalMemberFunction, 0xae7d00);
    addColor(CodeHighlightingType::InheritedClassMember, 0x705000); // Dark orange
    addColor(CodeHighlightingType::InheritedMemberFunction, 0x705000);
    addColor(CodeHighlightingType::LocalVariable, 0x0C4D3C);
    addColor(CodeHighlightingType::FunctionVariable, 0x300085); // Less dark navy blue
    addColor(CodeHighlightingType::NamespaceVariable, 0x9F3C5F); // Rose
    addColor(CodeHighlightingType::GlobalVariable, 0x12762B); // Grass green
    addColor(CodeHighlightingType::Namespace, 0x6B2840); // Dark rose
    addColor(CodeHighlightingType::ForwardDeclaration, 0x5C5C5C); // Gray
    addColor(CodeHighlightingType::Macro, 0xA41239);
    addColor(CodeHighlightingType::MacroFunctionLike, 0x008080);

    {
        auto highlightUses = createAttribute(CodeHighlightingType::HighlightUses);
        highlightUses->setDefaultStyle(KSyntaxHighlighting::Theme::TextStyle::Normal);
        highlightUses->setForeground(highlightUses->selectedForeground());
        highlightUses->setBackground(highlightUses->selectedBackground());
        highlightUses->setBackgroundFillWhitespace(true);
        if (view) {
            const auto searchHighlight = view->theme().editorColor(KSyntaxHighlighting::Theme::SearchHighlight);
            highlightUses->setBackground(QColor::fromRgb(searchHighlight));
        }
    }
    {
        auto error = createAttribute(CodeHighlightingType::Error);
        error->setDefaultStyle(KSyntaxHighlighting::Theme::TextStyle::Error);
    }
}
}
