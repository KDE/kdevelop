/* 
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

/**
 * This files contains common helpers for using the du-chain in the context of c++
 * */

#ifndef CPPDUCHAIN_H
#define CPPDUCHAIN_H

#include <QList>
#include <QPair>
#include <QSet>
#include "cppduchainexport.h"
#include <language/duchain/types/abstracttype.h>
#include <language/duchain/identifier.h>

namespace KDevelop {
  class Declaration;
  class ClassMemberDeclaration;
  class DUContext;
  class TopDUContext;
  class CursorInRevision;
}

namespace Cpp {
  class EnvironmentFile;
  
KDEVCPPDUCHAIN_EXPORT const KDevelop::IndexedIdentifier& castIdentifier();

KDEVCPPDUCHAIN_EXPORT const KDevelop::IndexedIdentifier& unnamedNamespaceIdentifier();
  
/**
 * Searches context, and if the identifier is not found there, in its base-classes, as it should be in c++(Only if it's a class context).
 * @param topContext should be the top-context from where the search starts. This is needed to resolve template-parameters.
 * Du-chain must be locked before.
 * */
KDEVCPPDUCHAIN_EXPORT QList<KDevelop::Declaration*> findLocalDeclarations( KDevelop::DUContext* context, const KDevelop::Identifier& identifier, const KDevelop::TopDUContext* topContext, uint depth = 0 );

/**
 * Searches for declarations on the same level, either locally within the context, or namespace. \param identifier that will be declared in a given \param context
 * Tries to follow the C++ rules, that decide where a type may have been forward-declared.
 * */
QList<KDevelop::Declaration*> findDeclarationsSameLevel(KDevelop::DUContext* context, const KDevelop::Identifier& identifier, const KDevelop::CursorInRevision& position);

/**
 * Takes and returns a list of declarations together with their inheritance-depth.
 *
 * In c++, declarations with a lower inheritance-depth overload those with a higher
 * inheritance-depth. Hence such entries are hidden here.
 *
 * This also removes forward-declarations when real declarations with the same
 * identifier are in the list.
 *
 * Furthermore this method removes const methods if @p preferConst is false and removes
 * non-const methods if @p preferConst is true. If there methods are not overloaded
 * by constness though, we only hide non-const methods if @p preferConst is true, i.e.
 * const methods will also be shown if @p preferConst is false.
 * */
KDEVCPPDUCHAIN_EXPORT QList< QPair<KDevelop::Declaration*, int> > hideOverloadedDeclarations(const QList< QPair<KDevelop::Declaration*, int> >& declarations, bool preferConst);

  /**Tries determining the local class that the given code-context belongs to.
   *
   * This works within contexts of type DUContext::Other, as well as in argument contexts of type DUContext::Function(also within function definitions).
   */
KDEVCPPDUCHAIN_EXPORT KDevelop::Declaration* localClassFromCodeContext(const KDevelop::DUContext* context);

  /**Tries determining the local function that the given code-context belongs to.
   *
   * This works within contexts of type DUContext::Other, as well as in argument contexts of type DUContext::Function(also within function definitions).
   */
KDEVCPPDUCHAIN_EXPORT KDevelop::Declaration* localFunctionFromCodeContext(const KDevelop::DUContext* context);

  /**
   * Returns the logical namespace the given context belongs to, visibility-wise.
   * @param identifier An ending part of identifier of the given context(May be the full identifier)
   * @param context A context
   * @param source The context from where to start searching
   * @return The part of "identifier" that represents a namespace
   */
KDEVCPPDUCHAIN_EXPORT KDevelop::QualifiedIdentifier namespaceScopeComponentFromContext(KDevelop::QualifiedIdentifier identifier, const KDevelop::DUContext* context, const KDevelop::TopDUContext* source);

/**
 * Returns whether the given declaration can be accessed from the given context. Checks for private/protected and such stuff.
 * @param fromContext The scope-context from where to check. May be zero.
 * @param declaration The declaration to check access to. Must not be zero.
 * @param source Top-context fromw here to start the computations
 * @param declarationContext The context within which the declaration is visible. This can be a different context than declaration->context() in case of inheritance
 * */
KDEVCPPDUCHAIN_EXPORT bool isAccessible(KDevelop::DUContext* fromContext, KDevelop::ClassMemberDeclaration* declaration, KDevelop::TopDUContext* source, KDevelop::DUContext* declarationContext);

/**
 * Returns the logical parent-context of the given context. This may be a different than the physical parent-context
 * in case of external class-definitions.
 */
KDEVCPPDUCHAIN_EXPORT KDevelop::DUContext* logicalParentContext(KDevelop::DUContext* context, KDevelop::TopDUContext* source);

/**
 * Preprocesses the given string, taking the environment from the given environment-file.
 * DUChain does not need to be locked.
 * @param line All macros that are defined before this line are used
 * */
KDEVCPPDUCHAIN_EXPORT QString preprocess( const QString& text, EnvironmentFile* file, int line, QSet<KDevelop::IndexedString> disableMacros = QSet<KDevelop::IndexedString>() );

///Extracts a normalized signature and identifier from a specifier like "mySignal(int)"
KDEVCPPDUCHAIN_EXPORT QPair<KDevelop::Identifier, QByteArray> qtFunctionSignature(QByteArray fullFunction);

///Exchanges all occurences of @param replace in @param id with @param replaceWith
KDEVCPPDUCHAIN_EXPORT KDevelop::Identifier exchangeQualifiedIdentifier(KDevelop::Identifier id, KDevelop::QualifiedIdentifier replace, KDevelop::QualifiedIdentifier replaceWith);

///Exchanges all occurences of @param replace in @param id with @param replaceWith
KDEVCPPDUCHAIN_EXPORT KDevelop::IndexedTypeIdentifier exchangeQualifiedIdentifier(KDevelop::IndexedTypeIdentifier id, KDevelop::QualifiedIdentifier replace, KDevelop::QualifiedIdentifier replaceWith);

///Tries to un-typedef the given type using the uses directly before the given declaration.
KDEVCPPDUCHAIN_EXPORT KDevelop::IndexedTypeIdentifier unTypedefType(KDevelop::Declaration* decl, KDevelop::IndexedTypeIdentifier type);

///Computes an identifier that represents the given type as well as possible. If the type can not be represented as IndexedTypeIdentifier cleanly,
///the qualified identifier will be set to an expression.
KDEVCPPDUCHAIN_EXPORT KDevelop::IndexedTypeIdentifier identifierForType(KDevelop::AbstractType::Ptr type, KDevelop::TopDUContext* top);

///Returns the type that should be used for shortened printing of the same.
KDEVCPPDUCHAIN_EXPORT KDevelop::AbstractType::Ptr typeForShortenedString(KDevelop::Declaration* decl);
///Returns a shortened string version of the type attached to the given declaration.
///@param desiredLength the desired length. No guarantee that the resulting string will be this short. With the default-value, no shortening will happen in most cases.
///@param ctx visibility context to consider. All prefixes of types are shortened to the minimum length while staying visible from here
///@param stripPrefix this prefix will be stripped from qualified identifiers. This is useful to remove parts of the current context.
KDEVCPPDUCHAIN_EXPORT QString shortenedTypeString(KDevelop::Declaration* decl, KDevelop::DUContext* ctx, int desiredLength = 10000, KDevelop::QualifiedIdentifier stripPrefix = KDevelop::QualifiedIdentifier());
KDEVCPPDUCHAIN_EXPORT QString shortenedTypeString(KDevelop::AbstractType::Ptr type, KDevelop::DUContext* ctx, int desiredLength = 10000, KDevelop::QualifiedIdentifier stripPrefix = KDevelop::QualifiedIdentifier());
KDEVCPPDUCHAIN_EXPORT KDevelop::IndexedTypeIdentifier shortenedTypeIdentifier(KDevelop::AbstractType::Ptr type, KDevelop::DUContext* ctx, int desiredLength = 10000, KDevelop::QualifiedIdentifier stripPrefix = KDevelop::QualifiedIdentifier());

///Returns a simplified string version of the given type: Template default-parameters are stripped away, qualified identifiers are simplified so they are as short as possible, while staying visible from the given context.
KDEVCPPDUCHAIN_EXPORT QString simplifiedTypeString(KDevelop::AbstractType::Ptr type, KDevelop::DUContext* visibilityFrom);

KDEVCPPDUCHAIN_EXPORT bool isFriend(KDevelop::Declaration* _class, KDevelop::Declaration* _friend);

///Shortens the type by resolving typedefs that are not useful
KDEVCPPDUCHAIN_EXPORT KDevelop::AbstractType::Ptr shortenTypeForViewing(KDevelop::AbstractType::Ptr type);

///Returns a type that has all template types replaced with DelayedType's that have their template default parameters stripped away,
///and all scope prefixes removed that are redundant within the given context.
///The given type must be a "simple" type (neither a pointer, nor a reference type). Use shortenedTypeString for extended needs.
///The returned type should not actively be used in the  type-system, but rather only for displaying.
KDevelop::AbstractType::Ptr stripType(KDevelop::AbstractType::Ptr type, KDevelop::DUContext* ctx);

///Returns the template context attached to this declaration, if any
KDEVCPPDUCHAIN_EXPORT KDevelop::DUContext* getTemplateContext(KDevelop::Declaration* decl, const KDevelop::TopDUContext* source = 0);
///Returns the template context import by this context, if any
KDEVCPPDUCHAIN_EXPORT KDevelop::DUContext* getTemplateContext(KDevelop::DUContext* ctx, const KDevelop::TopDUContext* source = 0);
///Removes all prefixes of the qualified identifier that are not required within the current context
///(The returned identifier may be empty if the id identifies a namespace that is imported into the current context)
KDEVCPPDUCHAIN_EXPORT KDevelop::QualifiedIdentifier stripPrefixes(KDevelop::DUContext* ctx, KDevelop::QualifiedIdentifier id);
}

#endif
