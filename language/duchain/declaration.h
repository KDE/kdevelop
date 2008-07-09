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

#include "language/editor/documentrangeobject.h"
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
class Declaration;

struct ImportTraceItem;


typedef QVarLengthArray<ImportTraceItem, 40> ImportTrace;


///Represents a declaration only by its global indices
class KDEVPLATFORMLANGUAGE_EXPORT IndexedDeclaration {
  public:
    IndexedDeclaration(uint topContext = 0, uint declarationIndex = 0);
    //Duchain must be read locked
    Declaration* declaration() const;
    bool operator==(const IndexedDeclaration& rhs) const {
      return m_topContext == rhs.m_topContext && m_declarationIndex == rhs.m_declarationIndex;
    }
    uint hash() const {
      return (m_topContext * 53 + m_declarationIndex) * 23;
    }

    bool isValid() const {
      return m_topContext != 0 || m_declarationIndex != 0;
    }

  private:
  uint m_topContext;
  uint m_declarationIndex;
};

/**
 * \short Represents a single declaration in a definition-use chain.
 *
 * \note A du-context can be freely edited as long as it's parent-context is zero.
 * In the moment the parent-context is set, the context may only be edited when it
 * is allowed to edited it's top-level context (@see TopLevelContext::inDUChain())
 */
class KDEVPLATFORMLANGUAGE_EXPORT Declaration : public DUChainBase
{
public:
  /// Access types
  enum AccessPolicy {
    Public    /**< a public declaration */,
    Protected /**< a protected declaration */,
    Private   /**< a private declaration */
  };
  /// Const and volatile flags
  enum CVSpec {
    CVNone = 0     /**< no CV given */,
    Const = 0x1    /**< a const declaration */,
    Volatile = 0x2 /**< a volatile declaration */
  };
  /// Enumeration of the types of declarations
  enum Kind {
    Type     /**< A type is declared, like a class-declaration or function-declaration, or a typedef("class MyClass {};") */,
    Instance /**< An instance of a type is declared("MyClass m;") */,
    NamespaceAlias/**< This is a namespace-alias. You can safely cast this object to NamespaceAliasDeclaration. */,
    Alias /**<This is an alias-declaration. You can safely cast this object to AliasDeclaration. */
  };

  Q_DECLARE_FLAGS(CVSpecs, CVSpec)

  /**
   * Constructor.
   *
   * If \a parentContext is in the symbol table, the declaration will automatically be added into the symbol table.
   *
   * \param url url of the document where this occurred
   * \param range range of the alias declaration's identifier
   * \param parentContext context in which this declaration occurred
   * */
  Declaration(const HashedString& url, const SimpleRange& range, DUContext* parentContext);
  ///Copy-constructor for cloning
  Declaration(const Declaration& rhs);
  /// Destructor
  virtual ~Declaration();

  virtual TopDUContext* topContext() const;

  /// Determine whether this declaration is a forward declaration. \returns true if this is a forward declaration, otherwise returns false.
  virtual bool isForwardDeclaration() const;
  /// Returns this declaration as a forward declaration, if it is one. \returns this declaration as a forward declaration if it is one, otherwise null.
  ForwardDeclaration* toForwardDeclaration();
  /// Returns this declaration as a forward declaration, if it is one. \returns this declaration as a forward declaration if it is one, otherwise null.
  const ForwardDeclaration* toForwardDeclaration() const;

  /// Determine whether this declaration is a function declaration. \returns true if this is a function declaration, otherwise returns false.
  virtual bool isFunctionDeclaration() const;

  /**
   * Determine whether this declaration is accessible through the du-chain.
   * If it is, it cannot be edited without holding the du-chain write lock.
   *
   * \sa DUChain::lock()
   * \sa DUChainWriteLocker
   *
   * \returns true if the Declaration is already inserted into a duchain.
   */
  virtual bool inDUChain() const;

  /// Access whether this declaration is also a definition. \returns true if this declaration is also a definition, otherwise false.
  bool isDefinition() const;
  /// Set whether this declaration is also a definition. \param dd set to true if this declaration is also a definition, otherwise false.
  void setDeclarationIsDefinition(bool dd);

  /// Determine if this declaration is a type-alias (in c++ typedef).  \returns true if the declaration is a type alias, otherwise false.
  bool isTypeAlias() const;
  /// Set whether this declaration is a type alias. \param typeAlias true if the declaration is a type alias, otherwise false.
  void setIsTypeAlias(bool typeAlias);

  /**
   * Find the declaration for this definition, if one exists.
   *
   * @param topContext the top-context from which to search
   * \returns the declaration matching this definition, otherwise null if no matching declaration has been found.
   * */
  Declaration* declaration(TopDUContext* topContext = 0) const;

  /**
   * Find the definition for this declaration, if one exists.
   *
   * \returns the definition matching this declaration, otherwise null if no matching definition has been found.
   * */
  Declaration* definition() const;

  /**
   * Retrieve the declaration which is specialized with the given \a specialization index in the given \a topContext.
   *
   * \param specialization the specialization index (see DeclarationId)
   * \param topContext the top context to search in
   * */
  virtual Declaration* specialize(uint specialization, const TopDUContext* topContext);

  /**
   * Set the definition for this declaration.
   *
   * Definitions and declarations are coupled by identity
   * rather than by their pointer. When you call this, you will
   * effectively set the definition for ALL declarations that have:
   * - The same qualifiedIdentifier()
   * - Are declared within the same file
   * - Have the same additionalIdentity() value
   *
   * \param definition the definition for this declaration.
   */
  void setDefinition(Declaration* definition);

  /**
   * Retrieve the context that is opened by this declaration, if one exists.
   *
   * For example, a class will have a declaration which is contained within the context in which
   * it is declared, and a new context will be created to hold class members.  This function returns
   * that context.
   *
   * \returns the internal context for this declaration
   * */
  DUContext * internalContext() const;

  /**
   * Set the internal \a context for this declaration.
   *
   * \param context the internal context
   */
  void setInternalContext(DUContext* context);

  /**
   * Determine the logical internal context for the resolved form of this declaration.
   *
   * * If this declaration has a definition, and the definition is resolved, it returns the internal context of the definition.
   * * If this declaration is a forward-declaration, the forward-declaration is resolved, it returns the internal context of the resolved declaration.
   * * If this is a type-alias, it returns the internal context of the actual type.
   * * Otherwise, it returns the same as internalContext().
   *
   * \param topContext Needed to resolve forward-declarations.
   * \returns the resolved internal context, as described above
   * */
  virtual DUContext * logicalInternalContext(const TopDUContext* topContext) const;

  /**
   * This is a convenience function to determine the resolved declaration, if this is a forward declaration.
   * Otherwise, it just returns this declaration.
   * \param topContext Context within which to search for the resolved declaration.
   * \returns the resolved declaration if one was found, otherwise this declaration.
   * */
  const Declaration* logicalDeclaration(const TopDUContext* topContext) const;

  /// \copydoc
  Declaration* logicalDeclaration(const TopDUContext* topContext);

  /**
   * Access the parent context of this declaration.
   * \returns the parent context of this declaration.
   * */
  DUContext* context() const;

  /**
   * Create an indexed reference to this declaration.
   * \returns the indexed reference to this declaration.
   */
  IndexedDeclaration indexed() const;

  /**
   * Set the context in which this declaration occurs.
   *
   * When setContext() is called, this declaration is inserted into the given context.
   * You only need to be able to write this declaration. You do not need write-privileges
   * for the context, because addDeclaration(..) works separately to that.
   *
   * If the given context is not in the symbol-table, or if the declaration is inserted anonymously,
   *  or if the context is zero, this declaration is removed from the symbol-table.
   * Else it is added to the symbol table with the new scope. See TopDUContext for information about the symbol table.
   *
   * \param context New context which contains this declaration.
   * \param anonymous If this is set, this declaration will be added anonymously into the parent-context.
   *                  This way it can never be found through any of the context's functions, and will
   *                  not be deleted when the context is deleted, so it must be deleted from elsewhere.
   * */
  void setContext(DUContext* context, bool anonymous = false);

  /// Convenience function to return this declaration's type dynamically casted to \a T. \returns this declaration's type as \a T, or null if there is no type or it is not of type \a T.
  template <class T>
  TypePtr<T> type() const { return TypePtr<T>::dynamicCast(abstractType()); }

  /**
   * Access this declaration's type.
   *
   * \note You should not compare or permanently store instances of AbstractType::Ptr. Use IndexedType instead.
   * \returns this declaration's type, or null if none has been assigned.
   */
  AbstractType::Ptr abstractType() const;

  /**
   * Set this declaration's type.
   * \param type the type to assign.
   */
  template <class T>
  void setType(TypePtr<T> type) { setAbstractType(AbstractType::Ptr::staticCast(type)); }

  /**
   * Set this declaration's \a type.
   *
   * \param type this declaration's new type.
   */
  virtual void setAbstractType(AbstractType::Ptr type);

  /**
   * Return an indexed form of this declaration's type.
   * Should be preferred, this is the fastest way, and the correct way for doing equality-comparsion.
   *
   * \returns the declaration's type.
   */
  IndexedType indexedType() const;

  /**
   * Set this declaration's \a identifier.
   *
   * \param identifier this declaration's new identifier
   */
  void setIdentifier(const Identifier& identifier);

  /**
   * Access this declaration's \a identifier.
   *
   * \returns this declaration's identifier, or a null identifier if it has none.
   */
  const Identifier& identifier() const;

  /**
   * Determine the global qualified identifier of this declaration.
   *
   * \note This function is expensive, equalQualifiedIdentifier() is preferred if you
   *       just want to compare equality.
   */
  QualifiedIdentifier qualifiedIdentifier() const;

  /**
   * Compares the qualified identifier of this declaration with the other one, without needing to compute it.
   * This is more efficient than comparing the results of qualifiedIdentifier().
   *
   * \param rhs declaration to compare identifiers with
   * \returns true if the identifiers are equal, otherwise false.
   */
  bool equalQualifiedIdentifier(const Declaration* rhs) const;

  /**
   * Returns the kind of this declaration. @see Kind
   * */
  Kind kind() const;

  /**
   * Set the kind.
   *
   * \param kind new kind
   * */
  void setKind(Kind kind);

  /**
   * Returns the comment associated to this declaration in the source-code, or an invalid string if there is none.
   * Stored in utf-8 encoding.
   * */
  QByteArray comment() const;

  /**
   * Sets the comment for this declaration. Should be utf-8 encoded.
   * */
  void setComment(const QByteArray& str);
  /// Sets the comment for this declaration.
  void setComment(const QString& str);

  /**
   * Access whether this declaration is in the symbol table.
   *
   * \returns true if this declaration is in the symbol table, otherwise false.
   */
  bool inSymbolTable() const;

  /**
   * Adds or removes this declaration to/from the symbol table.
   *
   * \param inSymbolTable true to add this declaration to the symbol table, false to remove it.
   */
  void setInSymbolTable(bool inSymbolTable);

  /**
   * Equivalence operator.
   * \param other Other declaration to compare.
   * \returns true if the declarations are equal, otherwise false.
   */
  bool operator==(const Declaration& other) const;

  /**
   * Determine this declaration as a string. \returns this declaration as a string.
   */
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
   *
   * */
  virtual uint specialization() const;

  /**
   * @see DeclarationId
   * */
  virtual DeclarationId id() const;

  /**
   * Returns an index that uniquely identifies this declaration within its surrounding top-context. That index can be passed
   * to TopDUContext::declarationFromIndex(index) to get the declaration.
   * This is only valid when the declaration is not a specialization (specialization() returns 0), and if it is not anonymous in its context.
   *
   * \note for this to be valid, allocateOwnIndex() must have been called first.
   * \returns the index of the declaration within its TopDUContext.
   */
  uint ownIndex() const;

  /**
   * Clear the index for this declaration in the top context that was allocated with allocateOwnIndex().
   */
  void clearOwnIndex();

  /**
   * Create an index to this declaration from the topContext().  Needed to be able to retrieve ownIndex().
   */
  void allocateOwnIndex();

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
   * \note You do not have to implement this for your language if you are not going to use it(the du-chain itself does not and should not depend on it).
   * */
  virtual Declaration* clone() const;

protected:
  /**
   * Copy constructor.
   *
   * \param dd private data to copy
   */
  Declaration( DeclarationPrivate & dd );

  /**
    * Constructor for copy constructors in subclasses.
    *
    * \param dd data to copy.
    * \param url document url in which this object is located.
    * \param range text range which this object covers.
    */
  Declaration( DeclarationPrivate & dd, const HashedString& url, const SimpleRange& range );

private:
  Q_DECLARE_PRIVATE(Declaration)
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(KDevelop::Declaration::CVSpecs)

#endif // DECLARATION_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
