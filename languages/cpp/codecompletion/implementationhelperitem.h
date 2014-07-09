/*
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

#ifndef IMPLEMENTATIONHELPERITEM_H
#define IMPLEMENTATIONHELPERITEM_H

#include "item.h"

using namespace KDevelop;

namespace Cpp {

/**
 * TODO: refactor the mess that is introduced by reusing this class without a CodeCompletionContext in the CppNewClass wizard
 * 
 * Especially the handling of the forcedParentIdentifier should be reworked, I don't like the implementation I hacked in
 * to support overriding of ctors there.
 * 
 * ~ Milian
 */
class ImplementationHelperItem : public NormalDeclarationCompletionItem
{
public:
  enum HelperType {
    Override,
    CreateDefinition,
    CreateSignalSlot
  };
  ImplementationHelperItem(HelperType type, KDevelop::DeclarationPointer decl = KDevelop::DeclarationPointer(), QExplicitlySharedDataPointer<Cpp::CodeCompletionContext> context=QExplicitlySharedDataPointer<Cpp::CodeCompletionContext>(), int _inheritanceDepth = 0, int _listOffset=0);
  
  virtual QVariant data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const;
  virtual void execute(KTextEditor::View* view, const KTextEditor::Range& word) override;
  //Arguments + "const"
  QString signaturePart(bool includeDefaultParams);
  HelperType m_type;
  
  ///Returns the text that this item would yield if it was executed
  QString insertionText(KUrl currentDocumentUrl = QUrl(), KTextEditor::Cursor currentDocumentCursor = KTextEditor::Cursor(), QualifiedIdentifier forceParentScope = QualifiedIdentifier());
  
  virtual bool dataChangedWithInput() const;

  private:
  ///DUChain must be read-locked
  QString getOverrideName(const QualifiedIdentifier& forcedParentIdentifier = QualifiedIdentifier()) const;
};

}

#endif // IMPLEMENTATIONHELPERITEM_H
