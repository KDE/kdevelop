/*
   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "missingincludecompletionitem.h"
#include <language/duchain/namespacealiasdeclaration.h>
#include <language/duchain/persistentsymboltable.h>
#include <language/duchain/types/abstracttype.h>
#include <language/duchain/types/delayedtype.h>
#include <language/duchain/types/identifiedtype.h>
#include "cppduchain/navigationwidget.h"
#include "cppduchain/typeutils.h"
#include "cpplanguagesupport.h"
#include "cppcodecompletionmodel.h"
#include <klocale.h>
#include <ktexteditor/document.h>

using namespace KTextEditor;
using namespace KDevelop;

QualifiedIdentifier removeTemplateParameters(QualifiedIdentifier baseIdentifier) {
  QualifiedIdentifier  identifier;
  for(int a = 0; a < baseIdentifier.count(); ++a) {
    Identifier part = baseIdentifier.at(a);
    part.clearTemplateIdentifiers();
    identifier.push(part);
  }
  return identifier;
}

QList<KDevelop::CompletionTreeItemPointer> itemsForFile(QString displayTextPrefix, QString file, KUrl::List includePaths, KUrl currentPath, IndexedDeclaration decl, uint argumentHintDepth) {
  QList<KDevelop::CompletionTreeItemPointer> ret;
  //We have found a potential declaration. Now find the shortest include path.
  QString shortestDirective;
  bool isRelativeToCurrentDir = false;
  foreach(KUrl includePath, includePaths) {
    QString relative = KUrl::relativePath( QFileInfo(includePath.path()).canonicalFilePath(), QFileInfo(file).canonicalFilePath() );
    
    if(shortestDirective.isEmpty() || relative.length() < shortestDirective.length()) {
      shortestDirective = relative;
      if(shortestDirective.startsWith("./"))
        shortestDirective = shortestDirective.mid(2);
      
      isRelativeToCurrentDir = includePath.equals( currentPath );
    }
  }
  if(!shortestDirective.isEmpty()) {
    if(isRelativeToCurrentDir)
      shortestDirective = "\"" + shortestDirective + "\"";
    else
      shortestDirective = "<" + shortestDirective + ">";
    
    ret << KDevelop::CompletionTreeItemPointer(new MissingIncludeCompletionItem(shortestDirective, displayTextPrefix, decl, argumentHintDepth));
  }
  return ret;
}

QList<KDevelop::CompletionTreeItemPointer> missingIncludeCompletionItems(QString expression, QString displayTextPrefix, Cpp::ExpressionEvaluationResult expressionResult, KDevelop::DUContext* context, int argumentHintDepth, bool namespaceAllowed) {
  
  AbstractType::Ptr type = TypeUtils::targetType(expressionResult.type.type(), context->topContext());
  
  //Collect all visible "using namespace" imports
  QList<Declaration*> imports = context->findDeclarations( globalImportIdentifier );
  QSet<QualifiedIdentifier> prefixes;
  prefixes.insert(QualifiedIdentifier());
  foreach(Declaration* importDecl, imports) {
    NamespaceAliasDeclaration* aliasDecl = dynamic_cast<NamespaceAliasDeclaration*>(importDecl);
    if(aliasDecl) {
      prefixes.insert(aliasDecl->importIdentifier());
    }else{
      kDebug() << "Import is not based on NamespaceAliasDeclaration";
    }
  }
  
  QualifiedIdentifier namespaceScope = context->scopeIdentifier(false);
  for(int a = 1; a < namespaceScope.count(); ++a)
    prefixes << namespaceScope.mid(a); //Also search within enclosing namespaces
  
  QList<KDevelop::CompletionTreeItemPointer> ret;

  QualifiedIdentifier identifier;
  if(type) {
    DelayedType::Ptr delayed = type.cast<DelayedType>();
    if(delayed)
      //Remove all template parameters, because the symbol-table doesn't know about those
      identifier = removeTemplateParameters(delayed->identifier());
    IdentifiedType* idType = dynamic_cast<IdentifiedType*>(type.unsafeData());
    if(idType) {
      identifier = removeTemplateParameters(idType->qualifiedIdentifier());
    }
  }else{
    //expression probably contains a part that needs to be resolved
    
    if(expression.contains(".") || expression.contains("->")) {
      ///@todo Check if parts of the expression are unresolved, like in "unresolvedClass.callFunction"
      kDebug() << "doing nothing with expression" << expression;
    }else{
      kDebug() << "looking up" << expression << "as qualified identifier";
      identifier = removeTemplateParameters(QualifiedIdentifier(expression));
      QList<Declaration*> visibleDecls = context->findDeclarations(identifier);
      foreach(Declaration* decl, visibleDecls) {
        if(!decl->isForwardDeclaration())
          return ret; //Do not search for fitting declarations if a valid one is visible right now from here
      }
    }
  }
  
  if(identifier.isEmpty()) {
    return ret;
  }
  
  KUrl currentUrl(context->topContext()->url().str());
  KUrl currentPath(context->topContext()->url().str());
  currentPath.setFileName(QString());
  
  KUrl::List includePaths = CppLanguageSupport::self()->findIncludePaths(currentUrl, 0);
  includePaths.prepend(currentPath);
  
  ///Search the persistent symbol table
  foreach(QualifiedIdentifier prefix, prefixes) {
    prefix.setExplicitlyGlobal(false);
    const IndexedDeclaration* declarations;
    uint declarationCount;
    QualifiedIdentifier id = prefix + identifier;

    PersistentSymbolTable::self().declarations( id, declarationCount, declarations );
    for(uint a = 0; a < declarationCount; ++a) {
      Declaration* decl = declarations[a].declaration();
      if(decl && !decl->isForwardDeclaration()) {
        QString file(decl->topContext()->url().toUrl().path());
        ret += itemsForFile(displayTextPrefix, file, includePaths, currentPath, decl, argumentHintDepth);
      }
    }
  }
  if(ret.isEmpty() && namespaceAllowed) {
    foreach(QualifiedIdentifier prefix, prefixes) {
      prefix.setExplicitlyGlobal(false);
      const IndexedDUContext* contexts;
      uint contextCount;
      QualifiedIdentifier id = prefix + identifier;

      PersistentSymbolTable::self().contexts( id, contextCount, contexts );
      for(uint a = 0; a < contextCount; ++a) {
        QString file = contexts[a].indexedTopContext().url().str();
        ret += itemsForFile(identifier.toString() + " " + displayTextPrefix, file, includePaths, currentPath, IndexedDeclaration(), argumentHintDepth);
      }
    }
  }
  
  return ret;
}

QVariant MissingIncludeCompletionItem::data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const {
  DUChainReadLocker lock(DUChain::lock(), 500);
  if(!lock.locked()) {
    kDebug(9007) << "Failed to lock the du-chain in time";
    return QVariant();
  }
  switch (role) {
    case KTextEditor::CodeCompletionModel::IsExpandable:
      return QVariant(true);
    case KTextEditor::CodeCompletionModel::ExpandingWidget: {
      if(!m_decl.data())
        return QVariant();
      
      Cpp::NavigationWidget* nav = new Cpp::NavigationWidget(DeclarationPointer(m_decl.data()), TopDUContextPointer(m_decl.data()->topContext()));
      model->addNavigationWidget(this, nav);

       QVariant v;
       v.setValue<QWidget*>((QWidget*)nav);
       return v;
    }
    case Qt::DisplayRole:
      switch (index.column()) {
        case KTextEditor::CodeCompletionModel::Prefix:
            return i18n("Add") +  " #include " + m_addedInclude + " " + i18n("for");
        case KTextEditor::CodeCompletionModel::Name: {
          if(!m_decl.data())
            return m_displayTextPrefix;
          else
            return m_displayTextPrefix + m_decl.data()->toString();
        }
      }
      break;
    case KTextEditor::CodeCompletionModel::ItemSelected:
    {
      if(!m_decl.data())
        return QVariant();
      return QVariant( Cpp::NavigationWidget::shortDescription(m_decl.data()) );
    }
  }

  return QVariant();
}

void MissingIncludeCompletionItem::execute(KTextEditor::Document* document, const KTextEditor::Range& word) {

  QString insertLine = "#include " + m_addedInclude;
  int lastLineWithInclude = 0;
  int checkLines = word.start().line() -1;
  for(int a = 0; a < checkLines; ++a) {
    QString lineText = document->line(a);
    if(lineText.trimmed().startsWith("#include")) {
      QString ending = lineText.trimmed();
      if(!ending.isEmpty())
        ending = ending.left( ending.length()-1 ).trimmed(); //Remove the last > or "
      
      if(!ending.endsWith(".moc"))
        lastLineWithInclude = a;
    }
  }
  document->insertLine(lastLineWithInclude+1, insertLine);
}

int MissingIncludeCompletionItem::inheritanceDepth() const {
  return 0;
}
