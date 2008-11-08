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

#include "./implementationhelperitem.h"
#include <ktexteditor/document.h>
#include <klocalizedstring.h>

ImplementationHelperItem::ImplementationHelperItem(KDevelop::DeclarationPointer decl, KSharedPtr<Cpp::CodeCompletionContext> context, int _inheritanceDepth, int _listOffset) : NormalDeclarationCompletionItem(decl, context, _inheritanceDepth, _listOffset) {
}

QVariant ImplementationHelperItem::data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const {
  QVariant ret = NormalDeclarationCompletionItem::data(index, role, model);
  if(index.column() == KTextEditor::CodeCompletionModel::Prefix && role == Qt::DisplayRole) {
    ret = i18n("Override") + " " + ret.toString();
  }
  if(role == KTextEditor::CodeCompletionModel::InheritanceDepth)
    return QVariant(0);
  return ret;
}

void ImplementationHelperItem::execute(KTextEditor::Document* document, const KTextEditor::Range& word) {
  QString newText;

  if(!useAlternativeText) {
    KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
    if(m_declaration) {
      newText = "virtual " + m_declaration->toString();
      if(m_declaration->abstractType() && m_declaration->abstractType()->modifiers() & AbstractType::ConstModifier)
        newText += " const";
      newText += ";";
    } else {
      kDebug() << "Declaration disappeared";
      return;
    }
  }else{
    newText = alternativeText;
  }

  document->replaceText(word, newText);
}
