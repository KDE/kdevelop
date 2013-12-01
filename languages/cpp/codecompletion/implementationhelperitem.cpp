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

#include "implementationhelperitem.h"
#include <ktexteditor/document.h>
#include <klocalizedstring.h>
#include <language/duchain/duchainutils.h>
#include "context.h"
#include "helpers.h"
#include <language/duchain/types/functiontype.h>
#include <language/duchain/classfunctiondeclaration.h>
#include <qtfunctiondeclaration.h>
#include <cppduchain.h>
#include <sourcemanipulation.h>
#include <language/codegen/coderepresentation.h>
#include <language/codegen/documentchangeset.h>
#include <language/backgroundparser/backgroundparser.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>

namespace Cpp {

ImplementationHelperItem::ImplementationHelperItem(HelperType type, KDevelop::DeclarationPointer decl, KSharedPtr<Cpp::CodeCompletionContext> context, int _inheritanceDepth, int _listOffset) : NormalDeclarationCompletionItem(decl, KSharedPtr<KDevelop::CodeCompletionContext>::staticCast(context), _inheritanceDepth, _listOffset), m_type(type) {
}

#define RETURN_CACHED_ICON(name) {static QIcon icon(KIcon(name).pixmap(QSize(16, 16))); return icon;}

QString ImplementationHelperItem::getOverrideName(const KDevelop::QualifiedIdentifier& forcedParentIdentifier) const {
  QString ret;
  if(m_declaration) {
    ret = m_declaration->identifier().toString();

    KDevelop::ClassFunctionDeclaration* classDecl = dynamic_cast<KDevelop::ClassFunctionDeclaration*>(declaration().data());
    if(classDecl) {
      if(classDecl->isConstructor() || classDecl->isDestructor()) {
        if (forcedParentIdentifier.isEmpty() && completionContext() && completionContext()->duContext()) {
          ret = completionContext()->duContext()->localScopeIdentifier().toString();
        } else {
          ret = forcedParentIdentifier.last().toString();
        }
      }
      if(classDecl->isDestructor())
        ret = "~" + ret;
    }
  }
  return ret;
}

QVariant ImplementationHelperItem::data(const QModelIndex& index, int role, const KDevelop::CodeCompletionModel* model) const {
  QVariant ret = NormalDeclarationCompletionItem::data(index, role, model);
  if(role == Qt::DecorationRole) {
    if(index.column() == KTextEditor::CodeCompletionModel::Icon) {
      switch(m_type) {
        case Override: {
          KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
          KDevelop::ClassFunctionDeclaration* classFunction = dynamic_cast<ClassFunctionDeclaration*>(m_declaration.data());
          if(classFunction && classFunction->isAbstract())
            RETURN_CACHED_ICON("flag-red");

          RETURN_CACHED_ICON("CTparents");
        }
        case CreateDefinition:
          RETURN_CACHED_ICON("CTsuppliers"); ///@todo Better icon?
        case CreateSignalSlot:
          RETURN_CACHED_ICON("dialog-ok-apply"); ///@todo Better icon?
      }
    }
  }
  if(role == Qt::DisplayRole) {
    if(index.column() == KTextEditor::CodeCompletionModel::Prefix) {
      QString prefix;
      if(m_type == Override)
        prefix = i18nc("@action C++ code completion", "Override");
      if(m_type == CreateDefinition)
        prefix = i18nc("@action C++ code completion", "Implement");
      if(m_type == CreateSignalSlot)
        return i18nc("@action C++ code completion", "Create Slot");

      ret = QString(prefix + " " + ret.toString());
    }

    if(index.column() == KTextEditor::CodeCompletionModel::Name) {
      KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
      if(m_type == CreateSignalSlot) {
        ret = completionContext()->followingText();
        Cpp::QtFunctionDeclaration* cDecl = dynamic_cast<Cpp::QtFunctionDeclaration*>(m_declaration.data());

        if(cDecl && ret.toString().isEmpty())
          ret = cDecl->identifier().toString();

        return ret;
      }else if(m_type == Override) {
        ret = getOverrideName();
      }
      if(declaration().data() && m_type != Override) {
        QualifiedIdentifier parentScope = declaration()->context()->scopeIdentifier(true);
        parentScope = Cpp::stripPrefixes(m_completionContext->duContext(), parentScope);
        if(!parentScope.isEmpty())
          ret = QString(parentScope.toString() + "::" + ret.toString());
      }
    }
    if(index.column() == KTextEditor::CodeCompletionModel::Arguments) {
      KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
      KDevelop::ClassFunctionDeclaration* classFunction = dynamic_cast<ClassFunctionDeclaration*>(m_declaration.data());
      if(classFunction && classFunction->isAbstract())
        ret = QString(ret.toString() + " = 0");
    }
  }

  if(role == KTextEditor::CodeCompletionModel::ItemSelected) {
    KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
    if(declaration().data() && m_type == Override) {
      QualifiedIdentifier parentScope = declaration()->context()->scopeIdentifier(true);
      return i18n("From %1", parentScope.toString());
    }
  }

  if(role == KTextEditor::CodeCompletionModel::InheritanceDepth)
    return QVariant(0);
  return ret;
}

QString ImplementationHelperItem::signaturePart(bool includeDefaultParams) {
  KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
  QString ret;
  createArgumentList(*this, ret, 0, includeDefaultParams, true);
  if(m_declaration->abstractType() && m_declaration->abstractType()->modifiers() & AbstractType::ConstModifier)
    ret += " const";
  return ret;
}

QString ImplementationHelperItem::insertionText(KUrl url, KDevelop::SimpleCursor position, QualifiedIdentifier forceParentScope) {
  KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());
  QString newText;
  if(!m_declaration)
    return QString();

  DUContext* duContext = 0;
  if(completionContext())
    duContext = completionContext()->duContext(); ///@todo Take the DUContext from somewhere lese

  KDevelop::ClassFunctionDeclaration* classFunction = dynamic_cast<ClassFunctionDeclaration*>(m_declaration.data());

  ///@todo Move these functionalities into sourcemanipulation.cpp
  if(m_type == Override) {
    if(!useAlternativeText) {

      if(!classFunction || !classFunction->isConstructor())
        newText = "virtual ";
      else if(classFunction && classFunction->isConstructor() && classFunction->isExplicit())
        newText = "explicit ";

      if(m_declaration) {
        FunctionType::Ptr asFunction = m_declaration->type<FunctionType>();
        if(asFunction && asFunction->returnType())
            newText += Cpp::simplifiedTypeString(asFunction->returnType(), duContext) + " ";

        newText += getOverrideName(forceParentScope);

        newText += signaturePart(true);
        newText += ";";
      } else {
        kDebug() << "Declaration disappeared";
        return QString();
      }
    }else{
      newText = alternativeText;
    }
  }else if(m_type == CreateDefinition) {
      TopDUContext* topContext = DUChainUtils::standardContextForUrl(url);
      if(topContext) {
        duContext = topContext->findContextAt(topContext->transformToLocalRevision(position));
      }

      QualifiedIdentifier scope = m_declaration->qualifiedIdentifier();

      if(!forceParentScope.isEmpty() && !scope.isEmpty()) {
        if (classFunction && classFunction->isConstructor()) {
          // HACK: for the new class dialog use the parentscope's identifier for the ctor
          //       see also hack further below
          scope = forceParentScope;
          scope.push(forceParentScope.last());
        } else {
          scope = forceParentScope;
          scope.push(m_declaration->identifier());
        }
      }else{
        //Shorten the scope considering the context
        scope = Cpp::stripPrefixes(duContext, scope);
      }

/*      if(scope.count() <= localScope.count() || !scope.toString().startsWith(localScope.toString()))
        return QString();

      scope = scope.mid( localScope.count() );*/

      FunctionType::Ptr asFunction = m_declaration->type<FunctionType>();

      if(asFunction && asFunction->returnType())
          newText += Cpp::simplifiedTypeString(asFunction->returnType(), duContext) + " ";
      newText += scope.toString();
      newText += signaturePart(false);

      if(classFunction && classFunction->isConstructor()) {
        KDevelop::DUContext* funCtx = classFunction->internalFunctionContext();
        if(funCtx) {
          typedef QMultiMap<IndexedType, QString> TypeNamesMap;  // type -> names of arguments with this type
          TypeNamesMap namesForType;
          typedef QMap<QString, IndexedType> NameTypeMap;
          NameTypeMap typeForName;     // argument name -> type
          foreach (const Declaration* arg, funCtx->localDeclarations()) {
            namesForType.insert(arg->indexedType(), arg->identifier().toString());
            typeForName.insert(arg->identifier().toString(), arg->indexedType());
          }
          bool started = false;
          const QVector< KDevelop::DUContext::Import > imports = classFunction->context()->importedParentContexts();
          foreach(const DUContext::Import& import, imports) {
            KDevelop::DUContext* ctx = import.context(topContext);
            if(ctx && ctx->type() == DUContext::Class && ctx->owner()) {
              const int MaxMismatchCount = 20; // hope there won't be more than 20 arguments
              int bestMismatchCount = MaxMismatchCount;
              QString bestBaseCallText;
              bool isBestDefault = false;
              foreach (Declaration* decl, ctx->localDeclarations()) {
                ClassFunctionDeclaration* func = dynamic_cast<ClassFunctionDeclaration*>(decl);
                if (!func || !func->isConstructor()) {
                  continue;
                }
                QString baseCallText;
                int mismatchCount = 0;   // Matched arguments for base constructor
                if(!started)
                  baseCallText += ": ";
                else
                  baseCallText += ", ";
                if (!forceParentScope.isEmpty()) {
                  //HACK: for the new class dialog use the direct declaration's identifier for the ctor call,
                  //      as otherwise we'd try to call a indirect parent's ctor
                  baseCallText += m_declaration->identifier().toString();
                } else {
                  baseCallText += ctx->owner()->identifier().toString();
                }
                baseCallText += '(';
                QVector<Declaration*> parentArgs = func->internalFunctionContext()->localDeclarations();
                for(int a = 0; a < parentArgs.size(); ++a) {
                  if(a)
                    baseCallText += ", ";
                  Declaration* parentArg = parentArgs[a];
                  NameTypeMap::const_iterator byName = typeForName.constFind(parentArg->identifier().toString());
                  if (byName != typeForName.constEnd() && byName.value() == parentArg->indexedType()) {
                    // Both type and name match - good
                    baseCallText += byName.key();
                  } else {
                    TypeNamesMap::const_iterator byType = namesForType.constFind(parentArg->indexedType());
                    if (byType != namesForType.constEnd()) {
                      // Anything with a suitable type will do
                      baseCallText += byType.value();
                    } else {
                      ++mismatchCount;
                    }
                  }
                }
                if (mismatchCount < bestMismatchCount || (mismatchCount == bestMismatchCount && isBestDefault)) {
                  bestMismatchCount = mismatchCount;
                  bestBaseCallText = baseCallText + ')';
                  isBestDefault = (parentArgs.size() == 0);
                  if (mismatchCount == 0 && !isBestDefault) {
                    break;
                  }
                }
              }
              if (bestMismatchCount < MaxMismatchCount && !isBestDefault) {   // There was an actual match
                started = true;
                newText += bestBaseCallText;
              }
            }
          }
        }
      }

      newText += "\n{\n";

      if(asFunction) {

        ClassFunctionDeclaration* overridden = dynamic_cast<ClassFunctionDeclaration*>(DUChainUtils::getOverridden(m_declaration.data()));

        if(!forceParentScope.isEmpty())
          overridden = dynamic_cast<ClassFunctionDeclaration*>(m_declaration.data());

        if(overridden && !overridden->isAbstract() && !overridden->isDestructor() && !overridden->isConstructor()) {
          QualifiedIdentifier baseScope = overridden->qualifiedIdentifier();
          // baseScope == scope is the case for the "default method" implementations in the CreateClass wizard
          const bool callParent = baseScope != scope;
          QString parentCall;
          if (callParent) {
            bool foundShorter = true;
            do {
              foundShorter = false;
              QualifiedIdentifier candidate = baseScope;
              if(candidate.count() > 2) {
                candidate.pop();
                QList<Declaration*> decls = m_declaration->context()->findDeclarations(candidate);
                if(decls.contains(overridden)) {
                  foundShorter = true;
                  baseScope = candidate;
                }
              }
            }while(foundShorter);

            parentCall += baseScope.toString() + '(';
            DUContext* ctx = m_declaration->internalContext();
            if(ctx->type() == DUContext::Function) {
              bool first = true;
              foreach(Declaration* decl, ctx->localDeclarations()) {
                if(!first)
                  parentCall += ", ";
                first = false;
                parentCall += decl->identifier().toString();
              }
            }

            parentCall += ");";
          }

          const bool needsReturn = asFunction->returnType() && asFunction->returnType()->toString() != "void";
          // true when we return something like Foo* and are currently in class Foo
          bool returnThis = false;
          // true when we return something like const Foo& and are currently in class Foo
          bool returnThisDeref = false;
          QualifiedIdentifier parentScope = scope;
          parentScope.pop(); // function identifier
          if (needsReturn) {
            StructureType::Ptr sType;
            bool wasRef = false;
            bool wasPtr = false;
            if (ReferenceType::Ptr refType = asFunction->returnType().cast<ReferenceType>()) {
              wasRef = true;
              sType = refType->baseType().cast<StructureType>();
            } else if (PointerType::Ptr ptrType = asFunction->returnType().cast<PointerType>()) {
              wasPtr = true;
              sType = ptrType->baseType().cast<StructureType>();
            }
            if (sType && sType->qualifiedIdentifier() == parentScope) {
              Q_ASSERT(wasPtr || wasRef);
              if (wasPtr) {
                returnThis = true;
              } else if (wasRef) {
                returnThisDeref = true;
              }
            }
          }

          if (!needsReturn && callParent) {
            newText += parentCall;
          } else if (needsReturn && (returnThisDeref || returnThis)) {
            if (callParent) {
              newText += parentCall;
            }
            newText += "return ";
            if (returnThisDeref) {
              newText += "*this;";
            } else {
              newText += "this;";
            }
          } else if (needsReturn && callParent) {
            newText += "return " + parentCall;
          } else if (needsReturn) {
            newText += "///TODO: return ...;";
          }

        }
      }

      newText += "\n}\n";
  }

  return newText;
}

void ImplementationHelperItem::execute(KTextEditor::Document* document, const KTextEditor::Range& word) {

  if(m_type == CreateSignalSlot) {
    //Step 1: Decide where to put the declaration
    KDevelop::DUChainReadLocker lock(KDevelop::DUChain::lock());

    IndexedString doc;
    {
      QSet<DUContext*> containers = completionContext()->memberAccessContainers();

      if(containers.isEmpty())
        return;
      else
        doc = (*containers.begin())->url();
    }

    lock.unlock();
    //Make sure the top-context is up-to-date, waiting for an update if required
    KDevelop::ReferencedTopDUContext updated( DUChain::self()->waitForUpdate(doc, TopDUContext::AllDeclarationsAndContexts) );

    if(!updated) {
      kDebug() << "not creating slot because failed to update" << doc.str();
      return;
    }
    lock.lock();

    QSet<DUContext*> containers = completionContext()->memberAccessContainers();

    if(containers.isEmpty())
      return;

    DUContext* classContext = *containers.begin();

    Cpp::SourceCodeInsertion insertion(updated.data());
    insertion.setContext(classContext);
    insertion.setAccess(Declaration::Private);

    QString slotName = completionContext()->followingText();
    if(slotName.isEmpty())
      slotName = completionContext()->m_connectedSignalIdentifier.toString();

    QString slotSignature = QString::fromUtf8(completionContext()->m_connectedSignalNormalizedSignature);
    insertion.insertSlot(slotName, slotSignature);

    QString name = completionContext()->followingText();
    if(name.isEmpty() && m_declaration)
      name = m_declaration->identifier().toString();

    lock.unlock();

    if(!insertion.changes().applyAllChanges()) {
      kDebug() << "failed";
      return;
    }

    ICore::self()->languageController()->backgroundParser()->addDocument(doc);
    executeSignalSlotCompletionItem( document, word, false, slotName, slotSignature );
  }else{
    //this code assumes (safely for now) that the "word" range is on one line
    KTextEditor::Range rangeToReplace(word.start().line(), 0, word.end().line(), word.end().column());
    QString rangeToReplaceText = document->text(rangeToReplace);
    QString replacementText = insertionText(document->url(), SimpleCursor(rangeToReplace.end()));
    //Don't replace anything before end of comment, open or closing bracket, or semicolon
    QRegExp replaceAfter = QRegExp("inline|[{}/;]");
    int noReplace = replaceAfter.lastIndexIn(rangeToReplaceText) + replaceAfter.matchedLength() - 1;
    if (noReplace >= 0)
    {
      rangeToReplace = KTextEditor::Range(word.start().line(), noReplace + 1, word.end().line(), word.end().column());
      replacementText.prepend(" ");
    }
    DocumentChangeSet changes;
    changes.addChange(DocumentChange(IndexedString(document->url()), rangeToReplace, document->text(rangeToReplace), replacementText));
    changes.applyAllChanges();
   }
}

bool ImplementationHelperItem::dataChangedWithInput() const {
  return m_type == CreateSignalSlot;
}

}
