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

class TopDUContextPrivate : public DUContextPrivate
{
public:
  TopDUContextPrivate( TopDUContext* ctxt)
    : DUContextPrivate(ctxt), m_inDuChain(false), m_searchingImport(false), m_flags(TopDUContext::NoFlags), m_ctxt(ctxt), m_currentUsedDeclarationIndex(0)
  {
  }

  bool m_hasUses  : 1;
  bool m_deleting : 1;
  bool m_inDuChain : 1;
  mutable bool m_searchingImport : 1; //Used to prevent endless recursion, protected by importSearchMutex
  TopDUContext::Flags m_flags;
  TopDUContext* m_ctxt;
  ParsingEnvironmentFilePointer m_file;
  QList<ProblemPointer> m_problems;

  //Adds the context to this and all contexts that import this, and manages m_recursiveImports
  void addImportedContextRecursively(const TopDUContext* context) {
    addImportedContextRecursion(context, context, 1);
    
    QHash<const TopDUContext*, QPair<int, const TopDUContext*> > b = context->d_func()->m_recursiveImports;
    for(RecursiveImports::const_iterator it = b.begin(); it != b.end(); ++it)
      addImportedContextRecursion(context, it.key(), (*it).first+1); //Add contexts that were imported earlier into the given one

/*    QSet<TopDUContext*> closure;
    const_cast<TopDUContext*>(context)->d_func()->childClosure(closure);
    foreach(TopDUContext* ctx, closure) {
      Q_ASSERT(ctx == context || ctx->d_func()->m_recursiveImports.contains(context));
    }*/
  }

  //Removes the context from this and all contexts that import this, and manages m_recursiveImports
  void removeImportedContextRecursively(const TopDUContext* context) {
    QVector<QPair<TopDUContext*, const TopDUContext*> > rebuild;
    removeImportedContextRecursion(context, context, rebuild);

    QHash<const TopDUContext*, QPair<int, const TopDUContext*> > b = context->d_func()->m_recursiveImports;
    for(RecursiveImports::const_iterator it = b.begin(); it != b.end(); ++it) {
//       Q_ASSERT(m_recursiveImports.contains(it.key()));
      if(m_recursiveImports.contains(it.key()) && m_recursiveImports[it.key()].second == context)
        removeImportedContextRecursion(context, it.key(), rebuild); //Remove all contexts that are imported through the context
    }
    
    rebuildImportStructureRecursion(rebuild);
  
/*    QSet<TopDUContext*> closure;
    const_cast<TopDUContext*>(context)->d_func()->childClosure(closure);
    foreach(TopDUContext* ctx, closure) {
      Q_ASSERT(ctx == context || ctx->d_func()->m_recursiveImports.contains(context));
    }*/
  }
  
  //Has an entry for every single recursively imported file, that contains the shortest path, and the next context on that path to the imported context.
  //This does not need to be stored to disk, because it is defined implicitly.
  //What makes this most complicated is the fact that loops are allowed in the import structure.
  typedef QHash<const TopDUContext*, QPair<int, const TopDUContext*> > RecursiveImports;
  RecursiveImports m_recursiveImports;

  ///Is used to count up the used declarations while building uses
  uint m_currentUsedDeclarationIndex;

  ///Maps a declarationIndex to a DeclarationId, which is used when the entry in m_usedDeclaration is zero.
  QVector<DeclarationId> m_usedDeclarationIds;
  ///Maps a declarationIndex to an actual used Declaration
  QVector<DeclarationPointer> m_usedDeclarations;

  /**Maps a declarationIndex to local declarations. Generally, negative indices are considered
   * to be indices within m_usedLocalDeclarations, and positive indices within m_usedDeclarationIds
   * Any declarations that are within the same top-context are considered local.
   * */
  QVector<Declaration*> m_usedLocalDeclarations;
  private:

    void childClosure(QSet<TopDUContext*>& children) {
      if(children.contains(m_ctxt))
        return;
      children.insert(m_ctxt);
      for(QVector<DUContext*>::iterator it = m_importedChildContexts.begin(); it != m_importedChildContexts.end(); ++it) {
        TopDUContext* top = dynamic_cast<TopDUContext*>(*it);
        if(top)
          top->d_func()->childClosure(children);
      }
    }
    
  void addImportedContextRecursion(const TopDUContext* traceNext, const TopDUContext* imported, int depth) {

    if(imported == m_ctxt)
      return;
    
    RecursiveImports::iterator it = m_recursiveImports.find(imported);
    if(it == m_recursiveImports.end()) {
      //Insert new path to "imported"
      m_recursiveImports[imported] = qMakePair(depth, traceNext);
    }else{
      //It would be better if we would use the following code, but it creates too much cost in updateImportedContextRecursion when imports are removed again.
      
      //Check whether the new way to "imported" is shorter than the stored one
      if((*it).first > depth) {
        //Add a shorter path
        (*it).first = depth;
        Q_ASSERT(traceNext);
        (*it).second = traceNext;
        Q_ASSERT(traceNext == imported || (traceNext->d_func()->m_recursiveImports.contains(imported) && traceNext->d_func()->m_recursiveImports[imported].first < (*it).first));
      }else{
        //The imported context is already imported through a same/better path, so we can just stop processing. This saves us from endless recursion.
        return;
      }
    }
    
    for(QVector<DUContext*>::iterator it = m_importedChildContexts.begin(); it != m_importedChildContexts.end(); ++it) {
      TopDUContext* top = dynamic_cast<TopDUContext*>(*it);
      if(top)
        top->d_func()->addImportedContextRecursion(m_ctxt, imported, depth+1);
    }
  }

  void removeImportedContextRecursion(const TopDUContext* traceNext, const TopDUContext* imported, QVector<QPair<TopDUContext*, const TopDUContext*> >& rebuild) {

    if(imported == m_ctxt)
      return;
    
    RecursiveImports::iterator it = m_recursiveImports.find(imported);
    if(it == m_recursiveImports.end()) {
      //We don't import. Just return, this saves us from endless recursion.
      return;
    }else{
      //Check whether we have imported "imported" through "traceNext". If not, return. Else find a new trace.
      if((*it).second == traceNext) {
        //We need to remove the import through traceNext. Check whether there is another imported context that imports it.

        m_recursiveImports.erase(it); //In order to prevent problems, we completely remove everything, and re-add it.
                                      //Just updating these complex structures is very hard.
        
        rebuild.append(qMakePair(m_ctxt, imported));
        //We MUST do this before finding another trace, because else we would create loops
        for(QVector<DUContext*>::iterator childIt = m_importedChildContexts.begin(); childIt != m_importedChildContexts.end(); ++childIt) {
          TopDUContext* top = dynamic_cast<TopDUContext*>(*childIt);
          if(top)
            top->d_func()->removeImportedContextRecursion(m_ctxt, imported, rebuild); //Don't use 'it' from here on, it may be invalid
        }

        /*        TopDUContext* newTraceNext = 0;
        int newDepth = 0;
        
        //Now find out whether we still import the context through another trace
        for(QVector<DUContextPointer>::iterator parentIt = m_importedParentContexts.begin(); parentIt != m_importedParentContexts.end(); ++parentIt) {
          TopDUContext* top = dynamic_cast<TopDUContext*>(parentIt->data());
          if(top && top != traceNext) {
            RecursiveImports::const_iterator it2 = top->d_func()->m_recursiveImports.find(imported);
            if(it2 != top->d_func()->m_recursiveImports.end()) {
              //Found through another import than traceNext, so compute a new depth
              //Take the shortest path
              if(!newDepth || (newDepth > (*it2).first + 1)) {

                newTraceNext = top;
                newDepth = (*it2).first + 1;
              }
            }
          }
        }

        if(newTraceNext) {
          //Re-add the trace
          
          m_recursiveImports[imported] = qMakePair<int, const TopDUContext*>(newDepth, newTraceNext);
          
          for(QVector<DUContext*>::iterator childIt = m_importedChildContexts.begin(); childIt != m_importedChildContexts.end(); ++childIt) {
            TopDUContext* top = dynamic_cast<TopDUContext*>(*childIt);
            if(top)
              top->d_func()->addImportedContextRecursion(m_ctxt, imported, newDepth+1);
          }
        }*/
      }
    }
  }

  //Updates the trace to 'imported'
  void rebuildStructure(const TopDUContext* imported) {
    if(m_ctxt == imported)
      return;
    
    for(QVector<DUContextPointer>::iterator parentIt = m_importedParentContexts.begin(); parentIt != m_importedParentContexts.end(); ++parentIt) {
      TopDUContext* top = dynamic_cast<TopDUContext*>(parentIt->data());
      if(top) {
        if(top == imported) {
          addImportedContextRecursion(top, imported, 1);
        }else{
          RecursiveImports::const_iterator it2 = top->d_func()->m_recursiveImports.find(imported);
          if(it2 != top->d_func()->m_recursiveImports.end()) {

            addImportedContextRecursion(top, imported, (*it2).first + 1);
          }
        }
      }
    }
  }

  void rebuildImportStructureRecursion(const QVector<QPair<TopDUContext*, const TopDUContext*> >& rebuild) {

    for(int a = rebuild.size()-1; a >= 0; --a) {
      //Find the best imported parent
      rebuild[a].first->d_func()->rebuildStructure(rebuild[a].second);
    }
  }
};


ImportTrace TopDUContext::importTrace(const TopDUContext* target) const
  {
    ImportTrace ret;

    TopDUContextPrivate::RecursiveImports::const_iterator it = d_func()->m_recursiveImports.find(target);
    
    if(it == d_func()->m_recursiveImports.end())
      return ret;

    const TopDUContext* nextContext = (*it).second;
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
  setInSymbolTable(true);
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

bool TopDUContext::findDeclarationsInternal(const QList<QualifiedIdentifier>& identifiers, const SimpleCursor& position, const AbstractType::Ptr& dataType, QList<Declaration*>& ret, const ImportTrace& /*trace*/, SearchFlags flags) const
{
  ENSURE_CAN_READ
  kDebug() << identifiers;
//  foreach(id, identifiers)
//      kDebug() << "searching" << id;

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

  if( dynamic_cast<const TopDUContext*>(origin) ) {
    return d_func()->m_recursiveImports.contains(static_cast<const TopDUContext*>(origin));
  } else {
    return DUContext::imports(origin, position);
  }
 }

void TopDUContext::addImportedParentContext(DUContext* context, const SimpleCursor& position, bool anonymous) {
  DUContext::addImportedParentContext(context, position, anonymous);
  d_func()->addImportedContextRecursively(static_cast<TopDUContext*>(context));
}

void TopDUContext::removeImportedParentContext(DUContext* context) {
  DUContext::removeImportedParentContext(context);
  d_func()->removeImportedContextRecursively(static_cast<TopDUContext*>(context));
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
    if(declarationIndex >= 0 && declarationIndex < d_func()->m_usedLocalDeclarations.size())
      return d_func()->m_usedLocalDeclarations[declarationIndex];
    else
      return 0;
  }else{
    if(declarationIndex >= 0 && declarationIndex < d_func()->m_usedDeclarationIds.size())
    {
      if(d_func()->m_usedDeclarations[declarationIndex])
        return d_func()->m_usedDeclarations[declarationIndex].data();

      //If no real declaration is available, we need to search the declaration. This is currently not used, we need to think about whether we need it.
      return d_func()->m_usedDeclarationIds[declarationIndex].getDeclaration(const_cast<TopDUContext*>(this));
    }else{
      return 0;
    }
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
