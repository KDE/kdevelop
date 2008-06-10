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
#include <ktexteditor/codecompletionmodel.h>

#include <duchainpointer.h>
#include <codecompletion/codecompletionitem.h>
#include "codecompletioncontext.h"
#include "includeitem.h"

namespace Cpp {
  class CodeCompletionContext;
}

namespace KTextEditor {
  class CodeCompletionModel;
  class Document;
  class Range;
  class Cursor;
}

class QModelIndex;
class CppCodeCompletionModel;

//A completion item used for completion of normal declarations while normal code-completion
class NormalDeclarationCompletionItem : public KDevelop::CompletionTreeItem {
public:
  NormalDeclarationCompletionItem(KDevelop::DeclarationPointer decl = KDevelop::DeclarationPointer(), KSharedPtr<Cpp::CodeCompletionContext> context=KSharedPtr<Cpp::CodeCompletionContext>(), int _inheritanceDepth = 0, int _listOffset=0) : declaration(decl), completionContext(context), m_inheritanceDepth(_inheritanceDepth), listOffset(_listOffset) {
  }
  
  virtual void execute(KTextEditor::Document* document, const KTextEditor::Range& word);

  virtual QVariant data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const;

  KDevelop::DeclarationPointer declaration;
  KSharedPtr<Cpp::CodeCompletionContext> completionContext;
  int m_inheritanceDepth; //Inheritance-depth: 0 for local functions(within no class), 1 for within local class, 1000+ for global items.
  int listOffset; //If it is an argument-hint, this contains the offset within the completion-context's function-list
  QString alternativeText; //Text shown when declaration is zero
  //If this is a completion for an include-file, this contains the file.

  virtual int inheritanceDepth() const;
  virtual int argumentHintDepth() const;
};

//A completion item used for completing include-files
class IncludeFileCompletionItem : public KDevelop::CompletionTreeItem {
public:
  IncludeFileCompletionItem(const Cpp::IncludeItem& include) : includeItem(include) {
  }

  virtual QVariant data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const;

  virtual void execute(KTextEditor::Document* document, const KTextEditor::Range& word);

  virtual int inheritanceDepth() const;
  virtual int argumentHintDepth() const;

  Cpp::IncludeItem includeItem;
};

#endif
