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

#include "definition.h"
#include "declaration.h"
#include "classfunctiondeclaration.h"
#include "ducontext.h"
#include "duchain.h"
#include "classmemberdeclaration.h"
#include "typesystem.h"

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

KIcon DUChainUtils::iconForProperties(KTextEditor::CodeCompletionModel::CompletionProperties p)
{
  QString iconName;

  if( (p & CodeCompletionModel::Variable) )
    iconName = "CVprotected_var";
  else if( (p & CodeCompletionModel::Variable) && (p & CodeCompletionModel::Protected) )
    iconName = "CVprotected_var";
  else if( (p & CodeCompletionModel::Variable) && (p & CodeCompletionModel::Private) )
    iconName = "CVprivate_var";
  else if( (p & CodeCompletionModel::Union) && (p & CodeCompletionModel::Protected) )
    iconName = "protected_union";
  else if( (p & CodeCompletionModel::Enum) && (p & CodeCompletionModel::Protected) )
    iconName = "protected_enum";
  else if( (p & CodeCompletionModel::Struct) && (p & CodeCompletionModel::Private) )
    iconName = "private_struct";
  else if( (p & CodeCompletionModel::Slot) && (p & CodeCompletionModel::Protected) )
    iconName = "CVprotected_slot";
  else if( (p & CodeCompletionModel::Enum) && (p & CodeCompletionModel::Private) )
    iconName = "private_enum";
  else if( (p & CodeCompletionModel::Signal) && (p & CodeCompletionModel::Protected) )
    iconName = "CVprotected_signal";
  else if( (p & CodeCompletionModel::Slot) && (p & CodeCompletionModel::Private) )
    iconName = "CVprivate_slot";
  else if( (p & CodeCompletionModel::Class) && (p & CodeCompletionModel::Protected) )
    iconName = "protected_class";
  else if( (p & CodeCompletionModel::Class) && (p & CodeCompletionModel::Private) )
    iconName = "private_class";
  else if( (p & CodeCompletionModel::Union) && (p & CodeCompletionModel::Private) )
    iconName = "private_union";
  else if( (p & CodeCompletionModel::TypeAlias) && ((p & CodeCompletionModel::Const) /*||  (p & CodeCompletionModel::Volatile)*/) )
    iconName = "CVtypedef";
  else if( (p & CodeCompletionModel::Function) && (p & CodeCompletionModel::Protected) )
    iconName = "protected_function";
  else if( (p & CodeCompletionModel::Function) && (p & CodeCompletionModel::Private) )
    iconName = "private_function";
  else if( p & CodeCompletionModel::Signal )
    iconName = "signal";
  else if( p & CodeCompletionModel::Variable )
    iconName = "CVpublic_var";
  else if( p & CodeCompletionModel::Enum )
    iconName = "enum";
  else if( p & CodeCompletionModel::Class )
    iconName = "class";
  else if( p & CodeCompletionModel::Slot )
    iconName = "CVpublic_slot";
  else if( p & CodeCompletionModel::Union )
    iconName = "union";
  else if( p & CodeCompletionModel::TypeAlias )
    iconName = "typedef";
  else if( p & CodeCompletionModel::Function )
    iconName = "function";
  else if( p & CodeCompletionModel::Struct )
    iconName = "struct";
  else if( p & CodeCompletionModel::Protected )
    iconName = "protected_field";
  else if( p & CodeCompletionModel::Private )
    iconName = "private_field";
  else
    iconName = "field";

  return KIcon(iconName);
}

KIcon DUChainUtils::iconForDeclaration(Declaration* dec)
{
  return iconForProperties(completionProperties(dec));
}
