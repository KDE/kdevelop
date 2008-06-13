/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>
    Copyright 2007-2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#ifndef DECLARATION_H
#define DECLARATION_H

#include <QtCore/QList>
#include <qcontainerfwd.h>
#include <editor/documentrangeobject.h>

#include "language/duchain/identifier.h"
#include "language/duchain/typesystem.h"
#include "language/duchain/duchainbase.h"

class QByteArray;

namespace KTextEditor {
  class SmartRange;
}

namespace KDevelop
{

class AbstractType;
class DUContext;
class Use;
class ForwardDeclaration;
class DeclarationPrivate;
class DeclarationId;

struct ImportTraceItem;


typedef QVarLengthArray<ImportTraceItem, 40> ImportTrace;

/**
 * Represents a single declaration in a definition-use chain.
 *
 * NOTE: A du-context can be freely edited as long as it's parent-context is zero.
 * In the moment the parent-context is set, the context may only be edited when it
 * is allowed to edited it's top-level context(@see TopLevelContext::inDUChain()
 */
class KDEVPLATFORMLANGUAGE_EXPORT Declaration : public DUChainBase
{

public:
  enum Scope {
    GlobalScope    /**< a global declaration */,
    NamespaceScope /**< a namespace declaration */,
    ClassScope     /**< a class declaration */,
    FunctionScope  /**< a function declaration */,
    LocalScope     /**< a local declaration */
  };
  enum AccessPolicy {
    Public    /**< a public declaration */,
    Protected /**< a protected declaration */,
    Private   /**< a private declaration */
  };
  enum CVSpec {
    CVNone = 0     /**< no CV given */,
    Const = 0x1    /**< a const declaration */,
    Volatile = 0x2 /**< a volatile declaration */
  };

  enum Kind {
    Type     /**< A type is declared, like a class-declaration or function-declaration, or a typedef("class MyClass {};") */,
    Instance /**< An instance of a type is declared("MyClass m;") */,
    NamespaceAlias/**< This is a namespace-alias. You can safely cast this object to NamespaceAliasDeclaration. */
  };

  Q_DECLARE_FLAGS(CVSpecs, CVSpec)

  /**
   * If @param parentContext is in the symbol table, the declaration will automatically be added into the symbol table.
   * */
  Declaration(const HashedString& url, const SimpleRange& range, Scope scope, DUContext* parentContext);
  ///Copy-constructor for cloning
  Declaration(const Declaration& rhs);
  virtual ~Declaration();

  virtual TopDUContext* topContext() const;

  virtual bool isForwardDeclaration() const;
  ForwardDeclaration* toForwardDeclaration();
  const ForwardDeclaration* toForwardDeclaration() const;

  ///Returns true if this declaration is accessible through the du-chain, and thus cannot be edited without a du-chain write lock
  virtual bool inDUChain() const;

  bool isDefinition() const;
  void setDeclarationIsDefinition(bool dd);

  ///Is this a type-alias(in c++ typedef)?
  bool isTypeAlias() const;
  void setIsTypeAlias(bool);

  /**
   * If this is a definition, and has an attached declaration, that declaration is returned.
   * Else zero.
   * @param topContext the top-context from which to search
   * */
  Declaration* declaration(TopDUContext* topContext = 0) const;

  /**
   * If this is a declaration, and has an attached definition, that definition is returned.
   * Else zero.
   * */
  Declaration* definition() const;

  /**
   * Set the definition for this declaration.
   * 
   * Definitions and declarations are coupled by identity
   * rather than by their pointer. When you call this, you will
   * effectively set the definition for ALL declarations that have:
   * - The same qualifiedIdentifier()
   * - Are declared within the same file
   * - Have the same additionalIdentity() value
   */
  void setDefinition(Declaration* definition);

  /**
   * This returns the context that is opened by this declaration, or zero.
   * */
  DUContext * internalContext() const;
  void setInternalContext(DUContext* context);

  /**
   * If this declaration has a definition, and the definition is resolved, it returns the internal context of the definition.
   * If this declaration is a forward-declaration, the forward-declaration is resolved, and the internal context of the resolved declaration.
   * If this is a type-alias, returns the internal context of the actual type.
   * is returned.
   * Else, returns the same as internalContext().
   * @param topContext Needed to resolve forward-declarations.
   * */
  virtual DUContext * logicalInternalContext(const TopDUContext* topContext) const;

  /**
   * Convenience function:
   * If this is a forward-declaration that can be resolved, returns the resolved forward-declaration.
   * Else returns this.
   * @param topContext Needed to resolve forward-declarations.
   * */
  const Declaration* logicalDeclaration(const TopDUContext* topContext) const;
  Declaration* logicalDeclaration(const TopDUContext* topContext);
  
  /**
   * Returns the parent-context of this declaration.
   * */
  DUContext* context() const;
  /**
   * When setContext(..) is called, this declaration is inserted into the given context
   * You only need to be able to write this declaration. You do not need write-privileges for the context, because addDeclaration(..) works separately from that.
   * @param anonymous If this is set, this declaration will be added anonymously into the parent-context. That way it can never be found through any of the context's functions, and will not be deleted when the context is deleted, so it must be deleted from elsewhere.
   *
   * If the given context is not in the symbol-table, or if the declaration is inserted anonymously, or if the context is zero, this declaration is removed from the symbol-table.
   * Else it is added to the symbol table with the new scope. @see TopDUContext for information about the symbol table.
   * */
  void setContext(DUContext* context, bool anonymous = false);

  Scope scope() const;

  template <class T>
  KSharedPtr<T> type() const { return KSharedPtr<T>::dynamicCast(abstractType()); }

  template <class T>
  void setType(KSharedPtr<T> type) { setAbstractType(AbstractType::Ptr::staticCast(type)); }

  AbstractType::Ptr abstractType() const;
  virtual void setAbstractType(AbstractType::Ptr type);

  void setIdentifier(const Identifier& identifier);
  const Identifier& identifier() const;

  ///Returns the global qualified identifier of this declaration. Is expensive.
  QualifiedIdentifier qualifiedIdentifier() const;

  ///Compares the qualified identifier of this declaration with the other one, without needing to compute it.
  ///This is more efficient than comparing the results of qualifiedIdentifier().
  bool equalQualifiedIdentifier(const Declaration* rhs) const;
  
  /**
   * Returns the kind of this declaration. @see Kind
   * */
  Kind kind() const;

  /**
   * Set the kind.
   * */
  void setKind(Kind kind);

  /**
   * Returns the comment associated to this declaration in the source-code, or an invalid string if there is none.
   * Stored in utf-8 encoding
   * */
  QByteArray comment() const;
  /**
   * Sets the comment for this declaration. Should be utf-8 encoded.
   * */
  void setComment(const QByteArray& str);
  void setComment(const QString& str);

  /**
   * Provides a mangled version of this definition's identifier, for use in a symbol table.
   */
  QString mangledIdentifier() const;

  bool inSymbolTable() const;
  //Adds or removes this declaration to/from the symbol table
  void setInSymbolTable(bool inSymbolTable);

  bool operator==(const Declaration& other) const;

  virtual QString toString() const;

  ///@todo The following two are convenience-functions. Think whether they should stay here, or be moved out.
  
  /**
   * Returns a list of pairs:
   * An url of a file, paired together with all use-ranges of this declaration in that file.
   * The uses are unique, no 2 uses are returend that have the same range within the same file.
   *
   * This is a non-trivial operation.
   * */
  QMap<HashedString, QList<SimpleRange> > uses() const;

  /**
   * Collects the smart-ranges for all uses. Uses that do not have smart ranges are not represented
   * in the result.
   * */
  QList<KTextEditor::SmartRange*> smartUses() const;
  
  /**
    * This hash-value should differentiate between multiple different
    * declarations that have the same qualifiedIdentifier, but should have a different
    * identity, and thus own Definitions and own Uses assigned.
    *
    * Affected by function-arguments, whether this is a template-declaration, etc..
    * */
  virtual uint additionalIdentity() const;

  /**
   * @see DeclarationId
   * */
  virtual DeclarationId id() const;
  
  /**
   * Returns a clone of this declaration, with the difference that:
   * - context will be zero
   * - internalContext will be zero
   * - definition will be zero
   * - forwardDeclarations will be zero
   * - the list of uses will be empty
   *
   * The declaration will not be registered anywhere, so you must care about its deletion.
   *
   * This declaration's text-range will be referenced from the clone, so the clone must not live longer than the original.
   *
   * Sub-classes should implement this and should copy as much information into the clone as possible without breaking the du-chain.
   * Sub-classes should also implement a public copy-constructor that can be used for cloning by sub-classes.
   *
   * ---> You do not have to implement this for your language if you are not going to use it(the du-chain itself does not and should not depend on it).
   * */
  virtual Declaration* clone() const;
protected:
    Declaration( DeclarationPrivate & dd );
    Declaration( DeclarationPrivate & dd, const HashedString& url, const SimpleRange& range, Scope scope );

private:
  Q_DECLARE_PRIVATE(Declaration)
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(KDevelop::Declaration::CVSpecs)

#endif // DECLARATION_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
