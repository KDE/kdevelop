/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda<rodda@kde.org>
    Copyright 2007-2009 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef CODECOMPLETIONTESTHELPER_H
#define CODECOMPLETIONTESTHELPER_H

#include <QtTest/QtTest>
#include <QtGui/QStandardItemModel>

#include "../duchain/declaration.h"
#include "../duchain/duchain.h"
#include "codecompletionitem.h"


using namespace KTextEditor;

using namespace KDevelop;


QStandardItemModel& fakeModel() {
  static QStandardItemModel model;
  model.setColumnCount(10);
  model.setRowCount(10);
  return model;
}

namespace QTest {
  template<>
  char* toString(const Cursor& cursor)
  {
    QByteArray ba = "Cursor(";
    ba += QByteArray::number(cursor.line()) + ", " + QByteArray::number(cursor.column());
    ba += ')';
    return qstrdup(ba.data());
  }
  template<>
  char* toString(const QualifiedIdentifier& id)
  {
    QByteArray arr = id.toString().toLatin1();
    return qstrdup(arr.data());
  }
  template<>
  char* toString(const Identifier& id)
  {
    QByteArray arr = id.toString().toLatin1();
    return qstrdup(arr.data());
  }
  /*template<>
  char* toString(QualifiedIdentifier::MatchTypes t)
  {
    QString ret;
    switch (t) {
      case QualifiedIdentifier::NoMatch:
        ret = "No Match";
        break;
      case QualifiedIdentifier::Contains:
        ret = "Contains";
        break;
      case QualifiedIdentifier::ContainedBy:
        ret = "Contained By";
        break;
      case QualifiedIdentifier::ExactMatch:
        ret = "Exact Match";
        break;
    }
    QByteArray arr = ret.toString().toLatin1();
    return qstrdup(arr.data());
  }*/
  template<>
  char* toString(const Declaration& def)
  {
    QString s = QString("Declaration %1 (%2): %3")
        .arg(def.identifier().toString())
        .arg(def.qualifiedIdentifier().toString())
        .arg(reinterpret_cast<long>(&def));
    return qstrdup(s.toLatin1().constData());
  }
  template<>
  char* toString(const TypePtr<AbstractType>& type)
  {
    QString s = QString("Type: %1")
        .arg(type ? type->toString() : QString("<null>"));
    return qstrdup(s.toLatin1().constData());
  }
}

//Helper-class for testing completion-items
//Just initialize it with the context and the text, and then use the members, for simple cases only "names"
//the template parameter is your language specific CodeCompletionContext
template <class T>
struct CodeCompletionItemTester {
   
  typedef KSharedPtr< KDevelop::CompletionTreeElement > Element;
    
  CodeCompletionItemTester(DUContext* context, QString text = "; ") {
    completionContext = new  T(DUContextPointer(context), text, QString(), context->range().end);
    bool abort = false;
    items = completionContext->completionItems(abort);
    
    
    addElements(completionContext->ungroupedElements());
    
    foreach(Item i, items)
      names << i->data(fakeModel().index(0, KTextEditor::CodeCompletionModel::Name), Qt::DisplayRole, 0).toString();
  }
  
  void addElements(QList<Element> elements) {
    foreach(Element element, elements) {
      Item item(dynamic_cast<CompletionTreeItem*>(element.data()));
      if(item)
        items << item;
      CompletionTreeNode* node = dynamic_cast<CompletionTreeNode*>(element.data());
      if(node)
        addElements(node->children);
    }
  }
  
  QStringList names; //Names of all completion-items, not sorted
  typedef KSharedPtr <KDevelop::CompletionTreeItem > Item;
  QList <Item > items; //All items retrieved, sorted by name

  KSharedPtr <T> completionContext;
  
  //Convenience-function to retrieve data from completion-items by name
  QVariant itemData(QString itemName, int column = KTextEditor::CodeCompletionModel::Name, int role = Qt::DisplayRole) {
    return itemData(names.indexOf(itemName), column, role);
  }
  
  QVariant itemData(int itemNumber, int column = KTextEditor::CodeCompletionModel::Name, int role = Qt::DisplayRole) {
    if(itemNumber < 0 || itemNumber >= items.size())
      return QVariant();
    
    return items[itemNumber]->data(fakeModel().index(0, column), role, 0);
  }
};

#endif // CODECOMPLETIONTESTHELPER_H
