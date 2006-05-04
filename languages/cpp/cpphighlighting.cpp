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

#include <ktexteditor/attribute.h>
#include <ktexteditor/smartrange.h>

#include "parser/codemodel.h"

using namespace KTextEditor;

CppHighlighting::CppHighlighting( QObject * parent )
  : QObject(parent)
{
}

CppHighlighting::~ CppHighlighting( )
{
  qDeleteAll(m_definitionAttributes);
  qDeleteAll(m_declarationAttributes);
  qDeleteAll(m_referenceAttributes);
  qDeleteAll(m_depthAttributes);
}

KTextEditor::Attribute * CppHighlighting::attributeForType( Types type, Contexts context ) const
{
  KTextEditor::Attribute* a = 0L;
  switch (context) {
    case Definition:
      return 0L;
      a = m_definitionAttributes[type];
      break;

    case Declaration:
      return 0L;
      a = m_declarationAttributes[type];
      break;

    case Reference:
      a = m_referenceAttributes[type];
      break;
  }

  if (!a) {
    a = new KTextEditor::Attribute();
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
        KTextEditor::Attribute* d = new KTextEditor::Attribute();
        d->setBackground(Qt::blue);
        //d->setForeground(Qt::white);
        d->setTextOutline(QPen(Qt::white));
        a->setDynamicAttribute(Attribute::ActivateMouseIn, d, true);
        a->setEffects(Attribute::EffectFadeIn | Attribute::EffectFadeOut);
        //a->setBackground(QColor(Qt::yellow).light(175));
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
        return 0L;
        a->setFontBold();
        break;

      case Declaration:
        return 0L;
        //a = m_declarationAttributes[type];
        break;

      case Reference:
        a->setFontUnderline(true);
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
      sr->setAttribute(attributeForType(FunctionType, Reference));
    if (c->definition())
      c->definition()->setAttribute(attributeForType(FunctionType, Definition));
    if (c->declaration())
      c->declaration()->setAttribute(attributeForType(FunctionType, Declaration));

    highlightModel(model, index);
  }
}

void CppHighlighting::highlightTree( KTextEditor::SmartRange * range ) const
{
  int depth = range->depth();
  while (depth >= m_depthAttributes.count()) {
    KTextEditor::Attribute* a = new KTextEditor::Attribute();
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
