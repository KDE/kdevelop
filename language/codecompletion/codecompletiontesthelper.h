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

#ifndef KDEVPLATFORM_CODECOMPLETIONTESTHELPER_H
#define KDEVPLATFORM_CODECOMPLETIONTESTHELPER_H

#include <QtTest/QtTest>
#include <QtGui/QStandardItemModel>

#include "../duchain/declaration.h"
#include "../duchain/duchain.h"
#include "codecompletionitem.h"
#include <language/codegen/coderepresentation.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/parsingenvironment.h>

#include <tests/testhelpers.h>

using namespace KTextEditor;

using namespace KDevelop;


QStandardItemModel& fakeModel() {
  static QStandardItemModel model;
  model.setColumnCount(10);
  model.setRowCount(10);
  return model;
}

/**
  * Helper-class for testing completion-items
  * Just initialize it with the context and the text, and then use the members, for simple cases only "names"
  * the template parameter is your language specific CodeCompletionContext
  */
template <class T>
struct CodeCompletionItemTester {
   
  typedef KSharedPtr< KDevelop::CompletionTreeElement > Element;
  
  //Creates a CodeCompletionItemTester for the parent context
  CodeCompletionItemTester parent() {
    KSharedPtr<T> parent = KSharedPtr<T>(dynamic_cast<T*>(completionContext->parentContext()));
    Q_ASSERT(parent);
    return CodeCompletionItemTester(parent);
  }

  //Standard constructor
  CodeCompletionItemTester(DUContext* context, QString text = "; ", QString followingText = "", CursorInRevision position = CursorInRevision::invalid()) {
    completionContext = new  T(DUContextPointer(context), text, followingText, position.isValid() ? position : context->range().end);

    init();
  }

  //Can be used if you already have the completion context
  CodeCompletionItemTester(KSharedPtr<T> context) {
    completionContext = context;
    
    init();
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

  bool containsDeclaration(Declaration* dec) const {
    foreach(Item item, items) {
        if (item->declaration().data() == dec) {
            return true;
        }
    }
    return false;
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
  private:
    void init() {
      if ( !completionContext->isValid() ) {
        kDebug() << "invalid completion context";
        return;
      }

      bool abort = false;
      items = completionContext->completionItems(abort);
      
      
      addElements(completionContext->ungroupedElements());
      
      foreach(Item i, items)
        names << i->data(fakeModel().index(0, KTextEditor::CodeCompletionModel::Name), Qt::DisplayRole, 0).toString();
    }
};

/**
 * Helper class that inserts the given text into the duchain under the specified name,
 * allows parsing it with a simple call to parse(), and automatically releases the top-context
 *
 * The duchain must not be locked when this object is destroyed
 */
struct InsertIntoDUChain
{
  ///Artificially inserts a file called @p name with the text @p text
  InsertIntoDUChain(QString name, QString text) : m_insertedCode(IndexedString(name), text), m_topContext(0) {
  }

  ~InsertIntoDUChain() {
    get();
    release();
  }
  
  ///The duchain must not be locked when this is called
  void release() {
    if(m_topContext) {
      DUChainWriteLocker lock;
      
      m_topContext = 0;
      
      QList< TopDUContext* > chains = DUChain::self()->chainsForDocument(m_insertedCode.file());
      foreach(TopDUContext* top, chains)
        DUChain::self()->removeDocumentChain(top);
    }
  }

  TopDUContext* operator->() {
    get();
    return m_topContext.data();
  }
  
  TopDUContext* tryGet() {
      DUChainReadLocker lock;
    return DUChain::self()->chainForDocument(m_insertedCode.file(), false);
  }
  
  void get() {
    if(!m_topContext)
      m_topContext = tryGet();
  }

  ///Helper function: get a declaration based on its qualified identifier
  Declaration* getDeclaration(QString id) {
    get();
    if(!topContext())
      return 0;
    return DeclarationId(IndexedQualifiedIdentifier(QualifiedIdentifier(id))).getDeclaration(topContext());
  }
  
  TopDUContext* topContext() {
    return m_topContext.data();
  }
  
  /**
    * Parses this inserted code as a stand-alone top-context
    * The duchain must not be locked when this is called
    *
    * @param features The features that should be requested for the top-context
    * @param update Whether the top-context should be updated if it already exists. Else it will be deleted.
    */
  void parse(uint features = TopDUContext::AllDeclarationsContextsAndUses, bool update = false) {
    
    if(!update)
      release();
    m_topContext = DUChain::self()->waitForUpdate(m_insertedCode.file(), (TopDUContext::Features)features, false);
    Q_ASSERT(m_topContext);
    DUChainReadLocker lock;
    Q_ASSERT(!m_topContext->parsingEnvironmentFile()->isProxyContext());
  }
  
  InsertArtificialCodeRepresentation m_insertedCode;
  ReferencedTopDUContext m_topContext;
};

#endif // KDEVPLATFORM_CODECOMPLETIONTESTHELPER_H
