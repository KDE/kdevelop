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

CodeHighlightingColors::CodeHighlightingColors(ColorCache* cache) : ConfigurableHighlightingColors("KDev Semantic Highlighting")
{
  {
    KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
    a->setForeground(QColor(cache->blendGlobalColor(0x005912))); //Dark green
    addAttribute(CodeHighlighting::ClassType, a);
    kDebug() << "Class color: " << (void*) 0x005912 << "->" << (void*) a->foreground().color().rgb();
  }
  {
    KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
    a->setForeground(QColor(cache->blendGlobalColor(0x35938d)));
    addAttribute(CodeHighlighting::TypeAliasType, a);
    kDebug() << "TypeAlias color: " << (void*) 0x35938d << "->" << (void*) a->foreground().color().rgb();
  }
  {
    KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
    a->setForeground(QColor(cache->blendGlobalColor(0x6c101e))); //Dark red
    addAttribute(CodeHighlighting::EnumType, a);
    kDebug() << "Enum color: " << (void*) 0x6c101e << "->" << (void*) a->foreground().color().rgb();
  }
  {
    KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
    a->setForeground(QColor(cache->blendGlobalColor(0x862a38))); //Greyish red
    addAttribute(CodeHighlighting::EnumeratorType, a);
    kDebug() << "Enumerator color: " << (void*) 0x862a38 << "->" << (void*) a->foreground().color().rgb();
  }
  {
    KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
    a->setForeground(QColor(cache->blendGlobalColor(0x21005A))); // Navy blue
    addAttribute(CodeHighlighting::FunctionType, a);
    kDebug() << "Function color: " << (void*) 0x21005A << "->" << (void*) a->foreground().color().rgb();
  }
  {
    KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
    a->setForeground(QColor(cache->blendGlobalColor(0x443069)));// Dark Burple (blue / purple)
    addAttribute(CodeHighlighting::MemberVariableType, a);
    kDebug() << "MemberVariable color: " << (void*) 0x443069 << "->" << (void*) a->foreground().color().rgb();
  }
  {
    KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
    a->setForeground(QColor(cache->blendGlobalColor(0xae7d00))); //Light orange
    addAttribute(CodeHighlighting::LocalClassMemberType, a);
    kDebug() << "LocalClassMember color: " << (void*) 0xae7d00 << "->" << (void*) a->foreground().color().rgb();
  }
  {
    KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
    a->setForeground(QColor(cache->blendGlobalColor(0x705000))); //Dark orange
    addAttribute(CodeHighlighting::InheritedClassMemberType, a);
    kDebug() << "InheritedClassMember color: " << (void*) 0x705000 << "->" << (void*) a->foreground().color().rgb();
  }
  {
    KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
    a->setForeground(QColor(cache->blendGlobalColor(0x0C4D3C)));
    addAttribute(CodeHighlighting::LocalVariableType, a);
    kDebug() << "LocalVariable color: " << (void*) 0x0C4D3C << "->" << (void*) a->foreground().color().rgb();
  }
  {
    KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
    a->setForeground(QColor(cache->blendGlobalColor(0x300085))); // Less dark navy blue
    addAttribute(CodeHighlighting::FunctionVariableType, a);
    kDebug() << "FunctionVariable color: " << (void*) 0x300085 << "->" << (void*) a->foreground().color().rgb();
  }
  {
    KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
    a->setForeground(QColor(cache->blendGlobalColor(0x9F3C5F))); // Rose
    addAttribute(CodeHighlighting::NamespaceVariableType, a);
    kDebug() << "NamespaceVariable color: " << (void*) 0x9F3C5F << "->" << (void*) a->foreground().color().rgb();
  }
  {
    KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
    a->setForeground(QColor(cache->blendGlobalColor(0x12762B))); // Grass green
    addAttribute(CodeHighlighting::GlobalVariableType, a);
    kDebug() << "GlobalVariable color: " << (void*) 0x12762B << "->" << (void*) a->foreground().color().rgb();
  }
  {
    KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
    a->setForeground(QColor(cache->blendGlobalColor(0x6B2840))); // Dark rose
    addAttribute(CodeHighlighting::NamespaceType, a);
    kDebug() << "Namespace color: " << (void*) 0x6B2840 << "->" << (void*) a->foreground().color().rgb();
  }
  {
    KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute(*defaultAttribute()));
//     a->setForeground(QColor(0x8b0019)); // Pure red
    a->setUnderlineColor(QColor(cache->blendGlobalColor(0x8b0019))); // Pure red
    a->setUnderlineStyle(QTextCharFormat::WaveUnderline);
    addAttribute(CodeHighlighting::ErrorVariableType, a);
    kDebug() << "ErrorVariable underline color: " << (void*) 0x8b0019 << "->" << (void*) a->underlineColor().rgb();
  }
  {
    KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute);
    a->setForeground(QColor(cache->blendGlobalColor(0x5C5C5C))); // Gray
    addAttribute(CodeHighlighting::ForwardDeclarationType, a);
    kDebug() << "ForwardDeclaration color: " << (void*) 0x5C5C5C << "->" << (void*) a->foreground().color().rgb();
  }
/*      case ScopeType:
    case TemplateType:
    case TemplateParameterType:
    case CodeType:
    case FileType:*/
}

}

// kate: space-indent on; indent-width 2; replace-trailing-space-save on; show-tabs on; tab-indents on; tab-width 2;
