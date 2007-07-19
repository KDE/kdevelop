/* This file is part of KDevelop
    Copyright (C) 2006-2007 Hamish Rodda <rodda@kde.org>

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
// kate: indent-width 2;

#include "declarationbuilder.h"

#include <ktexteditor/smartrange.h>
#include <ktexteditor/smartinterface.h>

#include <identifiedtype.h>
#include "cppeditorintegrator.h"
#include "name_compiler.h"
#include "classfunctiondeclaration.h"
#include "functiondeclaration.h"
#include "type_compiler.h"
#include "tokens.h"
#include "parsesession.h"
#include <definition.h>
#include <symboltable.h>
#include <forwarddeclaration.h>
#include <duchain.h>
#include <duchainlock.h>

using namespace KTextEditor;
using namespace KDevelop;

DeclarationBuilder::DeclarationBuilder (ParseSession* session)
  : DeclarationBuilderBase(session), m_inTypedef(false)
{
}

DeclarationBuilder::DeclarationBuilder (CppEditorIntegrator* editor)
  : DeclarationBuilderBase(editor), m_inTypedef(false)
{
}

TopDUContext* DeclarationBuilder::buildDeclarations(const Cpp::LexedFilePointer& file, AST *node, QList<DUContext*>* includes)
{
  TopDUContext* top = buildContexts(file, node, includes);

  Q_ASSERT(m_accessPolicyStack.isEmpty());
  Q_ASSERT(m_functionDefinedStack.isEmpty());

  return top;
}

DUContext* DeclarationBuilder::buildSubDeclarations(const KUrl& url, AST *node, KDevelop::DUContext* parent) {
  DUContext* top = buildSubContexts(url, node, parent);

  Q_ASSERT(m_accessPolicyStack.isEmpty());
  Q_ASSERT(m_functionDefinedStack.isEmpty());

  return top;
}


void DeclarationBuilder::visitFunctionDeclaration(FunctionDefinitionAST* node)
{
  parseStorageSpecifiers(node->storage_specifiers);
  parseFunctionSpecifiers(node->function_specifiers);

  m_functionDefinedStack.push(node->start_token);

  DeclarationBuilderBase::visitFunctionDeclaration(node);

  m_functionDefinedStack.pop();

  popSpecifiers();
}

void DeclarationBuilder::visitSimpleDeclaration(SimpleDeclarationAST* node)
{
  parseStorageSpecifiers(node->storage_specifiers);
  parseFunctionSpecifiers(node->function_specifiers);

  m_functionDefinedStack.push(0);

  DeclarationBuilderBase::visitSimpleDeclaration(node);

  m_functionDefinedStack.pop();

  popSpecifiers();
}

void DeclarationBuilder::visitDeclarator (DeclaratorAST* node)
{
  if (node->parameter_declaration_clause) {
    openDeclaration(node->id, node, true);

    /*if (!node->type_specifier) {
      // TODO detect identifiers not equal to classname
      if (currentDeclaration()->identifier().toString().startsWith('~'))
        static_cast<ClassFunctionDeclaration*>(currentDeclaration())->setDestructor(true);
      else
        static_cast<ClassFunctionDeclaration*>(currentDeclaration())->setConstructor(true);
    }*/

    applyFunctionSpecifiers();

  } else {
    openDefinition(node->id, node);
  }

  applyStorageSpecifiers();

  DeclarationBuilderBase::visitDeclarator(node);

  if (node->parameter_declaration_clause) {
    if (!m_functionDefinedStack.isEmpty() && m_functionDefinedStack.top() && node->id) {
      // TODO: make correct for incremental parsing; at the moment just skips if there is a definition
      Definition* def = 0;
      {
        DUChainReadLocker lock(DUChain::lock());
        def = currentDeclaration()->definition();
      }

      if (!def) {
        QualifiedIdentifier id = identifierForName(node->id);
        if (id.count() > 1) {
          KTextEditor::Cursor pos = m_editor->findPosition(m_functionDefinedStack.top(), KDevelop::EditorIntegrator::FrontEdge);

          //kDebug() << k_funcinfo << "Searching for declaration of " << id << endl;

          // TODO: potentially excessive locking
          DUChainWriteLocker lock(DUChain::lock());
          QList<Declaration*> declarations = currentContext()->findDeclarations(id, pos, lastType());
          foreach (Declaration* dec, declarations) {
            if (dec->isForwardDeclaration())
              continue;

            Declaration* oldDec = currentDeclaration();
            abortDeclaration();
            Definition* def = new Definition(oldDec->takeRange(), currentContext());
            delete oldDec;
            dec->setDefinition(def);

            // Resolve forward declarations
            foreach (Declaration* forward, declarations) {
              if (forward->isForwardDeclaration())
                forward->toForwardDeclaration()->setResolved(dec);
            }
            return;
          }
        }
      }
    }
  }

  closeDeclaration();
}

ForwardDeclaration * DeclarationBuilder::openForwardDeclaration(NameAST * name, AST * range)
{
  return static_cast<ForwardDeclaration*>(openDeclaration(name, range, false, true));
}

Declaration* DeclarationBuilder::openDefinition(NameAST* name, AST* rangeNode, bool isFunction)
{
  return openDeclaration(name, rangeNode, isFunction, false, true);
}

Declaration* DeclarationBuilder::openDeclaration(NameAST* name, AST* rangeNode, bool isFunction, bool isForward, bool isDefinition)
{
  DUChainWriteLocker lock(DUChain::lock());

  Declaration::Scope scope = Declaration::GlobalScope;
  switch (currentContext()->type()) {
    case DUContext::Namespace:
      scope = Declaration::NamespaceScope;
      break;
    case DUContext::Class:
      scope = Declaration::ClassScope;
      break;
    case DUContext::Function:
      scope = Declaration::LocalScope;
      break;
    default:
      break;
  }

  Range newRange = m_editor->findRange(name ? static_cast<AST*>(name) : rangeNode);

  QualifiedIdentifier id;

  if (name) {
    TypeSpecifierAST* typeSpecifier = 0; //Additional type-specifier for example the return-type of a cast operator
    id = identifierForName(name, &typeSpecifier);
    if( typeSpecifier && id == QualifiedIdentifier("operator<...cast...>") ) {
      if( typeSpecifier->kind == AST::Kind_SimpleTypeSpecifier )
        visitSimpleTypeSpecifier( static_cast<SimpleTypeSpecifierAST*>( typeSpecifier ) );
    }
  }

  Declaration* declaration = 0;

  if (recompiling()) {
    // Seek a matching declaration
    QMutexLocker lock(m_editor->smart() ? m_editor->smart()->smartMutex() : 0);

    // Translate cursor to take into account any changes the user may have made since the text was retrieved
    Range translated = newRange;
    if (m_editor->smart())
      translated = m_editor->smart()->translateFromRevision(translated);

    for (; nextDeclaration() < currentContext()->localDeclarations().count(); ++nextDeclaration()) {
      Declaration* dec = currentContext()->localDeclarations().at(nextDeclaration());

      if (dec->textRange().start() > translated.end() && dec->smartRange()) //Only break the loop if it is a smartrange, because if it is not one, it could not adapt to changes and hide valid smart-ranges that are in the order behind.
        break;
      //This works because dec->textRange() is taken from a smart-range. This means that now both ranges are translated to the current document-revision.
      if (dec->textRange() == translated &&
          dec->scope() == scope &&
          (id.isEmpty() && dec->identifier().toString().isEmpty()) || (!id.isEmpty() && id.last() == dec->identifier()) &&
          dec->isDefinition() == isDefinition && dec->isTypeAlias() == m_inTypedef)
      {
        if (isForward) {
          if (!dynamic_cast<ForwardDeclaration*>(dec))
            break;

        } else if (isFunction) {
          if (scope == Declaration::ClassScope) {
            if (!dynamic_cast<ClassFunctionDeclaration*>(dec))
              break;
          } else if (!dynamic_cast<AbstractFunctionDeclaration*>(dec)) {
            break;
          }

        } else if (scope == Declaration::ClassScope) {
          if (!dynamic_cast<ClassMemberDeclaration*>(dec))
            break;
        }

        // Match
        declaration = dec;

        //If the declaration does not have a smart-range, upgrade it if possible
        /*if( m_editor->smart() && !declaration->smartRange() ) {
          declaration->setTextRange( m_editor->createRange( newRange ) );
        }*/

        // Update access policy if needed
        if (currentContext()->type() == DUContext::Class) {
          ClassMemberDeclaration* classDeclaration = static_cast<ClassMemberDeclaration*>(declaration);
          if (classDeclaration->accessPolicy() != currentAccessPolicy()) {
            classDeclaration->setAccessPolicy(currentAccessPolicy());
          }
        }
        break;
      }
    }
  }

  if (!declaration) {
    Range* prior = m_editor->currentRange();
    Range* range = m_editor->createRange(newRange);
    m_editor->exitCurrentRange();
    Q_ASSERT(m_editor->currentRange() == prior);

    if (isForward) {
      declaration = new ForwardDeclaration(range, scope, currentContext());

    } else if (isFunction) {
      if (scope == Declaration::ClassScope)
        declaration = new ClassFunctionDeclaration(range, currentContext());
      else
        declaration = new FunctionDeclaration(range, scope, currentContext());

      if (!m_functionDefinedStack.isEmpty())
        declaration->setDeclarationIsDefinition(m_functionDefinedStack.top());

    } else if (scope == Declaration::ClassScope) {
      declaration = new ClassMemberDeclaration(range, currentContext());

    } else {
      declaration = new Declaration(range, scope, currentContext());
    }
    
    if( m_inTypedef )
      declaration->setIsTypeAlias(true);

    if (name) {
      // FIXME this can happen if we're defining a staticly declared variable
      //Q_ASSERT(m_nameCompiler->identifier().count() == 1);
      Q_ASSERT(!id.isEmpty());
      declaration->setIdentifier(id.last());
    }

    if (isDefinition)
      declaration->setDeclarationIsDefinition(true);

    if (currentContext()->type() == DUContext::Class)
      static_cast<ClassMemberDeclaration*>(declaration)->setAccessPolicy(currentAccessPolicy());

    switch (currentContext()->type()) {
      case DUContext::Global:
      case DUContext::Namespace:
      case DUContext::Class:
        SymbolTable::self()->addDeclaration(declaration);
        break;
      default:
        break;
    }
  }

  setEncountered(declaration);

  m_declarationStack.push(declaration);

  return declaration;
}

void DeclarationBuilder::classTypeOpened(AbstractType::Ptr type) {
  //We override this so we can get the class-declaration into a usable state(with filled type) earlier
    DUChainWriteLocker lock(DUChain::lock());

    IdentifiedType* idType = dynamic_cast<IdentifiedType*>(lastType().data());

    if( idType && idType->declaration() == 0 ) //When the given type has no declaration yet, assume we are declaring it now
        idType->setDeclaration( currentDeclaration() );

    currentDeclaration()->setType(type);
}

void DeclarationBuilder::closeDeclaration()
{
  if (lastType()) {
    DUChainWriteLocker lock(DUChain::lock());

    IdentifiedType* idType = dynamic_cast<IdentifiedType*>(lastType().data());

    if( idType && idType->declaration() == 0 ) //When the given type has no declaration yet, assume we are declaring it now
        idType->setDeclaration( currentDeclaration() );

    currentDeclaration()->setType(lastType());
  }

  //kDebug() << k_funcinfo << "Mangled declaration: " << currentDeclaration()->mangledIdentifier() << endl;

  m_declarationStack.pop();
}

void DeclarationBuilder::abortDeclaration()
{
  m_declarationStack.pop();
}

void DeclarationBuilder::visitTypedef(TypedefAST *def)
{
  m_inTypedef = true;
  DeclarationBuilderBase::visitTypedef(def);
  m_inTypedef = false;
}

void DeclarationBuilder::visitClassSpecifier(ClassSpecifierAST *node)
{
  openDefinition(node->name, node);

  int kind = m_editor->parseSession()->token_stream->kind(node->class_key);
  if (kind == Token_struct || kind == Token_union)
    m_accessPolicyStack.push(Declaration::Public);
  else
    m_accessPolicyStack.push(Declaration::Private);

  DeclarationBuilderBase::visitClassSpecifier(node);

  closeDeclaration();

  m_accessPolicyStack.pop();
}

void DeclarationBuilder::visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST* node)
{
  bool openedDeclaration = false;

  if (node->name) {
    QualifiedIdentifier id = identifierForName(node->name);
    KTextEditor::Cursor pos = m_editor->findPosition(node->start_token, KDevelop::EditorIntegrator::FrontEdge);

    Declaration* actual = 0;
    {
      DUChainReadLocker lock(DUChain::lock());
      QList<Declaration*> declarations = currentContext()->findDeclarations(id, pos);
      foreach (Declaration* declaration, declarations)
        if (!declaration->isForwardDeclaration()) {
          actual = declaration;
          break;
        }
    }

    if (!actual) {
      int kind = m_editor->parseSession()->token_stream->kind(node->type);
      // Create forward declaration
      switch (kind) {
        case Token_class:
        case Token_struct:
        case Token_union:
          openForwardDeclaration(node->name, node);
          openedDeclaration = true;
          break;
        case Token_enum:
        case Token_typename:
          // TODO what goes here...?
          break;
      }
    }
  }

  DeclarationBuilderBase::visitElaboratedTypeSpecifier(node);

  if (openedDeclaration)
    closeDeclaration();
}


void DeclarationBuilder::visitAccessSpecifier(AccessSpecifierAST* node)
{
  if (node->specs) {
    const ListNode<std::size_t> *it = node->specs->toFront();
    const ListNode<std::size_t> *end = it;
    do {
      int kind = m_editor->parseSession()->token_stream->kind(it->element);
      switch (kind) {
        case Token_signals:
        case Token_slots:
        case Token_k_dcop:
        case Token_k_dcop_signals:
          break;
        case Token_public:
          setAccessPolicy(Declaration::Public);
          break;
        case Token_protected:
          setAccessPolicy(Declaration::Protected);
          break;
        case Token_private:
          setAccessPolicy(Declaration::Private);
          break;
      }

      it = it->next;
    } while (it != end);
  }

  DeclarationBuilderBase::visitAccessSpecifier(node);
}

void DeclarationBuilder::parseStorageSpecifiers(const ListNode<std::size_t>* storage_specifiers)
{
  ClassMemberDeclaration::StorageSpecifiers specs = 0;

  if (storage_specifiers) {
    const ListNode<std::size_t> *it = storage_specifiers->toFront();
    const ListNode<std::size_t> *end = it;
    do {
      int kind = m_editor->parseSession()->token_stream->kind(it->element);
      switch (kind) {
        case Token_friend:
          specs |= ClassMemberDeclaration::FriendSpecifier;
          break;
        case Token_auto:
          specs |= ClassMemberDeclaration::AutoSpecifier;
          break;
        case Token_register:
          specs |= ClassMemberDeclaration::RegisterSpecifier;
          break;
        case Token_static:
          specs |= ClassMemberDeclaration::StaticSpecifier;
          break;
        case Token_extern:
          specs |= ClassMemberDeclaration::ExternSpecifier;
          break;
        case Token_mutable:
          specs |= ClassMemberDeclaration::MutableSpecifier;
          break;
      }

      it = it->next;
    } while (it != end);
  }

  m_storageSpecifiers.push(specs);
}

void DeclarationBuilder::parseFunctionSpecifiers(const ListNode<std::size_t>* function_specifiers)
{
  ClassFunctionDeclaration::FunctionSpecifiers specs = 0;

  if (function_specifiers) {
    const ListNode<std::size_t> *it = function_specifiers->toFront();
    const ListNode<std::size_t> *end = it;
    do {
      int kind = m_editor->parseSession()->token_stream->kind(it->element);
      switch (kind) {
        case Token_inline:
          specs |= ClassFunctionDeclaration::InlineSpecifier;
          break;
        case Token_virtual:
          specs |= ClassFunctionDeclaration::VirtualSpecifier;
          break;
        case Token_explicit:
          specs |= ClassFunctionDeclaration::ExplicitSpecifier;
          break;
      }

      it = it->next;
    } while (it != end);
  }

  m_functionSpecifiers.push(specs);
}

void DeclarationBuilder::visitParameterDeclaration(ParameterDeclarationAST* node) {
  DeclarationBuilderBase::visitParameterDeclaration(node);
  if( node->expression ) {
    //Fill default-parameters
    AbstractFunctionDeclaration* function = currentDeclaration<AbstractFunctionDeclaration>();
    if( function ) {
      QString param;
      for( size_t token = node->expression->start_token; token != node->expression->end_token; ++token )
        param += m_editor->tokenToString(token);

      function->addDefaultParameter(param);
    }
  }
}

void DeclarationBuilder::popSpecifiers()
{
  m_functionSpecifiers.pop();
  m_storageSpecifiers.pop();
}

void DeclarationBuilder::applyStorageSpecifiers()
{
  if (!m_storageSpecifiers.isEmpty() && m_storageSpecifiers.top() != 0)
    if (ClassMemberDeclaration* member = dynamic_cast<ClassMemberDeclaration*>(currentDeclaration())) {
      DUChainWriteLocker lock(DUChain::lock());

      member->setStorageSpecifiers(m_storageSpecifiers.top());
    }
}

void DeclarationBuilder::applyFunctionSpecifiers()
{
  if (!m_functionSpecifiers.isEmpty() && m_functionSpecifiers.top() != 0) {
    Q_ASSERT(dynamic_cast<ClassFunctionDeclaration*>(currentDeclaration()));
    ClassFunctionDeclaration* function = static_cast<ClassFunctionDeclaration*>(currentDeclaration());

    DUChainWriteLocker lock(DUChain::lock());

    function->setFunctionSpecifiers(m_functionSpecifiers.top());
  }
}

void DeclarationBuilder::openContext(DUContext * newContext)
{
  DeclarationBuilderBase::openContext(newContext);

  m_nextDeclarationStack.push(0);
}

void DeclarationBuilder::closeContext()
{
  DeclarationBuilderBase::closeContext();

  m_nextDeclarationStack.pop();
}
