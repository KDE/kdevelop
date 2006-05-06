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

#include <ktexteditor/smartrange.h>

#include "parser/codemodel.h"

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
    case Definition:
      a = m_definitionAttributes[type];
      break;

    case Declaration:
      a = m_declarationAttributes[type];
      break;

    case Reference:
      a = m_referenceAttributes[type];
      break;
  }

  if (!a) {
    a = KTextEditor::Attribute::Ptr(new KTextEditor::Attribute());
    a->setBackgroundFillWhitespace(true);
    switch (context) {
      case Definition:
        m_definitionAttributes.insert(type, a);
        break;

      case Declaration:
        m_declarationAttributes.insert(type, a);
        break;

      case Reference:
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
        KTextEditor::Attribute::Ptr e(new KTextEditor::Attribute());
        e->setForeground(Qt::green);
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

      case VariableType:
        a->setBackground(QColor(Qt::green).light());
        break;

      case NamespaceType:
        a->setForeground(QColor(Qt::blue).dark());
        break;

      case ScopeType:
      case TemplateType:
      case TemplateParameterType:
      case TypeAliasType:
      case MemberType:
      case CodeType:
      case EnumType:
      case EnumeratorType:
      case FileType:
        break;
    }

    switch (context) {
      case Definition:
        a->setFontBold();
        break;

      case Declaration:
        a = m_declarationAttributes[type];
        break;

      case Reference:
        a->setFontUnderline(true);

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
    else if (dynamic_cast<const _VariableModelItem*>(c))
      type = VariableType;
    else if (dynamic_cast<const _MemberModelItem*>(c))
      type = MemberType;

    foreach (KTextEditor::SmartRange* sr, c->references())
      sr->setAttribute(attributeForType(type, Reference));
    if (c->definition())
      c->definition()->setAttribute(attributeForType(type, Definition));
    if (c->declaration())
      c->declaration()->setAttribute(attributeForType(type, Declaration));

    highlightModel(model, index);
  }
}

void CppHighlighting::highlightTree( KTextEditor::SmartRange * range ) const
{
  int depth = range->depth();
  while (depth >= m_depthAttributes.count()) {
    KTextEditor::Attribute::Ptr a(new KTextEditor::Attribute());
    a->setBackground(QColor(Qt::white).dark(100 + (m_depthAttributes.count() * 25)));
    if (depth % 2)
      a->setOutline(Qt::red);
    m_depthAttributes.append(a);
  }
  range->setAttribute(m_depthAttributes[depth]);
  foreach (KTextEditor::SmartRange* child, range->childRanges())
    highlightTree(child);
}

#include "cpphighlighting.moc"

// kate: space-indent on; indent-width 2; replace-tabs on
