/*
 * This file is part of KDevelop
 *
 * Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
 * Copyright 2006 Hamish Rodda <rodda@kde.org>
 * Copyright 2009 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "configurablecolors.h"
#include "util/debug.h"

#define ifDebug(x)

namespace KDevelop
{

KTextEditor::Attribute::Ptr ConfigurableHighlightingColors::defaultAttribute() const
{
  return m_defaultAttribute;
}

void ConfigurableHighlightingColors::setDefaultAttribute(KTextEditor::Attribute::Ptr defaultAttrib)
{
  m_defaultAttribute = defaultAttrib;
}

KTextEditor::Attribute::Ptr ConfigurableHighlightingColors::getAttribute(int number) const
{
  return m_attributes[number];
}

void ConfigurableHighlightingColors::addAttribute(int number, KTextEditor::Attribute::Ptr attribute)
{
  m_attributes[number] = attribute;
}

ConfigurableHighlightingColors::ConfigurableHighlightingColors(QString highlightingName) : m_highlightingName(highlightingName)
{
  KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
  setDefaultAttribute(a);
}

#define ADD_COLOR(type, color_) \
  { \
    KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute); \
    a->setForeground(QColor(cache->blendGlobalColor(color_)));  \
    addAttribute(CodeHighlighting::type, a);  \
    ifDebug(qCDebug(LANGUAGE) << #type << "color: " << ((void*) color_) << "->" << a->foreground().color().name();) \
  }

CodeHighlightingColors::CodeHighlightingColors(ColorCache* cache) : ConfigurableHighlightingColors("KDev Semantic Highlighting")
{
  ADD_COLOR(ClassType, 0x005912) //Dark green
  ADD_COLOR(TypeAliasType, 0x35938d)
  ADD_COLOR(EnumType, 0x6c101e) //Dark red
  ADD_COLOR(EnumeratorType, 0x862a38) //Greyish red
  ADD_COLOR(FunctionType, 0x21005A) //Navy blue
  ADD_COLOR(MemberVariableType, 0x443069) //Dark Burple (blue/purple)
  ADD_COLOR(LocalClassMemberType, 0xae7d00) //Light orange
  ADD_COLOR(InheritedClassMemberType, 0x705000) //Dark orange
  ADD_COLOR(LocalVariableType, 0x0C4D3C)
  ADD_COLOR(FunctionVariableType, 0x300085) //Less dark navy blue
  ADD_COLOR(NamespaceVariableType, 0x9F3C5F) //Rose
  ADD_COLOR(GlobalVariableType, 0x12762B) //Grass green
  ADD_COLOR(NamespaceType, 0x6B2840) //Dark rose
  ADD_COLOR(ErrorVariableType, 0x8b0019) //Pure red
  ADD_COLOR(ForwardDeclarationType, 0x5C5C5C) //Gray

/*      case ScopeType:
    case TemplateType:
    case TemplateParameterType:
    case CodeType:
    case FileType:*/
}

}

// kate: space-indent on; indent-width 2; replace-trailing-space-save on; show-tabs on; tab-indents on; tab-width 2;
