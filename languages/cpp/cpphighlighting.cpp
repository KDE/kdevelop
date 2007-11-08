/*
 * KDevelop C++ Highlighting Support
 *
 * Copyright 2006 Hamish Rodda <rodda@kde.org>
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

#include "cpphighlighting.h"

#include <ktexteditor/smartrange.h>

#include <topducontext.h>
#include <declaration.h>
#include <definition.h>
#include <use.h>
#include "cpptypes.h"
#include <duchain.h>
#include <duchainlock.h>

using namespace KTextEditor;
using namespace KDevelop;

CppHighlighting::CppHighlighting( QObject * parent )
  : QObject(parent)
{
}

CppHighlighting::~CppHighlighting( )
{
}

KTextEditor::Attribute::Ptr CppHighlighting::attributeForType( Types type, Contexts context ) const
{
  KTextEditor::Attribute::Ptr a;
  switch (context) {
    case DefinitionContext:
      a = m_definitionAttributes[type];
      break;

    case DeclarationContext:
      a = m_declarationAttributes[type];
      break;

    case ReferenceContext:
      a = m_referenceAttributes[type];
      break;
  }

  if (!a) {
    a = KTextEditor::Attribute::Ptr(new KTextEditor::Attribute());
    a->setBackgroundFillWhitespace(true);
    switch (context) {
      case DefinitionContext:
        m_definitionAttributes.insert(type, a);
        break;

      case DeclarationContext:
        m_declarationAttributes.insert(type, a);
        break;

      case ReferenceContext:
        m_referenceAttributes.insert(type, a);
        break;
    }

    switch (type) {
      case UnknownType:
        // Chocolate orange
        a->setForeground(QColor(0xA0320A));
        break;

      case ArgumentType:
        // Steel
        a->setBackground(QColor(0x435361));
        break;

      case ClassType: {
        // Scarlet
        a->setForeground(QColor(0x7B0859));

        KTextEditor::Attribute::Ptr e(new KTextEditor::Attribute());
        e->setForeground(QColor(0x005500));
        a->setDynamicAttribute(Attribute::ActivateCaretIn, e);
        //a->setEffects(Attribute::EffectFadeIn | Attribute::EffectFadeOut);
        break;
      }

      case FunctionDefinitionType:
        //a->setFontBold();
        // fallthrough

      case FunctionType:
        // Navy blue
        a->setForeground(QColor(0x21005A));
        break;

      case MemberVariableType:
        // Dark Burple (blue / purple)
        a->setForeground(QColor(0x443069));
        break;

      case LocalVariableType:
        // Dark aquamarine
        a->setForeground(QColor(0x0C4D3C));
        break;

      case FunctionVariableType:
        // Less dark navy blue
        a->setForeground(QColor(0x300085));
        break;

      case NamespaceVariableType:
        // Rose
        a->setForeground(QColor(0x9F3C5F));
        break;

      case GlobalVariableType:
        // Grass green
        a->setForeground(QColor(0x12762B));
        break;

      case NamespaceType:
        // Dark rose
        a->setForeground(QColor(0x6B2840));
        break;

      case ErrorVariableType:
        // Slightly less intense red
        a->setForeground(QColor(0x9F3C5F));
        break;

      case ForwardDeclarationType:
        // Gray
        a->setForeground(QColor(0x5C5C5C));
        break;

      case ScopeType:
      case TemplateType:
      case TemplateParameterType:
      case TypeAliasType:
      case CodeType:
      case EnumType:
      case EnumeratorType:
      case FileType:
        break;
    }

    switch (context) {
      case DefinitionContext:
        a->setFontBold();
        break;

      case DeclarationContext:
        //a->setFontBold();
        a->setFontUnderline(true);
        break;

      case ReferenceContext:
        KTextEditor::Attribute::Ptr d(new KTextEditor::Attribute());
        d->setBackground(QColor(Qt::blue).light(190));
        d->setEffects(Attribute::EffectFadeIn | Attribute::EffectFadeOut);
        a->setDynamicAttribute(Attribute::ActivateMouseIn, d);
        break;
    }
  }

  return a;
}

void CppHighlighting::highlightTree( KTextEditor::SmartRange * range ) const
{
  int depth = range->depth();
  range->setAttribute(m_depthAttributes[depth]);
  foreach (KTextEditor::SmartRange* child, range->childRanges())
    highlightTree(child);
}

void CppHighlighting::outputRange( KTextEditor::SmartRange * range ) const
{
  kDebug( 9007 ) << range << QString(range->depth(), ' ') << *range << "attr" << range->attribute();
  Q_ASSERT(range->start() <= range->end());
  foreach (SmartRange* child, range->childRanges())
    outputRange(child);
}

void CppHighlighting::highlightDUChain(TopDUContext* context) const
{
  kDebug( 9007 ) << "highighting du chain";
  DUChainReadLocker lock(DUChain::lock());
  Q_ASSERT(context->topContext() == context);
  highlightDUChain(static_cast<DUContext*>(context));
}

void CppHighlighting::highlightDUChain(DUContext* context) const
{
  if (!context->smartRange())
    return;

  foreach (Declaration* dec, context->localDeclarations())
    highlightDeclaration(dec);

  foreach (Definition* def, context->localDefinitions())
    highlightDefinition(def);

  foreach (Use* use, context->uses())
    highlightUse(use);

  foreach (DUContext* child, context->childContexts())
    highlightDUChain(child);
}

KTextEditor::Attribute::Ptr CppHighlighting::attributeForDepth(int depth) const
{
  while (depth >= m_depthAttributes.count()) {
    KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute());
    a->setBackground(QColor(Qt::white).dark(100 + (m_depthAttributes.count() * 25)));
    a->setBackgroundFillWhitespace(true);
    if (depth % 2)
      a->setOutline(Qt::red);
    m_depthAttributes.append(a);
  }

  return m_depthAttributes[depth];
}

#include "cpphighlighting.moc"


CppHighlighting::Types CppHighlighting::typeForDeclaration(Declaration * dec) const
{
  Types type = LocalVariableType;
  if (dec->context()->scopeIdentifier().isEmpty())
    type = GlobalVariableType;
  if (dec->isForwardDeclaration())
    type = ForwardDeclarationType;
  else if (dec->type<CppClassType>())
    type = ClassType;
  else if (dec->type<CppFunctionType>())
    type = FunctionType;
  else
    switch (dec->context()->type()) {
      case DUContext::Namespace:
        type = NamespaceVariableType;
        break;
      case DUContext::Class:
        type = MemberVariableType;
        break;
      case DUContext::Function:
        type = FunctionVariableType;
        break;
      default:
        break;
    }

  return type;
}

void CppHighlighting::highlightDefinition(Definition * definition) const
{
  if (Declaration* declaration = definition->declaration())
    if (SmartRange* range = definition->smartRange())
      range->setAttribute(attributeForType(typeForDeclaration(declaration), DeclarationContext));
}

void CppHighlighting::highlightDeclaration(Declaration * declaration) const
{
  if (SmartRange* range = declaration->smartRange())
    range->setAttribute(attributeForType(typeForDeclaration(declaration), DeclarationContext));
}

void CppHighlighting::highlightUse(Use * use) const
{
  if (SmartRange* range = use->smartRange()) {
    Types type = ErrorVariableType;
    if (use->declaration())
      type = typeForDeclaration(use->declaration());

    range->setAttribute(attributeForType(type, ReferenceContext));
  }
}
