/*
 * KDevelop C++ Highlighting Support
 *
 * Copyright (c) 2006 Hamish Rodda <rodda@kde.org>
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

#include <QReadLocker>

#include <ktexteditor/smartrange.h>

#include "parser/codemodel.h"
#include "duchain/topducontext.h"
#include "duchain/declaration.h"
#include "duchain/definition.h"
#include "duchain/use.h"
#include "duchain/cpptypes.h"
#include "duchain/duchain.h"

using namespace KTextEditor;

CppHighlighting::CppHighlighting( QObject * parent )
  : QObject(parent)
{
}

CppHighlighting::~ CppHighlighting( )
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
        a->setForeground(QColor(0xBF3E0D));
        break;

      case ArgumentType:
        a->setBackground(QColor(Qt::green).light(175));
        break;

      case ClassType: {
        a->setForeground(Qt::blue);

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
        a->setBackground(QColor(Qt::blue).light(175));
        break;

      case MemberVariableType:
        a->setForeground(QColor(0xFF800D));
        break;

      case LocalVariableType:
        a->setForeground(QColor(0xA04F2C));
        break;

      case FunctionVariableType:
        a->setForeground(QColor(0x1F88A7));
        break;

      case NamespaceVariableType:
        a->setForeground(QColor(0xD568FD));
        break;

      case GlobalVariableType:
        a->setForeground(QColor(0x1FCB4A));
        break;

      case NamespaceType:
        a->setForeground(QColor(Qt::blue).dark());
        break;

      case ErrorVariableType:
        a->setForeground(Qt::red);
        break;

      case ForwardDeclarationType:
        a->setForeground(QColor(0x5C005C));
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

void CppHighlighting::highlightModel(CodeModel* model, const QModelIndex & parent) const
{
  int rowCount = 0;
  rowCount = model->rowCount(parent);

  for (int i = 0; i < rowCount; ++i) {
    QModelIndex index = model->index(i, 0, parent);
    const KDevItem* item = static_cast<const KDevCodeModel*>(index.model())->item(index);
    Types type = UnknownType;

    const KDevCodeItem* c = dynamic_cast<const KDevCodeItem*>(item);
    if (!c)
      continue;

    if (dynamic_cast<const _ClassModelItem*>(c))
      type = ClassType;
    else if (dynamic_cast<const _EnumModelItem*>(c))
      type = EnumType;
    else if (dynamic_cast<const _EnumeratorModelItem*>(c))
      type = EnumeratorType;
    else if (dynamic_cast<const _FileModelItem*>(c))
      type = FileType;
    else if (dynamic_cast<const _FunctionDefinitionModelItem*>(c))
      type = FunctionDefinitionType;
    else if (dynamic_cast<const _FunctionModelItem*>(c))
      type = FunctionType;
    else if (dynamic_cast<const _NamespaceModelItem*>(c))
      type = NamespaceType;
    else if (dynamic_cast<const _ScopeModelItem*>(c))
      type = ScopeType;
    else if (dynamic_cast<const _TemplateModelItem*>(c))
      type = TemplateType;
    else if (dynamic_cast<const _TemplateParameterModelItem*>(c))
      type = TemplateParameterType;
    else if (dynamic_cast<const _TypeAliasModelItem*>(c))
      type = TypeAliasType;

    foreach (KTextEditor::SmartRange* sr, c->references())
      sr->setAttribute(attributeForType(type, ReferenceContext));
    if (c->definition())
      c->definition()->setAttribute(attributeForType(type, DeclarationContext));
    if (c->declaration())
      c->declaration()->setAttribute(attributeForType(type, DeclarationContext));

    highlightModel(model, index);
  }
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
  kDebug() << range << QString(range->depth(), ' ') << *range << " attr " << range->attribute() << endl;
  Q_ASSERT(range->start() <= range->end());
  foreach (SmartRange* child, range->childRanges())
    outputRange(child);
}

void CppHighlighting::highlightDUChain(TopDUContext* context) const
{
  QReadLocker lock(DUChain::lock());
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

// kate: space-indent on; indent-width 2; replace-tabs on

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
