/* This  is part of KDevelop
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#include "topducontext.h"

#include "symboltable.h"
#include "declaration.h"

using namespace KTextEditor;

TopDUContext::TopDUContext(KTextEditor::Range* range)
  : DUContext(range)
  , m_hasUses(false)
{
}

TopDUContext::~TopDUContext( )
{
}

void TopDUContext::setHasUses(bool hasUses)
{
  m_hasUses = hasUses;
}

bool TopDUContext::hasUses() const
{
  return m_hasUses;
}

Declaration* TopDUContext::findDeclarationInternal(const QualifiedIdentifier& identifier, const KTextEditor::Cursor& position, const AbstractType::Ptr& dataType, QList<UsingNS*>& usingNS, bool inImportedContext) const
{
  Q_UNUSED(inImportedContext);

  QList<Declaration*> found = checkDeclarations(SymbolTable::self()->findDeclarations(identifier), position, dataType);

  if (found.count() == 1)
    return found.first();

  if (found.count() > 1) {
    /*kWarning() << k_funcinfo << "Multiple definitions for " << identifier.toString() << endl;
    foreach (Declaration* dec, found)
      kDebug() << "Found at " << dec->textRange() << " in document " << dec->url() << endl;*/

    return found.first();
  }

  if (!identifier.explicitlyGlobal()) {
    acceptUsingNamespaces(position, usingNS);

    if (!usingNS.isEmpty())
      return findDeclarationInNamespaces(identifier, position, dataType, usingNS);
  }

  return 0;
}

Declaration* TopDUContext::findDeclarationInNamespaces(const QualifiedIdentifier& identifier, const KTextEditor::Cursor& position, const AbstractType::Ptr& dataType, QList<UsingNS*>& usingNS, int depth) const
{
  foreach (UsingNS* ns, usingNS) {
    QualifiedIdentifier id = identifier.merge(ns->nsIdentifier);

    // FIXME nested using definitions

    QList<Declaration*> found = checkDeclarations(SymbolTable::self()->findDeclarations(id), position, dataType);

    if (found.count() == 1)
      return found.first();

    if (found.count() > 1) {
      /*kWarning() << k_funcinfo << "Multiple definitions for " << identifier.toString() << endl;
      foreach (Declaration* dec, found)
        kDebug() << "Found at " << dec->textRange() << " in document " << dec->url() << endl;*/

      return found.first();
    }
  }

  if (depth < 10) {
    foreach (UsingNS* ns, usingNS) {
      QList<UsingNS*> newUsingNS = findNestedNamespaces(position, ns);
      if (!newUsingNS.isEmpty())
        return findDeclarationInNamespaces(identifier.strip(ns->nsIdentifier), position, dataType, newUsingNS, depth + 1);
    }
  }

  return 0;
}

QList<DUContext::UsingNS*> TopDUContext::findNestedNamespaces(const KTextEditor::Cursor & position, UsingNS* ns) const
{
  QList<UsingNS*> nestedUsingNS;

  // Retrieve nested namespaces
  QList<DUContext*> contexts;
  checkContexts(DUContext::Namespace, SymbolTable::self()->findContexts(ns->nsIdentifier), position, contexts);

  foreach (DUContext* nsContext, contexts) {
    TopDUContext* origin = nsContext->topContext();
    bool doesImport = false;
    bool importEvaluated = false;
    bool sameDocument = nsContext->topContext() == this;

    foreach (UsingNS* nested, nsContext->usingNamespaces()) {
      if (sameDocument && position >= nested->textCursor()) {
        acceptUsingNamespace(nested, nestedUsingNS);

      } else {
        if (!importEvaluated) {
          doesImport = imports(origin);
          importEvaluated = true;
        }

        if (doesImport)
          acceptUsingNamespace(nested, nestedUsingNS);
        else
          break;
      }
    }
  }

  return nestedUsingNS;
}

bool TopDUContext::imports(TopDUContext * origin, int depth) const
{
  if (depth == 100) {
    kWarning() << k_funcinfo << "Imported context list too deep!" << endl;
    return false;
  }

  foreach (DUContext* context, importedParentContexts()) {
    Q_ASSERT(dynamic_cast<TopDUContext*>(context));
    TopDUContext* top = static_cast<TopDUContext*>(context);
    if (top == origin)
      return true;
    if (top->imports(origin, depth + 1))
      return true;
  }

  return false;
}

QList<Declaration*> TopDUContext::checkDeclarations(const QList<Declaration*>& declarations, const KTextEditor::Cursor& position, const AbstractType::Ptr& dataType) const
{
  QList<Declaration*> found;

  foreach (Declaration* dec, declarations) {
    if (dataType && dec->abstractType() != dataType)
      continue;

    TopDUContext* top = dec->context()->topContext();
    if (top != this) {
      // Make sure that this declaration is accessible
      // TODO when import location available, use that too
      if (!imports(top))
        continue;

    } else {
      if (dec->textRange().start() > position)
        continue;
    }

    found.append(dec);
  }

  return found;
}

void TopDUContext::findContextsInternal(ContextType contextType, const QualifiedIdentifier & identifier, const KTextEditor::Cursor & position, QList< UsingNS * >& usingNS, QList<DUContext*>& ret, bool inImportedContext) const
{
  Q_UNUSED(inImportedContext);

  checkContexts(contextType, SymbolTable::self()->findContexts(identifier), position, ret);

  // Don't search using definitions if we already found a match
  if (!ret.isEmpty())
    return;

  if (!identifier.explicitlyGlobal()) {
    acceptUsingNamespaces(position, usingNS);

    foreach (UsingNS* ns, usingNS) {
      QualifiedIdentifier id = identifier.merge(ns->nsIdentifier);

      // FIXME nested using definitions

      checkContexts(contextType, SymbolTable::self()->findContexts(id), position, ret);
    }
  }
}

void TopDUContext::checkContexts(ContextType contextType, const QList<DUContext*>& contexts, const KTextEditor::Cursor& position, QList<DUContext*>& ret) const
{
  foreach (DUContext* context, contexts) {
    //if (context->type() != contextType)
      //continue;

    TopDUContext* top = context->topContext();
    if (top != this) {
      // Make sure that this declaration is accessible
      // TODO when import location available, use that too
      if (!imports(top))
        continue;

    } else {
      if (context->textRange().start() > position)
        continue;
    }

    ret.append(context);
  }
}

// kate: indent-width 2;
