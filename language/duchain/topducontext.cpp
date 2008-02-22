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
#include "declarationid.h"
#include "namespacealiasdeclaration.h"
#include "abstractfunctiondeclaration.h"
#include <hashedstring.h>
#include <iproblem.h>
#include <limits>
#include "uses.h"

#include "ducontext_p.h"

using namespace KTextEditor;

namespace KDevelop
{

QMutex importSearchMutex;

class TopDUContextPrivate : public DUContextPrivate
{
public:
  TopDUContextPrivate( TopDUContext* ctxt)
    : DUContextPrivate(ctxt), m_inDuChain(false), m_searchingImport(false), m_flags(TopDUContext::NoFlags), m_ctxt(ctxt), m_currentUsedDeclarationIndex(0)
  {
  }
  /// Clears the import-cache for @param context in this and all context that import this
  /// This is potentially expensive, but needs to be done. We need to find better ways of managing the whole imports structure,
  /// maybe using SetRepository.
  void clearImportsCache(TopDUContext* context) {
    if( m_searchingImport )
      return;

    m_searchingImport = true;

    if( m_importsCache.contains(context) ) {
      m_importsCache.remove(context);
      foreach(DUContext* ctx, m_importedChildContexts) {
        if( TopDUContext* topCtx = dynamic_cast<TopDUContext*>(ctx) )
          topCtx->d_func()->clearImportsCache(context);
      }
    }
    
    m_searchingImport = false;
  }

  ///importSearchMutex must be locked before using this
  bool imports(const TopDUContext* target, int depth = 0) const
  {
    if(depth == 0)
      importSearchMutex.lock();
    
    m_searchingImport = true;

    const bool alwaysCache = true;
    
    if(alwaysCache || depth == 0) {
      QHash<const TopDUContext*, const TopDUContext*>::const_iterator it = m_importsCache.find(target);
      if(it != m_importsCache.end()) {
      m_searchingImport = false;
      if(depth == 0)
        importSearchMutex.unlock();
        
        return *it;
      }
    }
    
    if (depth == 100) {
      kWarning() << "Imported context list too deep! Infinite recursion?" ;
      m_searchingImport = false;
      if(depth == 0)
        importSearchMutex.unlock();
      return false;
    }

    const QList<DUContextPointer>& importedContexts(m_ctxt->importedParentContexts());
    QList<DUContextPointer>::const_iterator end = importedContexts.end();
    for( QList<DUContextPointer>::const_iterator it = importedContexts.begin(); it != end; ++it) {
      if(!(*it)) {
        kWarning() << "Imported context was invalidated";
        continue;
      }
      Q_ASSERT(dynamic_cast<TopDUContext*>((*it).data()));
      TopDUContext* top = static_cast<TopDUContext*>((*it).data());
      if (top == target) {
        if(alwaysCache || depth == 0) {
          m_importsCache[target] = top;
        }
        m_searchingImport = false;
        if(depth == 0)
          importSearchMutex.unlock();
        return true;
      }

      if(top->d_func()->m_searchingImport) //Prevent endless recursion
        continue;

      if (top->d_func()->imports(target, depth + 1)) {
        if(alwaysCache || depth == 0) {
          m_importsCache[target] = top;
        }
        m_searchingImport = false;
        if(depth == 0)
          importSearchMutex.unlock();
        return true;
      }
    }

    m_importsCache[target] = 0;
    m_searchingImport = false;
    if(depth == 0)
      importSearchMutex.unlock();
    return false;
  }

  bool m_hasUses  : 1;
  bool m_deleting : 1;
  bool m_inDuChain : 1;
  mutable bool m_searchingImport : 1; //Used to prevent endless recursion, protected by importSearchMutex
  TopDUContext::Flags m_flags;
  TopDUContext* m_ctxt;
  ParsingEnvironmentFilePointer m_file;
  QList<ProblemPointer> m_problems;
  ///Maps from the target context to the next one in the import trace.
  ///This can be used to reconstruct the import trace.
  ///@todo make this a simple "QSet<TopDUContext> m_importsCache;" by using the caches of all used top-contexts.
  mutable QHash<const TopDUContext*, const TopDUContext*> m_importsCache;

  uint m_currentUsedDeclarationIndex;

  ///Maps a declarationIndex to a DeclarationId.
  QVector<DeclarationId> m_usedDeclarationIds;
  ///Maps a declarationIndex to an actual used Declaration
  QVector<DeclarationPointer> m_usedDeclarations;

  /**Maps a declarationIndex to local declarations. Generally, negative indices are considered
   * to be indices within m_usedLocalDeclarations, and positive indices within m_usedDeclarationIds
   * Any declarations that are within the same top-context are considered local.
   * */
  QVector<Declaration*> m_usedLocalDeclarations;
  
};

ImportTrace TopDUContext::importTrace(const TopDUContext* target) const
  {
    ImportTrace ret;
    Q_D(const TopDUContext);
    if(!d->imports(target))
      return ret;

    const TopDUContext* nextContext = d->m_importsCache[target];
    if(nextContext) {
      ret << ImportTraceItem(this, DUContext::importPosition(nextContext));

      if(target != nextContext)
        ret += nextContext->importTrace(target);
    }else{
      kWarning() << "inconsistent import-structure";
    }
    return ret;
  }


TopDUContext::TopDUContext(const HashedString& url, const SimpleRange& range, ParsingEnvironmentFile* file)
  : DUContext(*new TopDUContextPrivate(this), url, range)
{
  Q_D(TopDUContext);
  d->m_hasUses = false;
  d->m_deleting = false;
  d->m_file = ParsingEnvironmentFilePointer(file);
}

IdentifiedFile TopDUContext::identity() const {
  Q_D(const TopDUContext);
  if( d->m_file )
    return d->m_file->identity();
  else
    return IdentifiedFile(url());
}

KSharedPtr<ParsingEnvironmentFile> TopDUContext::parsingEnvironmentFile() const {
  return d_func()->m_file;
}

TopDUContext::~TopDUContext( )
{
  d_func()->m_deleting = true;
  clearDeclarationIndices();
}

void TopDUContext::setHasUses(bool hasUses)
{
  d_func()->m_hasUses = hasUses;
}

bool TopDUContext::hasUses() const
{
  return d_func()->m_hasUses;
}

void TopDUContext::setParsingEnvironmentFile(ParsingEnvironmentFile* file) {
  d_func()->m_file = KSharedPtr<ParsingEnvironmentFile>(file);
}

bool TopDUContext::findDeclarationsInternal(const QList<QualifiedIdentifier>& identifiers, const SimpleCursor& position, const AbstractType::Ptr& dataType, QList<Declaration*>& ret, const ImportTrace& trace, SearchFlags flags) const
{
  ENSURE_CAN_READ

  QList<QualifiedIdentifier> targetIdentifiers;
  applyAliases(identifiers, targetIdentifiers, position, false);

  foreach( const QualifiedIdentifier& identifier, targetIdentifiers ) {
    QList<Declaration*> declarations = SymbolTable::self()->findDeclarations(identifier);
    ret += checkDeclarations(declarations, position, dataType, flags);
  }
  return true;
}

QList<Declaration*> TopDUContext::checkDeclarations(const QList<Declaration*>& declarations, const SimpleCursor& position, const AbstractType::Ptr& dataType, SearchFlags flags) const
{
  ENSURE_CAN_READ

  QList<Declaration*> found;

  foreach (Declaration* dec, declarations) {
    TopDUContext* top = dec->topContext();
    
    if((flags & OnlyFunctions) && !dynamic_cast<AbstractFunctionDeclaration*>(dec))
      continue;
    
    if (top != this) {
      if (dataType && dec->abstractType() != dataType)
        // The declaration doesn't match the type filter we are applying
        continue;

      // Make sure that this declaration is accessible
      if (!(flags & DUContext::NoImportsCheck) && !importsPrivate(top, position))
        continue;

    } else {
      if (dataType && dec->abstractType() != dataType)
        // The declaration doesn't match the type filter we are applying
        continue;

      if (dec->range().start >= position)
        if(!dec->context() || dec->context()->type() != Class)
            continue; // The declaration is behind the position we're searching from, therefore not accessible
    }

    // Success, this declaration is accessible
    found.append(dec);
  }

  // Returns the list of accessible declarations
  return found;
}


void TopDUContext::applyAliases( const QList<QualifiedIdentifier>& identifiers, QList<QualifiedIdentifier>& target, const SimpleCursor& position, bool canBeNamespace, int startPos, int endPos ) const
{
  QList<QualifiedIdentifier> currentIdentifiers = identifiers;
  int pos = startPos;
  bool ready = false;
  while(!ready && (endPos == -1 || pos < endPos))
  {
    ready = true;
    QList<QualifiedIdentifier> newCurrentIdentifiers;
    foreach( const QualifiedIdentifier& identifier, currentIdentifiers )
    {
      //This code is mainly a clone of DUContext::applyAliases
      bool addUnmodified = true;
      
      if( identifier.count() > pos ) {
        ready = false;

        /*if( !identifier.explicitlyGlobal() || pos != 0 )*/ { ///@todo check iso c++ if using-directives should be respected on top-level when explicitly global
          ///@todo this is bad for a very big repository(the chains should be walked for the top-context instead)
          //Find all namespace-imports at given scope
          QList<Declaration*> imports = SymbolTable::self()->findDeclarations( identifier.mid(0, pos) + globalImportIdentifier );
          imports = checkDeclarations(imports, position, AbstractType::Ptr(), NoSearchFlags);

          if( !imports.isEmpty() )
          {
            //We have namespace-imports.
            foreach( Declaration* importDecl, imports )
            {
              //Search for the identifier with the import-identifier prepended
              Q_ASSERT(dynamic_cast<NamespaceAliasDeclaration*>(importDecl));
              NamespaceAliasDeclaration* alias = static_cast<NamespaceAliasDeclaration*>(importDecl);
              QualifiedIdentifier identifierInImport = alias->importIdentifier() + identifier.mid(pos);
              QList<QualifiedIdentifier> temp;
              temp << identifierInImport;
              
              if( alias->importIdentifier().count() )
                applyAliases(temp, newCurrentIdentifiers, alias->range().start, canBeNamespace, alias->importIdentifier().count(), pos+1);
              else
                kDebug(9505) << "ERROR: Namespace imported by \"" << alias->identifier().toString() << "\" in scope " << identifier.mid(0,pos) << " is \"" << alias->importIdentifier() << "\"";
            }
          }
        }
      }

        if( identifier.count() > (canBeNamespace ? pos  : pos+1) ) { //If it cannot be a namespace, the last part of the scope will be ignored
          ready = false; //Find aliases
          QList<Declaration*> aliases = SymbolTable::self()->findDeclarations( identifier.mid(0, pos+1) );
          aliases = checkDeclarations(aliases, position, AbstractType::Ptr(), NoSearchFlags);
          if(!aliases.isEmpty()) {

            //The first part of the identifier has been found as a namespace-alias.
            //In c++, we only need the first alias. However, just to be correct, follow them all for now.
            foreach( Declaration* aliasDecl, aliases )
            {
              if(!dynamic_cast<NamespaceAliasDeclaration*>(aliasDecl))
                continue;
              
              if( aliasDecl->range().end > position )
                continue;

              addUnmodified = false; //The un-modified identifier can be ignored, because it will be replaced with the resolved alias
              
              NamespaceAliasDeclaration* alias = static_cast<NamespaceAliasDeclaration*>(aliasDecl);

              //Create an identifier where namespace-alias part is replaced with the alias target
              QList<QualifiedIdentifier> temp;
              temp << alias->importIdentifier() + identifier.mid(pos+1);

              if( alias->importIdentifier().count() )
                applyAliases(temp, newCurrentIdentifiers, alias->range().start, canBeNamespace, alias->importIdentifier().count(), pos+1);
              else
                kDebug(9505) << "ERROR: Namespace imported by \"" << alias->identifier().toString() << "\" in scope " << identifier.mid(0,pos) << "\" is \"" << alias->importIdentifier() << "\"";
            }
          }
        }
        
      
    
      if( addUnmodified )
          newCurrentIdentifiers << identifier;
    }
    
    currentIdentifiers = newCurrentIdentifiers;
    ++pos;
  }

  target += currentIdentifiers;
}

void TopDUContext::findContextsInternal(ContextType contextType, const QList<QualifiedIdentifier>& baseIdentifiers, const SimpleCursor& position, QList<DUContext*>& ret, SearchFlags flags) const {

  Q_UNUSED(flags);
  QList<QualifiedIdentifier> targetIdentifiers;
  applyAliases(baseIdentifiers, targetIdentifiers, position, contextType == Namespace);

  foreach( const QualifiedIdentifier& identifier, targetIdentifiers ) {
    QList<DUContext*> allContexts = SymbolTable::self()->findContexts(identifier);
    checkContexts(contextType, allContexts, position, ret, flags & DUContext::NoImportsCheck);
  }
}

void TopDUContext::checkContexts(ContextType contextType, const QList<DUContext*>& contexts, const SimpleCursor& position, QList<DUContext*>& ret, bool ignoreImports) const
{
  ENSURE_CAN_READ

  foreach (DUContext* context, contexts) {
    TopDUContext* top = context->topContext();

    if (top != this) {
      if (context->type() != contextType)
        continue;

      // Make sure that this declaration is accessible
      if (!ignoreImports && !importsPrivate(top, position))
        continue;

    } else {
      if (context->type() != contextType)
        continue;

      if (context->range().start > position)
        if(!context->parentContext() || context->parentContext()->type() != Class)
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
  return d_func()->m_deleting;
}

QList<ProblemPointer> TopDUContext::problems() const
{
  ENSURE_CAN_READ
  return d_func()->m_problems;
}

void TopDUContext::addProblem(const ProblemPointer& problem)
{
  ENSURE_CAN_WRITE
  d_func()->m_problems << problem;
}

void TopDUContext::clearProblems()
{
  ENSURE_CAN_WRITE
  d_func()->m_problems.clear();
}


bool TopDUContext::imports(const DUContext * origin, const SimpleCursor& position) const
{
  ENSURE_CAN_READ

  return importsPrivate(origin, position);
 }

bool TopDUContext::importsPrivate(const DUContext * origin, const SimpleCursor& position) const
{
  Q_UNUSED(position);
  // TODO use position

  if( dynamic_cast<const TopDUContext*>(origin) ) {
    return d_func()->imports(static_cast<const TopDUContext*>(origin));
  } else {
    kWarning() << "non top-context importet into top-context";
    return DUContext::imports(origin, position);
  }
 }

void TopDUContext::addImportedParentContext(DUContext* context, const SimpleCursor& position, bool anonymous) {
  d_func()->clearImportsCache(static_cast<TopDUContext*>(context));
  DUContext::addImportedParentContext(context, position, anonymous);
}

void TopDUContext::removeImportedParentContext(DUContext* context) {
  d_func()->clearImportsCache(static_cast<TopDUContext*>(context));
  DUContext::removeImportedParentContext(context);
}
 
/// Returns true if this object is registered in the du-chain. If it is not, all sub-objects(context, declarations, etc.)
bool TopDUContext::inDuChain() const {
  return d_func()->m_inDuChain;
}

/// This flag is only used by DUChain, never change it from outside.
void TopDUContext::setInDuChain(bool b) {
  d_func()->m_inDuChain = b;
}

TopDUContext::Flags TopDUContext::flags() const {
  return d_func()->m_flags;
}

void TopDUContext::setFlags(Flags f) {
  d_func()->m_flags = f;
}

void TopDUContext::clearDeclarationIndices() {
  ENSURE_CAN_WRITE
  for(int a = 0; a < d_func()->m_usedDeclarationIds.size(); ++a)
      DUChain::uses()->removeUse(d_func()->m_usedDeclarationIds[a], this);
  
  d_func()->m_usedDeclarations.clear();
  d_func()->m_usedDeclarationIds.clear();
  d_func()->m_usedLocalDeclarations.clear();
}

Declaration* TopDUContext::usedDeclarationForIndex(int declarationIndex) const {
  ENSURE_CAN_READ
  if(declarationIndex < 0) {
    declarationIndex = -(declarationIndex + 1);//Add one, because we have subtracted one in another place
    Q_ASSERT(declarationIndex >= 0 && declarationIndex < d_func()->m_usedLocalDeclarations.size());
    return d_func()->m_usedLocalDeclarations[declarationIndex];
  }else{
    Q_ASSERT(declarationIndex >= 0 && declarationIndex < d_func()->m_usedDeclarationIds.size());
    if(d_func()->m_usedDeclarations[declarationIndex])
      return d_func()->m_usedDeclarations[declarationIndex].data();

    //If no real declaration is available, we need to search the declaration. This is currently not used, we need to think about whether we need it.
    return d_func()->m_usedDeclarationIds[declarationIndex].getDeclaration(const_cast<TopDUContext*>(this));
  }
}

int TopDUContext::indexForUsedDeclaration(Declaration* declaration, bool create) {
  if(create) {
    ENSURE_CAN_WRITE
  }else{
    ENSURE_CAN_READ
  }
  if(declaration->topContext() == this) {
    //It is a local declaration, so we need a negative index.
    int index = d_func()->m_usedLocalDeclarations.indexOf(declaration);
    if(index != -1)
      return -index - 1; //Subtract one so it's always negative
    if(!create)
      return std::numeric_limits<int>::max();
    d_func()->m_usedLocalDeclarations.append(declaration);
    return -(d_func()->m_usedLocalDeclarations.count()-1) - 1; //Subtract one so it's always negative
  }else{
    DeclarationId id = declaration->id();
    int index = d_func()->m_usedDeclarationIds.indexOf(id);

    if(index != -1)
      return index;
    if(!create)
      return std::numeric_limits<int>::max();

    d_func()->m_usedDeclarationIds.append(id);
    d_func()->m_usedDeclarations.append(DeclarationPointer(declaration));

    DUChain::uses()->addUse(id, this);
    
    return d_func()->m_usedDeclarationIds.count()-1;
  }
}

QList<KTextEditor::SmartRange*> allSmartUses(TopDUContext* context, Declaration* declaration) {
  QList<KTextEditor::SmartRange*> ret;
  int declarationIndex = context->indexForUsedDeclaration(declaration, false);
  if(declarationIndex == std::numeric_limits<int>::max())
    return ret;
  return allSmartUses(context, declarationIndex);
}

QList<SimpleRange> allUses(TopDUContext* context, Declaration* declaration) {
  QList<SimpleRange> ret;
  int declarationIndex = context->indexForUsedDeclaration(declaration, false);
  if(declarationIndex == std::numeric_limits<int>::max())
    return ret;
  return allUses(context, declarationIndex);
}

}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
