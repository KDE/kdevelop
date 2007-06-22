/* This file is part of KDevelop
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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
#include <languageexport.h>

#include <identifier.h>
#include <documentrangeobject.h>
#include <typesystem.h>
#include <duchainbase.h>

namespace KDevelop
{

class AbstractType;
class DUContext;
class Use;
class Definition;
class ForwardDeclaration;

/**
 * Represents a single declaration in a definition-use chain.
 */
class KDEVPLATFORMLANGUAGE_EXPORT Declaration : public DUChainBase
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
    Type, //A type is declared, like a class-declaration or function-declaration("class MyClass {};")
    Instance //An instance of a type is declared("MyClass m;")
  };
  
  Q_DECLARE_FLAGS(CVSpecs, CVSpec)

  Declaration(KTextEditor::Range* range, Scope scope, DUContext* context);
  virtual ~Declaration();

  virtual TopDUContext* topContext() const;

  const QList<ForwardDeclaration*>& forwardDeclarations() const;
  void addForwardDeclaration(ForwardDeclaration* );
  void removeForwardDeclaration(ForwardDeclaration* );

  virtual bool isForwardDeclaration() const;
  ForwardDeclaration* toForwardDeclaration();
  const ForwardDeclaration* toForwardDeclaration() const;

  bool isDefinition() const;
  void setDeclarationIsDefinition(bool dd);

  /**
   * Retrieve the definition for this use.
   */
  Definition* definition() const;

  /**
   * Set the definition for this use.
   */
  void setDefinition(Definition* definition);

  DUContext* context() const;
  void setContext(DUContext* context);

  Scope scope() const;

  template <class T>
  KSharedPtr<T> type() const { return KSharedPtr<T>::dynamicCast(abstractType()); }

  template <class T>
  void setType(KSharedPtr<T> type) { setAbstractType(AbstractType::Ptr::staticCast(type)); }

  AbstractType::Ptr abstractType() const;
  void setAbstractType(AbstractType::Ptr type);

  void setIdentifier(const Identifier& identifier);
  const Identifier& identifier() const;

  QualifiedIdentifier qualifiedIdentifier() const;

  /**
   * Returns the kind of this declaration. @see Kind
   * @todo the logic behind this does not work anymore once a type may be defined several times in different du-contexts
   * */
  Kind kind() const;
  
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

private:
  class DeclarationPrivate* const d;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(KDevelop::Declaration::CVSpecs)

#endif // DECLARATION_H

// kate: space-indent on; indent-width 2; tab-width: 4; replace-tabs on; auto-insert-doxygen on
