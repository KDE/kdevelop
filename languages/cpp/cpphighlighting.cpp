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
  qDeleteAll(m_attributes);
}

KTextEditor::Attribute * CppHighlighting::attributeForType( Types type ) const
{
  KTextEditor::Attribute* a = m_attributes[type];

  if (!a) {
    a = new KTextEditor::Attribute();
    m_attributes.insert(type, a);

    switch (type) {
      case UnknownType:
        a->setForeground(QColor(0xBF3E0D));
        break;

      case ArgumentType:
        a->setBackground(QColor(Qt::green).light(175));
        break;

      case ClassType:
        a->setBackground(QColor(Qt::yellow).light(175));
        break;

      case FunctionDefinitionType:
        a->setFontBold();
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
    if (const _ClassModelItem* c = dynamic_cast<const _ClassModelItem*>(item))
      foreach (KTextEditor::SmartRange* sr, c->references())
        sr->setAttribute(attributeForType(ClassType));

    else if (const KDevCodeItem* c = dynamic_cast<const KDevCodeItem*>(item))
      foreach (KTextEditor::SmartRange* sr, c->references())
        sr->setAttribute(attributeForType(UnknownType));

    highlightModel(model, index);
  }
}


#include "cpphighlighting.moc"

// kate: space-indent on; indent-width 2; replace-tabs on
