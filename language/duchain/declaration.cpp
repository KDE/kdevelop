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

#include "declaration.h"
#include "declaration_p.h"

#include <QByteArray>

#include <ktexteditor/smartrange.h>
#include <ktexteditor/document.h>

#include <hashedstring.h>

#include "topducontext.h"
#include "use.h"
#include "definition.h"
#include "symboltable.h"
#include "forwarddeclaration.h"
#include "duchain.h"
#include "duchainlock.h"
#include "identifiedtype.h"
#include "definition_p.h"
#include "ducontext_p.h"
#include "use_p.h"

using namespace KTextEditor;

namespace KDevelop
{

DeclarationPrivate::DeclarationPrivate() 
  : m_isDefinition(false), m_inSymbolTable(false),  
    m_isTypeAlias(false), m_anonymousInContext(false) 
{
  m_context = 0;
  m_definition = 0;
  m_kind = Declaration::Instance;
}
  
  
DeclarationPrivate::DeclarationPrivate( const DeclarationPrivate& rhs ) : DUChainBasePrivate(rhs)
{
  m_identifier = rhs.m_identifier;
  m_type = rhs.m_type;
  m_scope = rhs.m_scope;
  m_kind = rhs.m_kind;
  m_isDefinition = rhs.m_isDefinition;
  m_isTypeAlias = rhs.m_isTypeAlias;
  m_inSymbolTable = false;
  m_context = 0;
  m_definition = 0;
  m_anonymousInContext = rhs.m_anonymousInContext;
}
  
Declaration::Kind Declaration::kind() const {
  Q_D(const Declaration);
  return d->m_kind;
}

void Declaration::setKind(Kind kind) {
  Q_D(Declaration);
  d->m_kind = kind;
}

bool Declaration::inDUChain() const {
  Q_D(const Declaration);
  if( d->m_anonymousInContext )
    return false;
  TopDUContext* top = topContext();
  return top && top->inDuChain();
}

Declaration::Declaration( const HashedString& url, const SimpleRange& range, Scope scope, DUContext* context )
  : DUChainBase(*new DeclarationPrivate, url, range)
  , ContextOwner(this)
{
  d_func()->m_scope = scope;
  if(context)
    setContext(context);
}

Declaration::Declaration(const Declaration& rhs) 
  : DUChainBase(*new DeclarationPrivate( *rhs.d_func() )),
    ContextOwner(this) {
  setSmartRange(rhs.smartRange(), DocumentRangeObject::DontOwn);
}

Declaration::Declaration( DeclarationPrivate & dd ) : DUChainBase(dd), ContextOwner(this)
{
}

Declaration::Declaration( DeclarationPrivate & dd, const HashedString& url, const SimpleRange& range, Scope scope )
  : DUChainBase(dd, url, range), ContextOwner(this)
{
  Q_D(Declaration);
  d->m_scope = scope;
}

Declaration::~Declaration()
{
  Q_D(Declaration);
  // Inserted by the builder after construction has finished.
  if (d->m_inSymbolTable)
    SymbolTable::self()->removeDeclaration(this);

  QList<Use*> _uses = uses();
  foreach (Use* use, _uses)
    use->d_func()->setDeclaration(0);

  if (Definition* def = definition())
    def->d_func()->setDeclaration(0);

  // context is only null in the test cases
  if (context())
    context()->d_func()->removeDeclaration(this);

  setContext(0);

  if( IdentifiedType* type = dynamic_cast<IdentifiedType*>(d->m_type.data()) )
    if( type->declaration() == this )
      type->setDeclaration(0);

  setAbstractType(AbstractType::Ptr());

  //DUChain::declarationChanged(this, DUChainObserver::Deletion, DUChainObserver::NotApplicable);
}

QByteArray Declaration::comment() const {
  Q_D(const Declaration);
  return d->m_comment;
}

void Declaration::setComment(const QByteArray& str) {
  Q_D(Declaration);
  d->m_comment = str;
}

void Declaration::setComment(const QString& str) {
  Q_D(Declaration);
  d->m_comment = str.toUtf8();
}

void Declaration::removeUse( Use* use )
{
  ENSURE_CAN_WRITE
  Q_D(Declaration);
  use->d_func()->setDeclaration(0L);
  d->m_uses.removeAll(use);

  //DUChain::declarationChanged(this, DUChainObserver::Removal, DUChainObserver::Uses, use);
}

void Declaration::addUse( Use* use )
{
  ENSURE_CAN_WRITE
  Q_D(Declaration);
  use->d_func()->setDeclaration(this);
  d->m_uses.append(use);

  //DUChain::declarationChanged(this, DUChainObserver::Addition, DUChainObserver::Uses, use);
}

const QList< Use* > & Declaration::uses( ) const
{
  ENSURE_CAN_READ
  Q_D(const Declaration);
  return d->m_uses;
}

const Identifier& Declaration::identifier( ) const
{
  ENSURE_CAN_READ
  Q_D(const Declaration);
  return d->m_identifier;
}

void Declaration::setIdentifier(const Identifier& identifier)
{
  ENSURE_CAN_WRITE
  Q_D(Declaration);
  if( d->m_context )
    d->m_context->changingIdentifier( this, d->m_identifier, identifier );

  d->m_identifier = identifier;
  
  //DUChain::declarationChanged(this, DUChainObserver::Change, DUChainObserver::Identifier);
}

AbstractType::Ptr Declaration::abstractType( ) const
{
  ENSURE_CAN_READ
  Q_D(const Declaration);
  return d->m_type;
}

void Declaration::setAbstractType(AbstractType::Ptr type)
{
  ENSURE_CAN_WRITE
  Q_D(Declaration);
  //if (d->m_type)
    //DUChain::declarationChanged(this, DUChainObserver::Removal, DUChainObserver::DataType);

  d->m_type = type;

  //if (d->m_type)
    //DUChain::declarationChanged(this, DUChainObserver::Addition, DUChainObserver::DataType);
}

Declaration::Scope Declaration::scope( ) const
{
  ENSURE_CAN_READ
  Q_D(const Declaration);
  return d->m_scope;
}

QualifiedIdentifier Declaration::qualifiedIdentifier() const
{
  ENSURE_CAN_READ

  QualifiedIdentifier ret = context()->scopeIdentifier(true);
  ret.push(identifier());
  return ret;
}

QString Declaration::mangledIdentifier() const
{
  //GNU mangling specs from http://theory.uwinnipeg.ca/gnu/gcc/gxxint_15.html

  if (abstractType())
    return abstractType()->mangled();

  // Error...
  return qualifiedIdentifier().mangled();
}

DUContext * Declaration::context() const
{
  ENSURE_CAN_READ
  Q_D(const Declaration);
  return d->m_context;
}

void Declaration::setContext(DUContext* context, bool anonymous)
{
  ENSURE_CAN_WRITE
  Q_D(Declaration);
  if (d->m_context && context)
    Q_ASSERT(d->m_context->topContext() == context->topContext());

  if (d->m_context) {
    if( !d->m_anonymousInContext ) {
      d->m_context->d_func()->removeDeclaration(this);// if( )
        //DUChain::declarationChanged(this, DUChainObserver::Removal, DUChainObserver::Context, d->m_context);
    }
  }

  d->m_context = context;
  d->m_anonymousInContext = anonymous;

  if (d->m_context) {
    if(!d->m_anonymousInContext) {
      d->m_context->d_func()->addDeclaration(this);
      //DUChain::declarationChanged(this, DUChainObserver::Addition, DUChainObserver::Context, d->m_context);
    }
  }
}

const Declaration* Declaration::logicalDeclaration(const TopDUContext* topContext) const {
  ENSURE_CAN_READ
  if(isForwardDeclaration()) {
    const ForwardDeclaration* dec = toForwardDeclaration();
    Declaration* ret = dec->resolve(topContext);
    if(ret)
      return ret;
  }
  return this;
}

Declaration* Declaration::logicalDeclaration(const TopDUContext* topContext) {
  ENSURE_CAN_READ
  if(isForwardDeclaration()) {
    ForwardDeclaration* dec = toForwardDeclaration();
    Declaration* ret = dec->resolve(topContext);
    if(ret)
      return ret;
  }
  return this;
}

DUContext * Declaration::logicalInternalContext(const TopDUContext* topContext) const {
  ENSURE_CAN_READ
  
  if( definition() )
    return definition()->internalContext();

  if( d_func()->m_isTypeAlias ) {
    ///If this is a type-alias, return the internal context of the actual type.
    IdentifiedType* idType = dynamic_cast<IdentifiedType*>(abstractType().data());
    if( idType && idType->declaration() && idType->declaration() != this )
      return idType->declaration()->logicalInternalContext( topContext );
  }
  
  return ContextOwner::internalContext();
}

DUContext * Declaration::internalContext() const
{
  ENSURE_CAN_READ
  
  return ContextOwner::internalContext();
}

bool Declaration::operator ==(const Declaration & other) const
{
  ENSURE_CAN_READ

  return this == &other;
}

QString Declaration::toString() const
{
  return QString("%3 %4").arg(abstractType() ? abstractType()->toString() : QString("<notype>")).arg(identifier().toString());
}

// kate: indent-width 2;

bool Declaration::isDefinition() const
{
  ENSURE_CAN_READ
  Q_D(const Declaration);
  return d->m_isDefinition;
}

void Declaration::setDeclarationIsDefinition(bool dd)
{
  ENSURE_CAN_WRITE
  Q_D(Declaration);
  d->m_isDefinition = dd;
  if (d->m_isDefinition && definition()) {
    setDefinition(0);
  }
}

///@todo see whether it would be useful to create an own TypeAliasDeclaration sub-class for this
bool Declaration::isTypeAlias() const {
  Q_D(const Declaration);
  return d->m_isTypeAlias;
}

void Declaration::setIsTypeAlias(bool isTypeAlias) {
  Q_D(Declaration);
  d->m_isTypeAlias = isTypeAlias;
}

Definition* Declaration::definition() const
{
  ENSURE_CAN_READ
  Q_D(const Declaration);
  return d->m_definition;
}

void Declaration::setDefinition(Definition* definition)
{
  ENSURE_CAN_WRITE
  Q_D(Declaration);
  if (d->m_definition) {
    d->m_definition->d_func()->setDeclaration(0);

    //DUChain::declarationChanged(this, DUChainObserver::Removal, DUChainObserver::DefinitionRelationship, d->m_definition);
  }

  d->m_definition = definition;

  if (d->m_definition) {
    d->m_definition->d_func()->setDeclaration(this);
    d->m_isDefinition = false;

    //DUChain::declarationChanged(this, DUChainObserver::Addition, DUChainObserver::DefinitionRelationship, d->m_definition);
  }
}

bool Declaration::inSymbolTable() const
{
  Q_D(const Declaration);
  return d->m_inSymbolTable;
}

void Declaration::setInSymbolTable(bool inSymbolTable)
{
  Q_D(Declaration);
  d->m_inSymbolTable = inSymbolTable;
}

ForwardDeclaration* Declaration::toForwardDeclaration()
{
  return static_cast<ForwardDeclaration*>(this);
}

const ForwardDeclaration* Declaration::toForwardDeclaration() const
{
  return static_cast<const ForwardDeclaration*>(this);
}

TopDUContext * Declaration::topContext() const
{
  Q_D(const Declaration);
  if (d->m_context)
      return d->m_context->topContext();

    return 0;
}

Declaration* Declaration::clone() const  {
  return new Declaration(*this);
}

bool Declaration::isForwardDeclaration() const
{
  return false;
}


}

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on


