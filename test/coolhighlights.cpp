/* This file is part of the KDE libraries
   Copyright (C) 2005 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "coolhighlights.h"

#include <ktexteditor/attribute.h>

using namespace KTextEditor;

Attribute * CoolHighlights::classHighlight( )
{
  static Attribute* a = 0L;
  if (!a) {
    a = new Attribute();
    a->setBackground(QColor(Qt::yellow).light(175));
  }
  return a;
}

KTextEditor::Attribute * CoolHighlights::variableHighlight( )
{
  static Attribute* a = 0L;
  if (!a) {
    a = new Attribute();
    a->setBackground(QColor(Qt::green).light());
  }
  return a;
}

KTextEditor::Attribute * CoolHighlights::methodHighlight( )
{
  static Attribute* a = 0L;
  if (!a) {
    a = new Attribute();
    a->setBackground(QColor(Qt::blue).light(175));
  }
  return a;
}

KTextEditor::Attribute * CoolHighlights::variableDefinitionHighlight( )
{
  static Attribute* a = 0L;
  if (!a) {
    a = new Attribute();
    a->setBackground(QColor(Qt::green).light(175));
  }
  return a;
}

KTextEditor::Attribute * CoolHighlights::nextHighlight( )
{
  static Attribute* a = 0L;
  if (!a) {
    a = new Attribute();
    a->setBackground(QColor(Qt::red).light(175));
  }
  return a;
}

KTextEditor::Attribute * CoolHighlights::depthHighlight( int depth )
{
  static QList<Attribute*> a;
  if (a.count() < depth + 1) {
    for (int i = a.count(); i < depth + 1; ++i) {
      Attribute* b = new Attribute();
      b->setBackground(QColor(Qt::white).dark(100 + (depth * 5)));
      a << b;
    }
  }
  return a[depth];
}
