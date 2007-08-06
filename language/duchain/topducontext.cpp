/* This  is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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
#include "duchain.h"
#include "duchainlock.h"
#include "parsingenvironment.h"
#include "duchainpointer.h"

using namespace KTextEditor;

namespace KDevelop
{

class TopDUContextPrivate
{
public:
  TopDUContextPrivate( TopDUContext* ctxt)
    : m_inDuChain(false), m_ctxt(ctxt)
  {
  }
  bool imports(TopDUContext* origin, int depth) const
  {
    if (depth == 100) {
      kWarning() << k_funcinfo << "Imported context list too deep! Infinite recursion?" ;
      return false;
    }

    foreach (DUContextPointer context, m_ctxt->importedParentContexts()) {
      Q_ASSERT(dynamic_cast<TopDUContext*>(context.data()));
      TopDUContext* top = static_cast<TopDUContext*>(context.data());
      if (top == origin)
        return true;

      if (top->d->imports(origin, depth + 1))
        return true;
    }

    return false;
  }
  bool m_hasUses  : 1;
  bool m_deleting : 1;
  bool m_inDuChain : 1;
  TopDUContext* m_ctxt;
  ParsingEnvironmentFilePointer m_file;
};

TopDUContext::TopDUContext(KTextEditor::Range* range, ParsingEnvironmentFile* file)
  : DUContext(range)
  , d(new TopDUContextPrivate(this))
{
  d->m_hasUses = false;
  d->m_deleting = false;
  d->m_file = ParsingEnvironmentFilePointer(file);
}

IdentifiedFile TopDUContext::identity() const {
  if( d->m_file )
    return d->m_file->identity();
  else
    return IdentifiedFile(url());
}

KSharedPtr<ParsingEnvironmentFile> TopDUContext::parsingEnvironmentFile() const {
  return d->m_file;
}

TopDUContext::~TopDUContext( )
{
  d->m_deleting = true;
  delete d;
}

void TopDUContext::setHasUses(bool hasUses)
{
  d->m_hasUses = hasUses;
}

bool TopDUContext::hasUses() const
{
  return d->m_hasUses;
}

void TopDUContext::setParsingEnvironmentFile(ParsingEnvironmentFile* file) const {
  d->m_file = KSharedPtr<ParsingEnvironmentFile>(file);
}

void TopDUContext::findDeclarationsInternal(const QualifiedIdentifier& identifier, const KTextEditor::Cursor& position, const AbstractType::Ptr& dataType, QList<NamespaceAlias*>& usingNS, QList<Declaration*>& ret, bool inImportedContext) const
{
  Q_UNUSED(inImportedContext);

  ENSURE_CAN_READ

  // TODO: Insert namespace alias matching at this point ??

  // Retrieve all direct declarations (not accounting for using statements) for the identifier we're looking for
  QList<Declaration*> directDeclarations = SymbolTable::self()->findDeclarations(identifier);

  // Filter out those declarations that can't be reached from this point in the file
  // Eg. where the declaration is from a file that isn't #included
  ret += checkDeclarations(directDeclarations, position, dataType);

  if (!ret.isEmpty())
    // Success, finish up - don't need to check using definitions (I think this is right - as per standard - rodda)
    return;

  // If the identifier doesn't start with ::<Blah> FIXME wrong, and is identifier.merge correct then?? (in findDeclarationsInNamespaces)
  if (!identifier.explicitlyGlobal()) {
    // Integrate using namespace directives from this scope up to the current position into usingNS
    acceptUsingNamespaces(position, usingNS);

    // If there are using namespace directives
    if (!usingNS.isEmpty()) {

      // Search for declarations which match when taking into account using namespace directives
      findDeclarationsInNamespaces(identifier, position, dataType, usingNS, ret);

      if (!ret.isEmpty())
        // Success, finish up (I think this is right - as per standard - rodda)
        return;

      // Search for declarations which match when taking into account using namespace directives in a nested fashion
      for (int depth = 0; depth < 10; ++depth) {
        // TODO: Insert namespace alias matching at this point ?? -
        //  ? iterate directives, apply aliases, then iterate directives, try out using directives?
        foreach (NamespaceAlias* ns, usingNS) {
          // Find nested using directives
          QList<NamespaceAlias*> newNamespaceAlias = findNestedNamespaces(position, ns);

          if (!newNamespaceAlias.isEmpty())
            // Search for declarations which match when taking into account nested using namespace directives
            findDeclarationsInNamespaces(identifier.strip(ns->nsIdentifier), position, dataType, newNamespaceAlias, ret);
        }

        if (!ret.isEmpty())
          // Success, finish up (I think this is right - as per standard - rodda)
          return;
      }
    }
  }
}

void TopDUContext::findDeclarationsInNamespaces(const QualifiedIdentifier& identifier, const KTextEditor::Cursor& position, const AbstractType::Ptr& dataType, QList<NamespaceAlias*>& usingNS, QList<Declaration*>& ret) const
{
  // TODO: Insert namespace alias matching at this point ?? -
  //  ? iterate directives, apply aliases, then iterate directives, try out using directives?

  foreach (NamespaceAlias* ns, usingNS) {
    // Merge using namespace statement with identifier we're searching for
    QualifiedIdentifier id = identifier.merge(ns->nsIdentifier);

    // Note: Same as at top of findDeclarationsInternal
    // Retrieve all direct declarations (not accounting for using statements) for the identifier we're looking for
    QList<Declaration*> usingDeclarations = SymbolTable::self()->findDeclarations(id);

    // Filter out those declarations that can't be reached from this point in the file
    // Eg. where the declaration is from a file that isn't #included
    ret = checkDeclarations(usingDeclarations, position, dataType);
  }
}

QList<DUContext::NamespaceAlias*> TopDUContext::findNestedNamespaces(const KTextEditor::Cursor & position, NamespaceAlias* ns) const
{
  QList<NamespaceAlias*> nestedNamespaceAlias;

  QList<DUContext*> contexts;

  // Find all contexts with the name specified by the given using namespace statement
  QList<DUContext*> allContexts = SymbolTable::self()->findContexts(ns->nsIdentifier);

  // Filter out contexts which are not reachable from the position in this file
  checkContexts(DUContext::Namespace, allContexts, position, contexts);

  foreach (DUContext* nsContext, contexts) {
    TopDUContext* origin = nsContext->topContext();

    bool doesImport = false;
    bool importEvaluated = false;
    bool sameDocument = nsContext->topContext() == this;

    // Iterate the using namespace declarations in this context, accepting those which are imported at the current position
    foreach (NamespaceAlias* nested, nsContext->namespaceAliases()) {
      if (sameDocument && position >= nested->textCursor()) {
        acceptUsingNamespace(nested, nestedNamespaceAlias);

      } else {
        // Save time here, only check once if this context is imported at the current position
        if (!importEvaluated) {
          doesImport = imports(origin, nested->textCursor());
          importEvaluated = true;
        }

        if (doesImport)
          acceptUsingNamespace(nested, nestedNamespaceAlias);
        else
          break;
      }
    }
  }

  // Now we have a list of all accessible nested namespaces (1 level only though)
  return nestedNamespaceAlias;
}

bool TopDUContext::imports(TopDUContext * origin, const KTextEditor::Cursor& position) const
{
  ENSURE_CAN_READ

  Q_UNUSED(position);
  // TODO use position

  return d->imports(origin, 0);
}

QList<Declaration*> TopDUContext::checkDeclarations(const QList<Declaration*>& declarations, const KTextEditor::Cursor& position, const AbstractType::Ptr& dataType) const
{
  ENSURE_CAN_READ

  QList<Declaration*> found;

  foreach (Declaration* dec, declarations) {
    TopDUContext* top = dec->topContext();
    if (top != this) {
      if (dataType && dec->abstractType() != dataType)
        // The declaration doesn't match the type filter we are applying
        continue;

      // Make sure that this declaration is accessible
      if (!imports(top, position))
        continue;

    } else {
      if (dataType && dec->abstractType() != dataType)
        // The declaration doesn't match the type filter we are applying
        continue;

      if (dec->textRange().start() > position)
        // The declaration is after the position we're searching on, therefore not accessible
        continue;
    }

    // Success, this declaration is accessible
    found.append(dec);
  }

  // Returns the list of accessible declarations
  return found;
}

void TopDUContext::findContextsInternal(ContextType contextType, const QualifiedIdentifier & identifier, const KTextEditor::Cursor & position, QList< NamespaceAlias * >& usingNS, QList<DUContext*>& ret, bool inImportedContext) const
{
  Q_UNUSED(inImportedContext);

  // TODO: Insert namespace alias matching at this point ??

  // Retrieve all contexts with the matching identifier
  QList<DUContext*> allContexts = SymbolTable::self()->findContexts(identifier);
  // Check that these contexts are accessible from this position - deletes those that aren't
  checkContexts(contextType, allContexts, position, ret);

  if (!ret.isEmpty())
    // Success, finish up (I think this is right - as per standard - rodda)
    return;

  // If the identifier doesn't start with ::<Blah> FIXME wrong, and is identifier.merge correct then?? (in findContextsInNamespaces)
  if (!identifier.explicitlyGlobal()) {
    // Integrate using namespace directives from this scope up to the current position into usingNS
    acceptUsingNamespaces(position, usingNS);

    // If there are using namespace directives
    if (!usingNS.isEmpty()) {

      // Search for contexts which match when taking into account using namespace directives
      findContextsInNamespaces(contextType, identifier, position, usingNS, ret);

      if (!ret.isEmpty())
        // Success, finish up (I think this is right - as per standard - rodda)
        return;

      // Search for contexts which match when taking into account using namespace directives in a nested fashion
      for (int depth = 0; depth < 10; ++depth) {
        // TODO: Insert namespace alias matching at this point ?? -
        //  ? iterate directives, apply aliases, then iterate directives, try out using directives?

        foreach (NamespaceAlias* ns, usingNS) {
          // Find nested using directives
          QList<NamespaceAlias*> newNamespaceAlias = findNestedNamespaces(position, ns);
          if (!newNamespaceAlias.isEmpty())
            // Search for contexts which match when taking into account nested using namespace directives
            findContextsInNamespaces(contextType, identifier.strip(ns->nsIdentifier), position, newNamespaceAlias, ret);
        }
        if (!ret.isEmpty())
          // Success, finish up (I think this is right - as per standard - rodda)
          return;
      }
    }
  }
}

void TopDUContext::findContextsInNamespaces(ContextType contextType, const QualifiedIdentifier & identifier, const KTextEditor::Cursor & position, QList< NamespaceAlias * >& usingNS, QList<DUContext*>& ret) const
{
  // TODO: Insert namespace alias matching at this point ?? -
  //  ? iterate directives, apply aliases, then iterate directives, try out using directives?

  foreach (NamespaceAlias* ns, usingNS) {
    QualifiedIdentifier id = identifier.merge(ns->nsIdentifier);

    checkContexts(contextType, SymbolTable::self()->findContexts(id), position, ret);
  }
}

void TopDUContext::checkContexts(ContextType contextType, const QList<DUContext*>& contexts, const KTextEditor::Cursor& position, QList<DUContext*>& ret) const
{
  ENSURE_CAN_READ

  foreach (DUContext* context, contexts) {
    TopDUContext* top = context->topContext();

    if (top != this) {
      if (context->type() != contextType)
        continue;

      // Make sure that this declaration is accessible
      if (!imports(top, position))
        continue;

    } else {
      if (context->type() != contextType)
        continue;

      if (context->textRange().start() > position)
        continue;
    }

    ret.append(context);
  }
}

TopDUContext * TopDUContext::topContext() const
{
  return const_cast<TopDUContext*>(this);
}

bool TopDUContext::deleting() const
{
  return d->m_deleting;
}

/// Returns true if this object is registered in the du-chain. If it is not, all sub-objects(context, declarations, etc.)
bool TopDUContext::inDuChain() const {
  return d->m_inDuChain;
}

/// This flag is only used by DUChain, never change it from outside.
void TopDUContext::setInDuChain(bool b) {
  d->m_inDuChain = b;
}

}

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
