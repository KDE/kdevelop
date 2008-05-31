/*
 * DUChain Utilities
 *
 * Copyright 2007 Hamish Rodda <rodda@kde.org>
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

#include "declaration.h"
#include "classfunctiondeclaration.h"
#include "ducontext.h"
#include "duchain.h"
#include "use.h"
#include "declaration.h"
#include "duchainlock.h"
#include "classmemberdeclaration.h"
#include "typesystem.h"
#include "kiconloader.h"
#include <ilanguage.h>
#include <ilanguagesupport.h>
#include <icore.h>
#include <ilanguagecontroller.h>

using namespace KDevelop;
using namespace KTextEditor;

CodeCompletionModel::CompletionProperties DUChainUtils::completionProperties(Declaration* dec)
{
  CodeCompletionModel::CompletionProperties p;

  if (ClassMemberDeclaration* member = dynamic_cast<ClassMemberDeclaration*>(dec)) {
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
    }

    if (member->isStatic())
      p |= CodeCompletionModel::Static;
    if (member->isAuto())
      ;//TODO
    if (member->isFriend())
      p |= CodeCompletionModel::Friend;
    if (member->isRegister())
      ;//TODO
    if (member->isExtern())
      ;//TODO
    if (member->isMutable())
      ;//TODO
  }

  if (AbstractFunctionDeclaration* function = dynamic_cast<AbstractFunctionDeclaration*>(dec)) {
    if (function->isVirtual())
      p |= CodeCompletionModel::Virtual;
    if (function->isInline())
      p |= CodeCompletionModel::Inline;
    if (function->isExplicit())
      ;//TODO
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
      case AbstractType::TypeAbstract:
      case AbstractType::TypeDelayed:
        // TODO
        break;
    }

    if( dec->kind() == Declaration::Instance )
      p |= CodeCompletionModel::Variable;
  }

  if( dec->context()->type() == DUContext::Global )
    p |= CodeCompletionModel::GlobalScope;
  else if( dec->context()->type() == DUContext::Namespace )
    p |= CodeCompletionModel::NamespaceScope;
  else if( dec->context()->type() != DUContext::Class )
    p |= CodeCompletionModel::LocalScope;

  return p;
}

QIcon DUChainUtils::iconForProperties(KTextEditor::CodeCompletionModel::CompletionProperties p)
{
  QString iconName;

  if( (p & CodeCompletionModel::Variable) )
    if( (p & CodeCompletionModel::Protected) )
      iconName = "CVprotected_var";
    else if( p & CodeCompletionModel::Private )
      iconName = "CVprivate_var";
    else
      iconName = "CVpublic_var";

  else if( (p & CodeCompletionModel::Union) && (p & CodeCompletionModel::Protected) )
    iconName = "protected_union";

  else if( p & CodeCompletionModel::Enum )
    if( p & CodeCompletionModel::Protected )
      iconName = "protected_enum";
    else if( p & CodeCompletionModel::Private )
      iconName = "private_enum";
    else
      iconName = "enum";

  else if( p & CodeCompletionModel::Struct )
    if( p & CodeCompletionModel::Private )
      iconName = "private_struct";
    else
      iconName = "struct";

  else if( p & CodeCompletionModel::Slot )
    if( p & CodeCompletionModel::Protected )
      iconName = "CVprotected_slot";
    else if( p & CodeCompletionModel::Private )
      iconName = "CVprivate_slot";
    else
      iconName = "CVpublic_slot";

  else if( p & CodeCompletionModel::Signal )
    if( p & CodeCompletionModel::Protected )
      iconName = "CVprotected_signal";
    else
      iconName = "signal";

  else if( p & CodeCompletionModel::Class )
    if( (p & CodeCompletionModel::Class) && (p & CodeCompletionModel::Protected) )
      iconName = "protected_class";
    else if( (p & CodeCompletionModel::Class) && (p & CodeCompletionModel::Private) )
      iconName = "private_class";
    else
      iconName = "class";

  else if( p & CodeCompletionModel::Union )
    if( p & CodeCompletionModel::Private )
      iconName = "private_union";
    else
      iconName = "union";

  else if( p & CodeCompletionModel::TypeAlias )
    if ((p & CodeCompletionModel::Const) /*||  (p & CodeCompletionModel::Volatile)*/)
      iconName = "CVtypedef";
    else
      iconName = "typedef";

  else if( p & CodeCompletionModel::Function )
    if( p & CodeCompletionModel::Protected )
      iconName = "protected_function";
    else if( p & CodeCompletionModel::Private )
      iconName = "private_function";
    else
      iconName = "function";

  else if( p & CodeCompletionModel::Protected )
    iconName = "protected_field";
  else if( p & CodeCompletionModel::Private )
    iconName = "private_field";
  else
    iconName = "field";

  return KIcon(iconName);
}

QIcon DUChainUtils::iconForDeclaration(Declaration* dec)
{
  return iconForProperties(completionProperties(dec));
}

TopDUContext* DUChainUtils::standardContextForUrl(const KUrl& url) {
  KDevelop::TopDUContext* chosen = 0;

  QList<KDevelop::ILanguage*> languages = ICore::self()->languageController()->languagesForUrl(url);

  foreach( KDevelop::ILanguage* language, languages)
    if(!chosen)
      chosen = language->languageSupport()->standardContext(url);

  if(!chosen)
    return DUChain::self()->chainForDocument(HashedString(url.prettyUrl()));

  return chosen;
}

Declaration* declarationUnderCursor(const SimpleCursor& c, DUContext* ctx)
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

Declaration* DUChainUtils::itemUnderCursor(const KUrl& url, const SimpleCursor& c)
{
  KDevelop::TopDUContext* chosen = standardContextForUrl(url);
  
  if( chosen )
  {
    DUContext* ctx = chosen->findContextAt(c);
    
    while( ctx ) {
      //Try finding a declaration under the cursor
      Declaration* decl = declarationUnderCursor(c, ctx);
      if(decl)
        return decl;
      
      //Try finding a use under the cursor
      for(int a = 0; a < ctx->uses().count(); ++a)
        if( ctx->uses()[a].m_range.contains(c) )
          return ctx->topContext()->usedDeclarationForIndex(ctx->uses()[a].m_declarationIndex);

      ctx = ctx->parentContext(); //It may happen, for example in the case of function-declarations, that the use is one context higher.
    }
  }
  return 0;
}

Declaration* DUChainUtils::declarationForDefinition(Declaration* definition, TopDUContext* topContext)
{
  if(!definition)
    return 0;
  
  if(!topContext)
    topContext = definition->topContext();
  
  if(definition->isDefinition()) {
    Declaration* ret = definition->declaration();
    if(ret)
      return ret;
  }
  
  return definition;
}

Declaration* DUChainUtils::declarationInLine(const KDevelop::SimpleCursor& cursor, DUContext* ctx) {
  foreach(Declaration* decl, ctx->localDeclarations())
    if(decl->range().start.line == cursor.line)
      return decl;
  
  foreach(DUContext* child, ctx->childContexts()){
    Declaration* decl = declarationInLine(cursor, child);
    if(decl)
      return decl;
  }

  return 0;
}

