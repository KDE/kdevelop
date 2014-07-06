/*
 * KDevelop Generic Code Completion Support
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

#ifndef KDEVPLATFORM_NORMALDECLARATIONCOMPLETIONITEM_H
#define KDEVPLATFORM_NORMALDECLARATIONCOMPLETIONITEM_H

#include "codecompletionitem.h"
#include <language/languageexport.h>

namespace KDevelop {

class KDEVPLATFORMLANGUAGE_EXPORT NormalDeclarationCompletionItem : public CompletionTreeItem
{
public:
  NormalDeclarationCompletionItem(KDevelop::DeclarationPointer decl = KDevelop::DeclarationPointer(), QExplicitlySharedDataPointer<CodeCompletionContext> context=QExplicitlySharedDataPointer<CodeCompletionContext>(), int inheritanceDepth = 0);
  virtual KDevelop::DeclarationPointer declaration() const;
  QExplicitlySharedDataPointer<CodeCompletionContext> completionContext() const;
  virtual int inheritanceDepth() const;
  virtual int argumentHintDepth() const;
  QVariant data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const;

  void execute(KTextEditor::Document* document, const KTextEditor::Range& word);

protected:
  virtual QString declarationName() const;
  virtual QWidget* createExpandingWidget(const KDevelop::CodeCompletionModel* model) const;
  virtual bool createsExpandingWidget() const;
  virtual QString shortenedTypeString(KDevelop::DeclarationPointer decl, int desiredTypeLength) const;

  /**
   * Called after execute, you may insert additional chars for this declaration (eg. parens)
   * Default implementation does nothing
   */
  virtual void executed(KTextEditor::Document* document, const KTextEditor::Range& word);

  QExplicitlySharedDataPointer<CodeCompletionContext> m_completionContext;
  KDevelop::DeclarationPointer m_declaration;
  int m_inheritanceDepth; //Inheritance-depth: 0 for local functions(within no class), 1 for within local class, 1000+ for global items.
  
  static const int normalBestMatchesCount;
  static const bool shortenArgumentHintReturnValues;
  static const int maximumArgumentHintReturnValueLength;
  static const int desiredTypeLength;
  
};

}
#endif
