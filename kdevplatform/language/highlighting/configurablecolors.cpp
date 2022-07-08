/*
    SPDX-FileCopyrightText: 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2009 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "configurablecolors.h"

#include "codehighlighting.h"
#include "colorcache.h"

#include <debug.h>

#define ifDebug(x)

namespace KDevelop {
KTextEditor::Attribute::Ptr ConfigurableHighlightingColors::defaultAttribute() const
{
    return m_defaultAttribute;
}

void ConfigurableHighlightingColors::setDefaultAttribute(const KTextEditor::Attribute::Ptr& defaultAttrib)
{
    m_defaultAttribute = defaultAttrib;
}

KTextEditor::Attribute::Ptr ConfigurableHighlightingColors::attribute(CodeHighlightingType type) const
{
    return m_attributes[type];
}

void ConfigurableHighlightingColors::addAttribute(CodeHighlightingType type,
                                                  const KTextEditor::Attribute::Ptr& attribute)
{
    m_attributes[type] = attribute;
}

ConfigurableHighlightingColors::ConfigurableHighlightingColors()
{
    KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
    setDefaultAttribute(a);
}

#define ADD_COLOR(type, color_)                                                                                        \
    {                                                                                                                  \
        KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);                                                     \
        a->setForeground(QColor(cache->blendGlobalColor(color_)));                                                     \
        addAttribute(CodeHighlightingType::type, a);                                                                   \
        ifDebug(qCDebug(LANGUAGE) << #type << "color: " << #color_ << "->" << a->foreground().color().name();)         \
    }

CodeHighlightingColors::CodeHighlightingColors(ColorCache* cache) : ConfigurableHighlightingColors()
{
    // TODO: The set of colors doesn't work very well. Many colors simply too dark (even on the maximum "Global colorization intensity" they hardly distinguishable from grey) and look alike.
    ADD_COLOR(Class, 0x005912) // Dark green
    ADD_COLOR(TypeAlias, 0x35938d)
    ADD_COLOR(Enum, 0x6c101e) // Dark red
    ADD_COLOR(Enumerator, 0x862a38) // Greyish red
    ADD_COLOR(Function, 0x21005A) // Navy blue
    ADD_COLOR(MemberVariable, 0x443069) // Dark Burple (blue/purple)
    ADD_COLOR(LocalClassMember, 0xae7d00) // Light orange
    ADD_COLOR(InheritedClassMember, 0x705000) // Dark orange
    ADD_COLOR(LocalVariable, 0x0C4D3C)
    ADD_COLOR(FunctionVariable, 0x300085) // Less dark navy blue
    ADD_COLOR(NamespaceVariable, 0x9F3C5F) // Rose
    ADD_COLOR(GlobalVariable, 0x12762B) // Grass green
    ADD_COLOR(Namespace, 0x6B2840) // Dark rose
    ADD_COLOR(ErrorVariable, 0x8b0019) // Pure red
    ADD_COLOR(ForwardDeclaration, 0x5C5C5C) // Gray
    ADD_COLOR(Macro, 0xA41239)
    ADD_COLOR(MacroFunctionLike, 0x008080)
}
}
