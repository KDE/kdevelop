/* This file is part of KDevelop
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

#ifndef DECLARATION_H
#define DECLARATION_H

#include <QtCore/QList>
#include "../languageexport.h"

#include "identifier.h"
#include <documentrangeobject.h>
#include "typesystem.h"
#include "duchainbase.h"
#include "contextowner.h"

class QByteArray;

namespace KDevelop
{

class AbstractType;
class DUContext;
class Use;
class ForwardDeclaration;
class Definition;
class DeclarationPrivate;
/**
 * Represents a single declaration in a definition-use chain.
 *
 * NOTE: A du-context can be freely edited as long as it's parent-context is zero.
 * In the moment the parent-context is set, the context may only be edited when it
 * is allowed to edited it's top-level context(@see TopLevelContext::inDUChain()
 */
class KDEVPLATFORMLANGUAGE_EXPORT Declaration : public DUChainBase, public ContextOwner
{

public:
  enum Scope {
    GlobalScope,
    NamespaceScope,
    ClassScope,
    FunctionScope,
    LocalScope
  };
  enum AccessPolicy {
    Public,
    Protected,
    Private
  };
  enum CVSpec {
    CVNone = 0,
    Const = 0x1,
    Volatile = 0x2
  };

  enum Kind {
    Type, //A type is declared, like a class-declaration or function-declaration, or a typedef("class MyClass {};")
    Instance //An instance of a type is declared("MyClass m;")
  };

  Q_DECLARE_FLAGS(CVSpecs, CVSpec)

  Declaration(const HashedString& url, KTextEditor::Range* range, Scope scope, DUContext* context);
  ///Copy-constructor for cloning
  Declaration(const Declaration& rhs);
  virtual ~Declaration();

  virtual TopDUContext* topContext() const;

  const QList<ForwardDeclaration*>& forwardDeclarations() const;
  void addForwardDeclaration(ForwardDeclaration* );
  void removeForwardDeclaration(ForwardDeclaration* );

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
   * Retrieve the definition for this declaration.
   */
  Definition* definition() const;

  /**
   * Set the definition for this declaration.
   */
  void setDefinition(Definition* definition);

  /**
   * If this is a forward-declaration, it returns the internal context of the resolved declaration.
   * If this is a definition, and the definition is resolved, it returns the internal context of the definition.
   * */
  virtual DUContext * internalContext() const;

  /**
   * Returns the parent-context of this declaration.
   * */
  DUContext* context() const;
  /**
   * When setContext(..) is called, this declaration is inserted into the given context.
   * You only need to be able to write this declaration. You do not need write-privileges for the context, because addDeclaration(..) works separately from that.
   * @param anonymous If this is set, this declaration will be added anonymously into the parent-context. That way it can never be found through any of the context's functions, and will not be deleted when the context is deleted, so it must be deleted from elsewhere.
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

  ///Returns the global qualified identifier of this declaration
  QualifiedIdentifier qualifiedIdentifier() const;

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
  void setInSymbolTable(bool inSymbolTable);

  const QList<Use*>& uses() const;
  void addUse(Use* range);
  void removeUse(Use* range);

  bool operator==(const Declaration& other) const;

  virtual QString toString() const;

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
    Declaration( DeclarationPrivate & dd, const HashedString& url, KTextEditor::Range* range, Scope scope );
private:
  Q_DECLARE_PRIVATE(Declaration)
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(KDevelop::Declaration::CVSpecs)

#endif // DECLARATION_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
