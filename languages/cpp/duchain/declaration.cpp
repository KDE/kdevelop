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

#include "declaration.h"

#include <ktexteditor/smartrange.h>
#include <ktexteditor/document.h>

#include "ducontext.h"
#include "use.h"
#include "definition.h"
#include "cpptypes.h"
#include "symboltable.h"
#include "forwarddeclaration.h"

using namespace KTextEditor;

Declaration::Declaration(KTextEditor::Range* range, Scope scope, DUContext* context )
  : DUChainBase(context->topContext())
  , KDevDocumentRangeObject(range)
  , m_context(0)
  , m_scope(scope)
  , m_definition(0)
  , m_isDefinition(false)
  , m_inSymbolTable(false)
{
  Q_ASSERT(context);
  setContext(context);
}

Declaration::~Declaration()
{
  // Inserted by the builder after construction has finished.
  if (m_inSymbolTable)
    SymbolTable::self()->removeDeclaration(this);

  foreach (ForwardDeclaration* forward, m_forwardDeclarations)
    forward->setResolved(0);
  Q_ASSERT(m_forwardDeclarations.isEmpty());

  // context is only null in the test cases
  if (context())
    context()->removeDeclaration(this);

  setContext(0);

  setAbstractType(AbstractType::Ptr());

  qDeleteAll(m_uses);
}

void Declaration::removeUse( Use* range )
{
  range->setDeclaration(0L);
  m_uses.removeAll(range);
}

void Declaration::addUse( Use* range )
{
  range->setDeclaration(this);
  m_uses.append(range);
}

const QList< Use* > & Declaration::uses( ) const
{
  return m_uses;
}

const Identifier& Declaration::identifier( ) const
{
  return m_identifier;
}

void Declaration::setIdentifier(const Identifier& identifier)
{
  m_identifier = identifier;
}

AbstractType::Ptr Declaration::abstractType( ) const
{
  return m_type;
}

void Declaration::setAbstractType(AbstractType::Ptr type)
{
  if (CppIdentifiedType* idType = dynamic_cast<CppIdentifiedType*>(m_type.data()))
    idType->setDeclaration(0);

  m_type = type;

  if (CppIdentifiedType* idType = dynamic_cast<CppIdentifiedType*>(m_type.data()))
    idType->setDeclaration(this);
}

Declaration::Scope Declaration::scope( ) const
{
  return m_scope;
}

QualifiedIdentifier Declaration::qualifiedIdentifier() const
{
  QualifiedIdentifier ret = context()->scopeIdentifier(true);
  ret.push(identifier());
  return ret;
}

QString Declaration::mangledIdentifier() const
{
/*
GNU mangling specs from http://theory.uwinnipeg.ca/gnu/gcc/gxxint_15.html

`A'
 Indicates a C++ array type.
`b'
 Encodes the C++ bool type, and the Java boolean type.
`c'
 Encodes the C++ char type, and the Java byte type.
`C'
 A modifier to indicate a const type. Also used to indicate a const member function (in which cases it precedes the encoding of the method's class).
`d'
 Encodes the C++ and Java double types.
`e'
 Indicates extra unknown arguments ....
`f'
 Encodes the C++ and Java float types.
`F'
 Used to indicate a function type.
`H'
 Used to indicate a template function.
`i'
 Encodes the C++ and Java int types.
`J'
 Indicates a complex type.
`l'
 Encodes the C++ long type.
`P'
 Indicates a pointer type. Followed by the type pointed to.
`Q'
 Used to mangle qualified names, which arise from nested classes. Should also be used for namespaces (?). In Java used to mangle package-qualified names, and inner classes.
`r'
 Encodes the GNU C++ long double type.
`R'
 Indicates a reference type. Followed by the referenced type.
`s'
 Encodes the C++ and java short types.
`S'
 A modifier that indicates that the following integer type is signed. Only used with char. Also used as a modifier to indicate a static member function.
`t'
 Indicates a template instantiation.
`T'
 A back reference to a previously seen type.
`U'
 A modifier that indicates that the following integer type is unsigned. Also used to indicate that the following class or namespace name is encoded using Unicode-mangling.
`v'
 Encodes the C++ and Java void types.
`V'
 A modified for a const type or method.
`w'
 Encodes the C++ wchar_t type, and the Java char types.
`x'
 Encodes the GNU C++ long long type, and the Java long type.
`X'
 Encodes a template type parameter, when part of a function type.
`Y'
 Encodes a template constant parameter, when part of a function type.
`Z'
 Used for template type parameters.
*/
  if (abstractType())
    return abstractType()->mangled();

  // Error...
  return qualifiedIdentifier().mangled();
}

DUContext * Declaration::context() const
{
  return m_context;
}

void Declaration::setContext(DUContext* context)
{
  if (m_context && context)
    Q_ASSERT(m_context->topContext() == context->topContext());

  if (m_context)
    m_context->removeDeclaration(this);

  m_context = context;

  if (m_context)
    m_context->addDeclaration(this);
}

bool Declaration::operator ==(const Declaration & other) const
{
  return this == &other;
}

QString Declaration::toString() const
{
  return QString("Declaration: %3 %4").arg(abstractType() ? abstractType()->toString() : QString("<notype>")).arg(identifier().toString());
}

// kate: indent-width 2;

bool Declaration::isDefinition() const
{
  return m_isDefinition;
}

void Declaration::setDeclarationIsDefinition(bool dd)
{
  m_isDefinition = dd;
  if (m_isDefinition && m_definition) {
    setDefinition(0);
  }
}

Definition* Declaration::definition() const
{
  return m_definition;
}

void Declaration::setDefinition(Definition* definition)
{
  if (m_definition) {
    m_definition->setDeclaration(0);
    delete m_definition;
    m_definition = 0;
  }

  m_definition = definition;

  if (m_definition) {
    m_isDefinition = false;
  }
}

bool Declaration::inSymbolTable() const
{
  return m_inSymbolTable;
}

void Declaration::setInSymbolTable(bool inSymbolTable)
{
  m_inSymbolTable = inSymbolTable;
}

const QList< ForwardDeclaration * > & Declaration::forwardDeclarations() const
{
  return m_forwardDeclarations;
}

bool Declaration::isForwardDeclaration() const
{
  return false;
}

ForwardDeclaration* Declaration::forwardDeclaration()
{
  return static_cast<ForwardDeclaration*>(this);
}

const ForwardDeclaration* Declaration::forwardDeclaration() const
{
  return static_cast<const ForwardDeclaration*>(this);
}
