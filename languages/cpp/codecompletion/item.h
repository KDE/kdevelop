/*
 * KDevelop C++ Code Completion Support
 *
 * Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>
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

#ifndef COMPLETIONITEM_H
#define COMPLETIONITEM_H

#include <ksharedptr.h>
#include <language/duchain/duchainpointer.h>
#include <language/duchain/types/structuretype.h>
#include <language/codecompletion/codecompletionitem.h>
#include <language/codecompletion/normaldeclarationcompletionitem.h>
#include <language/codecompletion/abstractincludefilecompletionitem.h>
#include "../cppduchain/navigation/navigationwidget.h"

namespace KDevelop {
  class QualifiedIdentifier;
  class Identifier;
}

namespace KTextEditor {
  class CodeCompletionModel;
  class Document;
  class Range;
  class Cursor;
}

class QModelIndex;

struct CachedArgumentList : public QSharedData {
  QString text;
  QList<QVariant> highlighting;
};

namespace Cpp {
  class CodeCompletionContext;
  class CodeCompletionModel;

void setStaticMatchContext(QList<KDevelop::IndexedType> types);

void executeSignalSlotCompletionItem( KTextEditor::Document* document, const KTextEditor::Range& enteredWord,
                                      bool isSignal, const QString& name, const QString& signature );

//A completion item used for completion of normal declarations while normal code-completion
class NormalDeclarationCompletionItem : public KDevelop::NormalDeclarationCompletionItem {
public:
  NormalDeclarationCompletionItem(KDevelop::DeclarationPointer decl = KDevelop::DeclarationPointer(), KSharedPtr<KDevelop::CodeCompletionContext> context=KSharedPtr<KDevelop::CodeCompletionContext>(), int _inheritanceDepth = 0, int _listOffset=0)
    : KDevelop::NormalDeclarationCompletionItem(decl, context, _inheritanceDepth), useAlternativeText(false), prependScopePrefix(false), listOffset(_listOffset), m_isQtSignalSlotCompletion(false), m_isTemplateCompletion(false), m_fixedMatchQuality(-1) {
  }
  
  virtual void execute(KTextEditor::Document* document, const KTextEditor::Range& word);

  virtual QVariant data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const;
  
  //Prefix that will be stripped from all identifiers(For example the namespace)
  KDevelop::QualifiedIdentifier stripPrefix() const;
  
  bool completingTemplateParameters() const;
  
  mutable QString alternativeText; //Text shown when declaration is zero
  mutable QString prefixText; //Text prepended to declaration
  
  //If this is true, alternativeText will be shown in the list, and will be inserted on execution.
  //Also the scope will be set to LocalScope when this attribute is true.
  bool useAlternativeText;

  //If this is true, scope prefix will be prepended to text inserted on execution.
  bool prependScopePrefix;

  int listOffset; //If it is an argument-hint, this contains the offset within the completion-context's function-list

  //If this is true, the execution of the item should trigger the insertion of a complete SIGNAL/SLOT use
  bool m_isQtSignalSlotCompletion, m_isTemplateCompletion;

  //If this is not -1, it can be a fixed match-quality from 0 to 10, that will be used non-dynamically.
  //otherwise this is calculated and cached on the first read
  mutable int m_fixedMatchQuality;
  
  virtual KTextEditor::CodeCompletionModel::CompletionProperties completionProperties() const;

  KSharedPtr<CodeCompletionContext> completionContext() const;

protected:
  virtual QWidget* createExpandingWidget(const KDevelop::CodeCompletionModel* model) const;
  virtual bool createsExpandingWidget() const;
  virtual QString shortenedTypeString(KDevelop::DeclarationPointer decl, int desiredTypeLength) const;
private:
  
  void needCachedArgumentList() const;
  
  QString keepRemainingWord(const KDevelop::Identifier& id);
  QString keepRemainingWord(const KDevelop::StructureType::Ptr &type, const KDevelop::Identifier &id, const QString &insertAccessor);
  

  
  mutable KDevelop::DeclarationPointer m_cachedTypeStringDecl;
  mutable QString m_cachedTypeString;
  mutable uint m_cachedTypeStringLength;
  
  mutable KSharedPtr<CachedArgumentList> m_cachedArgumentList;
};

// Helper-item that manages the number of shown argument-hints
// When the user selects the item, the item stores an increased maximum in resetMaxArgumentHints()
// The number is reset after resetMaxArgumentHints() was called the next time.
class MoreArgumentHintsCompletionItem : public NormalDeclarationCompletionItem {
public:
  MoreArgumentHintsCompletionItem(KSharedPtr<KDevelop::CodeCompletionContext> context, QString text, uint oldNumber);
  
  virtual void execute(KTextEditor::Document* document, const KTextEditor::Range& word);
  
  // Maximum number of argument-hints that should be shown by code completion
  // Whenever this is called, the maximum number of arguments is reset afterwards,
  // so make sure you call it only once in the correct place.
  static uint resetMaxArgumentHints(bool isAutomaticCompletion);
private:
  uint m_oldNumber;
};

typedef KDevelop::AbstractIncludeFileCompletionItem<Cpp::NavigationWidget> BaseIncludeFileCompletionItem;

class IncludeFileCompletionItem : public BaseIncludeFileCompletionItem {
public:
    IncludeFileCompletionItem(const KDevelop::IncludeItem& include)
      : BaseIncludeFileCompletionItem(include) {}

    virtual void execute(KTextEditor::Document* document, const KTextEditor::Range& word);
};

class TypeConversionCompletionItem : public KDevelop::CompletionTreeItem {
  public:
    TypeConversionCompletionItem(QString text, KDevelop::IndexedType type, int argumentHintDepth, KSharedPtr<Cpp::CodeCompletionContext> completionContext);
    virtual int argumentHintDepth() const;
    virtual QVariant data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const;
    QList<KDevelop::IndexedType> type() const;
    void setPrefix(QString s);
    virtual void execute(KTextEditor::Document* document, const KTextEditor::Range& word);
  private:
    QString m_prefix;
    QString m_text;
    KDevelop::IndexedType m_type;
    int m_argumentHintDepth;
    KSharedPtr<Cpp::CodeCompletionContext> completionContext;
};

}

#endif
