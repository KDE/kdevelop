/*
 * DUChain Utilities
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
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

#include "duchainutils.h"

#include <kiconloader.h>

#include <interfaces/ilanguage.h>
#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>

#include "../interfaces/ilanguagesupport.h"

#include "declaration.h"
#include "classfunctiondeclaration.h"
#include "ducontext.h"
#include "duchain.h"
#include "use.h"
#include "duchainlock.h"
#include "classmemberdeclaration.h"
#include "functiondefinition.h"
#include "specializationstore.h"
#include "persistentsymboltable.h"
#include "classdeclaration.h"
#include "parsingenvironment.h"

using namespace KDevelop;
using namespace KTextEditor;

CodeCompletionModel::CompletionProperties DUChainUtils::completionProperties(const Declaration* dec)
{
  CodeCompletionModel::CompletionProperties p;

  if(dec->context()->type() == DUContext::Class) {
    if (const ClassMemberDeclaration* member = dynamic_cast<const ClassMemberDeclaration*>(dec)) {
      switch (member->accessPolicy()) {
        case Declaration::Public:
          p |= CodeCompletionModel::Public;
          break;
        case Declaration::Protected:
          p |= CodeCompletionModel::Protected;
          break;
        case Declaration::Private:
          p |= CodeCompletionModel::Private;
          break;
        default:
          break;
      }

      if (member->isStatic())
        p |= CodeCompletionModel::Static;
      if (member->isAuto())
        {}//TODO
      if (member->isFriend())
        p |= CodeCompletionModel::Friend;
      if (member->isRegister())
        {}//TODO
      if (member->isExtern())
        {}//TODO
      if (member->isMutable())
        {}//TODO
    }
  }

  if (const AbstractFunctionDeclaration* function = dynamic_cast<const AbstractFunctionDeclaration*>(dec)) {
    p |= CodeCompletionModel::Function;
    if (function->isVirtual())
      p |= CodeCompletionModel::Virtual;
    if (function->isInline())
      p |= CodeCompletionModel::Inline;
    if (function->isExplicit())
      {}//TODO
  }

  if( dec->isTypeAlias() )
    p |= CodeCompletionModel::TypeAlias;

  if (dec->abstractType()) {
    switch (dec->abstractType()->whichType()) {
      case AbstractType::TypeIntegral:
        p |= CodeCompletionModel::Variable;
        break;
      case AbstractType::TypePointer:
        p |= CodeCompletionModel::Variable;
        break;
      case AbstractType::TypeReference:
        p |= CodeCompletionModel::Variable;
        break;
      case AbstractType::TypeFunction:
        p |= CodeCompletionModel::Function;
        break;
      case AbstractType::TypeStructure:
        p |= CodeCompletionModel::Class;
        break;
      case AbstractType::TypeArray:
        p |= CodeCompletionModel::Variable;
        break;
      case AbstractType::TypeEnumeration:
        p |= CodeCompletionModel::Enum;
        break;
      case AbstractType::TypeEnumerator:
        p |= CodeCompletionModel::Variable;
        break;
      case AbstractType::TypeAbstract:
      case AbstractType::TypeDelayed:
      case AbstractType::TypeUnsure:
      case AbstractType::TypeAlias:
        // TODO
        break;
    }

    if( dec->abstractType()->modifiers() & AbstractType::ConstModifier )
      p |= CodeCompletionModel::Const;

    if( dec->kind() == Declaration::Instance && !dec->isFunctionDeclaration() )
      p |= CodeCompletionModel::Variable;
  }

  if (dec->context()) {
    if( dec->context()->type() == DUContext::Global )
      p |= CodeCompletionModel::GlobalScope;
    else if( dec->context()->type() == DUContext::Namespace )
      p |= CodeCompletionModel::NamespaceScope;
    else if( dec->context()->type() != DUContext::Class && dec->context()->type() != DUContext::Enum )
      p |= CodeCompletionModel::LocalScope;
  }

  return p;
}
/**We have to construct the item from the pixmap, else the icon will be marked as "load on demand",
 * and for some reason will be loaded every time it's used(this function returns a QIcon marked "load on demand"
 * each time this is called). And the loading is very slow. Seems like a bug somewhere, it cannot be ment to be that slow.
 */
#define RETURN_CACHED_ICON(name) {static QIcon icon(QIcon::fromTheme(name).pixmap(QSize(16, 16))); return icon;}

QIcon DUChainUtils::iconForProperties(KTextEditor::CodeCompletionModel::CompletionProperties p)
{

  if( (p & CodeCompletionModel::Variable) )
    if( (p & CodeCompletionModel::Protected) )
      RETURN_CACHED_ICON("CVprotected_var")
    else if( p & CodeCompletionModel::Private )
      RETURN_CACHED_ICON("CVprivate_var")
    else
      RETURN_CACHED_ICON("CVpublic_var")
  else
  if( (p & CodeCompletionModel::Union) && (p & CodeCompletionModel::Protected) )
    RETURN_CACHED_ICON("protected_union")

  else if( p & CodeCompletionModel::Enum )
    if( p & CodeCompletionModel::Protected )
      RETURN_CACHED_ICON("protected_enum")
    else if( p & CodeCompletionModel::Private )
      RETURN_CACHED_ICON("private_enum")
    else
      RETURN_CACHED_ICON("enum")

  else if( p & CodeCompletionModel::Struct )
    if( p & CodeCompletionModel::Private )
      RETURN_CACHED_ICON("private_struct")
    else
      RETURN_CACHED_ICON("struct")

  else if( p & CodeCompletionModel::Slot )
    if( p & CodeCompletionModel::Protected )
      RETURN_CACHED_ICON("CVprotected_slot")
    else if( p & CodeCompletionModel::Private )
      RETURN_CACHED_ICON("CVprivate_slot")
    else if(p & CodeCompletionModel::Public )
      RETURN_CACHED_ICON("CVpublic_slot")
    else RETURN_CACHED_ICON("slot")
  else if( p & CodeCompletionModel::Signal )
    if( p & CodeCompletionModel::Protected )
      RETURN_CACHED_ICON("CVprotected_signal")
    else
      RETURN_CACHED_ICON("signal")

  else if( p & CodeCompletionModel::Class )
    if( (p & CodeCompletionModel::Class) && (p & CodeCompletionModel::Protected) )
      RETURN_CACHED_ICON("protected_class")
    else if( (p & CodeCompletionModel::Class) && (p & CodeCompletionModel::Private) )
      RETURN_CACHED_ICON("private_class")
    else
      RETURN_CACHED_ICON("code-class")

  else if( p & CodeCompletionModel::Union )
    if( p & CodeCompletionModel::Private )
      RETURN_CACHED_ICON("private_union")
    else
      RETURN_CACHED_ICON("union")

  else if( p & CodeCompletionModel::TypeAlias )
    if ((p & CodeCompletionModel::Const) /*||  (p & CodeCompletionModel::Volatile)*/)
      RETURN_CACHED_ICON("CVtypedef")
    else
      RETURN_CACHED_ICON("typedef")

  else if( p & CodeCompletionModel::Function ) {
    if( p & CodeCompletionModel::Protected )
      RETURN_CACHED_ICON("protected_function")
    else if( p & CodeCompletionModel::Private )
      RETURN_CACHED_ICON("private_function")
    else
      RETURN_CACHED_ICON("code-function")
  }

  if( p & CodeCompletionModel::Protected )
    RETURN_CACHED_ICON("protected_field")
  else if( p & CodeCompletionModel::Private )
    RETURN_CACHED_ICON("private_field")
  else
    RETURN_CACHED_ICON("field")

  return QIcon::fromTheme();
}

QIcon DUChainUtils::iconForDeclaration(const Declaration* dec)
{
  return iconForProperties(completionProperties(dec));
}

TopDUContext* DUChainUtils::contentContextFromProxyContext(TopDUContext* top)
{
  if(!top)
    return 0;
  if(top->parsingEnvironmentFile() && top->parsingEnvironmentFile()->isProxyContext()) {
    if(!top->importedParentContexts().isEmpty())
    {
      DUContext* ctx = top->importedParentContexts()[0].context(0);
      if(!ctx)
        return 0;
      TopDUContext* ret = ctx->topContext();
      if(!ret)
        return 0;
      if(ret->url() != top->url())
        kDebug() << "url-mismatch between content and proxy:" << top->url().toUrl() << ret->url().toUrl();
      if(ret->url() == top->url() && !ret->parsingEnvironmentFile()->isProxyContext())
        return ret;
    }
    else {
      kDebug() << "Proxy-context imports no content-context";
    }
  } else
    return top;
  return 0;
}

TopDUContext* DUChainUtils::standardContextForUrl(const KUrl& url, bool preferProxyContext) {
  KDevelop::TopDUContext* chosen = 0;

  QList<KDevelop::ILanguage*> languages = ICore::self()->languageController()->languagesForUrl(url);

  foreach( KDevelop::ILanguage* language, languages)
  {
    if(!chosen)
    {
      if (language->languageSupport())
        chosen = language->languageSupport()->standardContext(url, preferProxyContext);
    }
  }

  if(!chosen)
    chosen = DUChain::self()->chainForDocument(IndexedString(url.pathOrUrl()), preferProxyContext);

  if(!chosen && preferProxyContext)
    return standardContextForUrl(url, false); // Fall back to a normal context
  
  return chosen;
}

Declaration* declarationUnderCursor(const CursorInRevision& c, DUContext* ctx)
{
  foreach( Declaration* decl, ctx->localDeclarations() )
    if( decl->range().contains(c) )
      return decl;

  //Search all collapsed sub-contexts. In C++, those can contain declarations that have ranges out of the context
  foreach( DUContext* subCtx, ctx->childContexts() ) {
    //This is a little hacky, but we need it in case of foreach macros and similar stuff
    if(subCtx->range().isEmpty() || subCtx->range().start.line == c.line || subCtx->range().end.line == c.line) {
      Declaration* decl = declarationUnderCursor(c, subCtx);
      if(decl)
        return decl;
    }
  }
  return 0;
}

Declaration* DUChainUtils::itemUnderCursor(const KUrl& url, const KDevelop::SimpleCursor& _c)
{
  KDevelop::TopDUContext* chosen = standardContextForUrl(url);

  if( chosen )
  {
    CursorInRevision c = chosen->transformToLocalRevision(_c);
    DUContext* ctx = chosen->findContextAt(c);

    while( ctx ) {
      //Try finding a declaration under the cursor
      Declaration* decl = declarationUnderCursor(c, ctx);
      if(decl)
        return decl;

      //Try finding a use under the cursor
      for(int a = 0; a < ctx->usesCount(); ++a)
        if( ctx->uses()[a].m_range.contains(c) )
          return ctx->topContext()->usedDeclarationForIndex(ctx->uses()[a].m_declarationIndex);

      ctx = ctx->parentContext(); //It may happen, for example in the case of function-declarations, that the use is one context higher.
    }
  }
  return 0;
}

KTextEditor::Range DUChainUtils::itemRangeUnderCursor(const KUrl& url, const KDevelop::SimpleCursor& cursor)
{
  KDevelop::TopDUContext* chosen = standardContextForUrl(url);

  if( chosen ) {
    CursorInRevision c = chosen->transformToLocalRevision(cursor);
    DUContext* ctx = chosen->findContextAt(c);
    if (ctx) {
      Declaration* decl = declarationUnderCursor(c, ctx);
      if (decl && decl->range().contains(c) ) {
        return decl->rangeInCurrentRevision().textRange();
      }

      for(int a = 0; a < ctx->usesCount(); ++a) {
        if( ctx->uses()[a].m_range.contains(c) ) {
          return ctx->transformFromLocalRevision(ctx->uses()[a].m_range).textRange();
        }
      }
    }
  }
  return KTextEditor::Range();
}

Declaration* DUChainUtils::declarationForDefinition(Declaration* definition, TopDUContext* topContext)
{
  if(!definition)
    return 0;

  if(!topContext)
    topContext = definition->topContext();

  if(dynamic_cast<FunctionDefinition*>(definition)) {
    Declaration* ret = static_cast<FunctionDefinition*>(definition)->declaration();
    if(ret)
      return ret;
  }

  return definition;
}

Declaration* DUChainUtils::declarationInLine(const KDevelop::SimpleCursor& _cursor, DUContext* ctx) {
  if(!ctx)
    return 0;
  
  CursorInRevision cursor = ctx->transformToLocalRevision(_cursor);
  
  foreach(Declaration* decl, ctx->localDeclarations()) {
    if(decl->range().start.line == cursor.line)
      return decl;
    DUContext* funCtx = getFunctionContext(decl);
    if(funCtx && funCtx->range().contains(cursor))
      return decl;
  }

  foreach(DUContext* child, ctx->childContexts()){
    Declaration* decl = declarationInLine(_cursor, child);
    if(decl)
      return decl;
  }

  return 0;
}

DUChainUtils::DUChainItemFilter::~DUChainItemFilter() {
}

void DUChainUtils::collectItems( DUContext* context, DUChainItemFilter& filter ) {

  QVector<DUContext*> children = context->childContexts();
  QVector<Declaration*> localDeclarations = context->localDeclarations();

  QVector<DUContext*>::const_iterator childIt = children.constBegin();
  QVector<Declaration*>::const_iterator declIt = localDeclarations.constBegin();

  while(childIt != children.constEnd() || declIt != localDeclarations.constEnd()) {

    DUContext* child = 0;
    if(childIt != children.constEnd())
      child = *childIt;

    Declaration* decl = 0;
    if(declIt != localDeclarations.constEnd())
      decl = *declIt;

    if(decl) {
      if(child && child->range().start.line >= decl->range().start.line)
        child = 0;
    }

    if(child) {
      if(decl && decl->range().start >= child->range().start)
        decl = 0;
    }

    if(decl) {
      if( filter.accept(decl) ) {
        //Action is done in the filter
      }

      ++declIt;
      continue;
    }

    if(child) {
      if( filter.accept(child) )
        collectItems(child, filter);
      ++childIt;
      continue;
    }
  }
}

KDevelop::DUContext* DUChainUtils::getArgumentContext(KDevelop::Declaration* decl) {
  DUContext* internal = decl->internalContext();
  if( !internal )
    return 0;
  if( internal->type() == DUContext::Function )
    return internal;
  foreach( const DUContext::Import &ctx, internal->importedParentContexts() ) {
    if( ctx.context(decl->topContext()) )
      if( ctx.context(decl->topContext())->type() == DUContext::Function )
        return ctx.context(decl->topContext());
  }
  return 0;
}

QList<IndexedDeclaration> DUChainUtils::collectAllVersions(Declaration* decl) {
  QList<IndexedDeclaration> ret;
  ret << IndexedDeclaration(decl);
  
  if(decl->inSymbolTable())
  {
    uint count;
    const IndexedDeclaration* allDeclarations;
    PersistentSymbolTable::self().declarations(decl->qualifiedIdentifier(), count, allDeclarations);
    for(uint a = 0; a < count; ++a)
      if(!(allDeclarations[a] == IndexedDeclaration(decl)))
        ret << allDeclarations[a];
  }
  
  return ret;
}

///For a class, returns all classes that inherit it
QList<Declaration*> DUChainUtils::getInheriters(const Declaration* decl, uint& maxAllowedSteps, bool collectVersions)
{
  QList<Declaration*> ret;
  
  if(!dynamic_cast<const ClassDeclaration*>(decl))
    return ret;

  if(maxAllowedSteps == 0)
    return ret;

  if(decl->internalContext() && decl->internalContext()->type() == DUContext::Class)
    FOREACH_ARRAY(const IndexedDUContext& importer, decl->internalContext()->indexedImporters()) {
      
      DUContext* imp = importer.data();
      
      if(!imp)
        continue;
      
      if(imp->type() == DUContext::Class && imp->owner())
        ret << imp->owner();

      --maxAllowedSteps;
      
      if(maxAllowedSteps == 0)
        return ret;
    }
    
    if(maxAllowedSteps == 0)
      return ret;

  if(collectVersions && decl->inSymbolTable()) {
    uint count;
    const IndexedDeclaration* allDeclarations;
    PersistentSymbolTable::self().declarations(decl->qualifiedIdentifier(), count, allDeclarations);
    for(uint a = 0; a < count; ++a) {
      ++maxAllowedSteps;
      
      if(allDeclarations[a].data() && allDeclarations[a].data() != decl) {
        ret += getInheriters(allDeclarations[a].data(), maxAllowedSteps, false);
      }
      
      if(maxAllowedSteps == 0)
        return ret;
    }
  }
  
  return ret;
}

QList<Declaration*> DUChainUtils::getOverriders(const Declaration* currentClass, const Declaration* overriddenDeclaration, uint& maxAllowedSteps) {
  QList<Declaration*> ret;
  
  if(maxAllowedSteps == 0)
    return ret;
  
  if(currentClass != overriddenDeclaration->context()->owner() && currentClass->internalContext())
    ret += currentClass->internalContext()->findLocalDeclarations(overriddenDeclaration->identifier(), CursorInRevision::invalid(), currentClass->topContext(), overriddenDeclaration->abstractType());
  
  foreach(Declaration* inheriter, getInheriters(currentClass, maxAllowedSteps))
    ret += getOverriders(inheriter, overriddenDeclaration, maxAllowedSteps);
  
  return ret;
}

static bool hasUse(DUContext* context, int usedDeclarationIndex) {
  if(usedDeclarationIndex == std::numeric_limits<int>::max())
    return false;
  
  for(int a = 0; a < context->usesCount(); ++a)
    if(context->uses()[a].m_declarationIndex == usedDeclarationIndex)
      return true;
    
  foreach(DUContext* child, context->childContexts())
    if(hasUse(child, usedDeclarationIndex))
      return true;
  return false;
}

bool DUChainUtils::contextHasUse(DUContext* context, Declaration* declaration) {
  return hasUse(context, context->topContext()->indexForUsedDeclaration(declaration, false));
}

static uint countUses(DUContext* context, int usedDeclarationIndex) {
  if(usedDeclarationIndex == std::numeric_limits<int>::max())
    return 0;
  
  uint ret = 0;
  
  for(int a = 0; a < context->usesCount(); ++a)
    if(context->uses()[a].m_declarationIndex == usedDeclarationIndex)
      ++ret;
    
  foreach(DUContext* child, context->childContexts())
    ret += countUses(child, usedDeclarationIndex);
  
  return ret;
}

uint DUChainUtils::contextCountUses(DUContext* context, Declaration* declaration) {
  return countUses(context, context->topContext()->indexForUsedDeclaration(declaration, false));
}

Declaration* DUChainUtils::getOverridden(const Declaration* decl) {
  const ClassFunctionDeclaration* classFunDecl = dynamic_cast<const ClassFunctionDeclaration*>(decl);
  if(!classFunDecl || !classFunDecl->isVirtual())
    return 0;
  
  QList<Declaration*> decls;

  foreach(const DUContext::Import &import, decl->context()->importedParentContexts()) {
    DUContext* ctx = import.context(decl->topContext());
    if(ctx)
      decls += ctx->findDeclarations(QualifiedIdentifier(decl->identifier()), 
                                            CursorInRevision::invalid(), decl->abstractType(), decl->topContext(), DUContext::DontSearchInParent);
  }

  foreach(Declaration* found, decls) {
    const ClassFunctionDeclaration* foundClassFunDecl = dynamic_cast<const ClassFunctionDeclaration*>(found);
    if(foundClassFunDecl && foundClassFunDecl->isVirtual())
      return found;
  }
    
  return 0;
}

DUContext* DUChainUtils::getFunctionContext(Declaration* decl) {
  DUContext* functionContext = decl->internalContext();
  if(functionContext && functionContext->type() != DUContext::Function) {
    foreach(const DUContext::Import& import, functionContext->importedParentContexts()) {
      DUContext* ctx = import.context(decl->topContext());
      if(ctx && ctx->type() == DUContext::Function)
        functionContext = ctx;
    }
  }
  
  if(functionContext && functionContext->type() == DUContext::Function)
    return functionContext;
  return 0;
}
