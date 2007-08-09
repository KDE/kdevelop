/* This file is part of KDevelop
    Copyright 2006-2007 Hamish Rodda <rodda@kde.org>

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

#include <QByteArray>

#include "templatedeclaration.h"

#include <ktexteditor/smartrange.h>
#include <ktexteditor/smartinterface.h>

#include "parser/type_compiler.h"
#include "parser/commentformatter.h"

#include <definition.h>
#include <symboltable.h>
#include <forwarddeclaration.h>
#include <duchain.h>
#include <duchainlock.h>
#include <identifiedtype.h>

#include "cppeditorintegrator.h"
#include "name_compiler.h"
#include "classfunctiondeclaration.h"
#include "functiondeclaration.h"
#include "templateparameterdeclaration.h"
#include "type_compiler.h"
#include "tokens.h"
#include "parsesession.h"
#include "cpptypes.h"

using namespace KTextEditor;
using namespace KDevelop;
using namespace Cpp;

QString stringFromSessionTokens( ParseSession* session, int start_token, int end_token ) {
    int startPosition = session->token_stream->position(start_token);
    int endPosition = session->token_stream->position(end_token);
    return QString::fromUtf8( QByteArray(session->contents() + startPosition, endPosition - startPosition) ); ///@todo Exact encoding?
}

DeclarationBuilder::DeclarationBuilder (ParseSession* session)
  : DeclarationBuilderBase(session), m_inTypedef(false)
{
}

DeclarationBuilder::DeclarationBuilder (CppEditorIntegrator* editor)
  : DeclarationBuilderBase(editor), m_inTypedef(false)
{
}

TopDUContext* DeclarationBuilder::buildDeclarations(const Cpp::EnvironmentFilePointer& file, AST *node, QList<DUContext*>* includes)
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

void DeclarationBuilder::visitTemplateParameter(TemplateParameterAST * ast) {
  TypeBuilder::visitTemplateParameter(ast);

  if( ast->type_parameter && ast->type_parameter->name ) {
    ///@todo deal with all the other stuff the AST may contain
    Declaration* dec = openDeclaration(ast->type_parameter->name, ast);
    TemplateParameterDeclaration* decl = dynamic_cast<TemplateParameterDeclaration*>(dec);
    Q_ASSERT(decl);
    DUChainWriteLocker lock(DUChain::lock());
    decl->setAbstractType(lastType());
    if( decl->type<CppTemplateParameterType>() ) {
      decl->type<CppTemplateParameterType>()->setDeclaration(dec);
    } else {
      kDebug(9007) << "bad last type";
    }

    if( ast->parameter_declaration ) {
      QualifiedIdentifier defaultParam;
      if( ast->parameter_declaration->type_specifier ) {
        TypeCompiler tc(m_editor->parseSession());
        tc.run(ast->parameter_declaration->type_specifier);
        defaultParam = tc.identifier();
      } else {
        QString str;
        ///Only record the strings, because these expressions may depend on template-parameters and thus must be evaluated later
        if( ast->parameter_declaration->declarator )
          str += stringFromSessionTokens( m_editor->parseSession(), ast->parameter_declaration->start_token, ast->parameter_declaration->end_token );
        if( ast->parameter_declaration->expression )
          str += stringFromSessionTokens( m_editor->parseSession(), ast->parameter_declaration->expression->start_token, ast->parameter_declaration->expression->end_token );
        defaultParam = QualifiedIdentifier(str);
      }

      decl->setDefaultParameter(defaultParam);
    }
  } else {
    kDebug(9007) << "DeclarationBuilder::visitTemplateParameter: type-parameter is missing";
  }
}

void DeclarationBuilder::parseComments(const ListNode<size_t> *comments)
{
  m_lastComment = CommentFormatter::formatComment(comments, m_editor->parseSession());
}


void DeclarationBuilder::visitFunctionDeclaration(FunctionDefinitionAST* node)
{

  parseComments(node->comments);
  parseStorageSpecifiers(node->storage_specifiers);
  parseFunctionSpecifiers(node->function_specifiers);

  m_functionDefinedStack.push(node->start_token);

  DeclarationBuilderBase::visitFunctionDeclaration(node);

  m_functionDefinedStack.pop();

  popSpecifiers();
}

void DeclarationBuilder::visitSimpleDeclaration(SimpleDeclarationAST* node)
{
  parseComments(node->comments);
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

          //kDebug(9007) << k_funcinfo << "Searching for declaration of" << id;

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

KDevelop::DUContext* hasTemplateContext( const QList<KDevelop::DUContext*>& contexts ) {
  foreach( DUContext* context, contexts )
    if( context->type() == KDevelop::DUContext::Template )
      return context;
  return 0;
}

KDevelop::DUContext* hasTemplateContext( const QList<KDevelop::DUContextPointer>& contexts ) {
  foreach( DUContextPointer context, contexts )
    if( context.data() && context->type() == KDevelop::DUContext::Template )
      return context.data();
  return 0;
}

//Check whether the given context is a template-context by checking whether it imports a template-parameter context
KDevelop::DUContext* isTemplateContext( KDevelop::DUContext* context ) {
  return hasTemplateContext( context->importedParentContexts() );
}

template<class DeclarationType>
DeclarationType* DeclarationBuilder::specialDeclaration( KTextEditor::Range* range )
{
    if( KDevelop::DUContext* ctx = hasTemplateContext(m_importedParentContexts) ) {
      Cpp::SpecialTemplateDeclaration<DeclarationType>* ret = new Cpp::SpecialTemplateDeclaration<DeclarationType>(range, currentContext());
      ret->setTemplateParameterContext(ctx);
      return ret;
    } else
      return new DeclarationType(range, currentContext());
}

template<class DeclarationType>
DeclarationType* DeclarationBuilder::specialDeclaration( KTextEditor::Range* range, int scope )
{
    if( KDevelop::DUContext* ctx = hasTemplateContext(m_importedParentContexts) ) {
      Cpp::SpecialTemplateDeclaration<DeclarationType>* ret = new Cpp::SpecialTemplateDeclaration<DeclarationType>(range, (KDevelop::Declaration::Scope)scope, currentContext());
      ret->setTemplateParameterContext(ctx);
      return ret;
    } else
      return new DeclarationType(range, (KDevelop::Declaration::Scope)scope, currentContext());
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
    case DUContext::Template:
      scope = Declaration::LocalScope;
      break;
    default:
      break;
  }

  Range newRange = m_editor->findRange(name ? static_cast<AST*>(name) : rangeNode);
  Q_ASSERT(newRange.start() != newRange.end());

  QualifiedIdentifier id;

  if (name) {
    TypeSpecifierAST* typeSpecifier = 0; //Additional type-specifier for example the return-type of a cast operator
    id = identifierForName(name, &typeSpecifier);
    if( typeSpecifier && id == QualifiedIdentifier("operator{...cast...}") ) {
      if( typeSpecifier->kind == AST::Kind_SimpleTypeSpecifier )
        visitSimpleTypeSpecifier( static_cast<SimpleTypeSpecifierAST*>( typeSpecifier ) );
    }
  }

  Identifier lastId;
  if( !id.isEmpty() )
    lastId = id.last();

  Declaration* declaration = 0;

  if (recompiling()) {
    // Seek a matching declaration
    QMutexLocker lock(m_editor->smart() ? m_editor->smart()->smartMutex() : 0);

    // Translate cursor to take into account any changes the user may have made since the text was retrieved
    Range translated = newRange;
    if (m_editor->smart())
      translated = m_editor->smart()->translateFromRevision(translated);
    Q_ASSERT(translated.start() != translated.end());

    for (; nextDeclaration() < currentContext()->localDeclarations().count(); ++nextDeclaration()) {
      Declaration* dec = currentContext()->localDeclarations().at(nextDeclaration());

      if (dec->textRange().start() > translated.end() && dec->smartRange()) //Only break the loop if it is a smartrange, because if it is not one, it could not adapt to changes and hide valid smart-ranges that are in the order behind.
        break;
      //This works because dec->textRange() is taken from a smart-range. This means that now both ranges are translated to the current document-revision.
      if (dec->textRange() == translated &&
          dec->scope() == scope &&
          (id.isEmpty() && dec->identifier().toString().isEmpty()) || (!id.isEmpty() && lastId == dec->identifier()) &&
          dec->isDefinition() == isDefinition && dec->isTypeAlias() == m_inTypedef)
      {
        ///@todo differentiate template-parameter declarations and all the template-stuff
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
    if( recompiling() )
      kDebug(9007) << "creating new declaration while recompiling: " << lastId << "(" << newRange << ")" << endl;
    Range* prior = m_editor->currentRange();
    Range* range = m_editor->createRange(newRange);
    
    m_editor->exitCurrentRange();
  Q_ASSERT(range->start() != range->end());
    
    Q_ASSERT(m_editor->currentRange() == prior);

    if (isForward) {
      declaration = new ForwardDeclaration(range, scope, currentContext());

    } else if (isFunction) {
      if (scope == Declaration::ClassScope) {
        declaration = specialDeclaration<ClassFunctionDeclaration>( range );
      } else {
        declaration = specialDeclaration<FunctionDeclaration>(range, scope );
      }

      if (!m_functionDefinedStack.isEmpty())
        declaration->setDeclarationIsDefinition(m_functionDefinedStack.top());

    } else if (scope == Declaration::ClassScope) {
        declaration = specialDeclaration<ClassMemberDeclaration>(range );
    } else if( currentContext()->type() == DUContext::Template ) {
      //This is a template-parameter.
      declaration = new TemplateParameterDeclaration( range, currentContext() );
    } else {
      declaration = specialDeclaration<Declaration>(range, scope );
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

    if (currentContext()->type() == DUContext::Class) {
      if(dynamic_cast<ClassMemberDeclaration*>(declaration)) //It may also be a forward-declaration, not based on ClassMemberDeclaration!
        static_cast<ClassMemberDeclaration*>(declaration)->setAccessPolicy(currentAccessPolicy());
    }

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

  if( !lastId.templateIdentifiers().isEmpty() ) {
    TemplateDeclaration* templateDecl = dynamic_cast<TemplateDeclaration*>(declaration);
    if( declaration && templateDecl ) {
      ///This is a template-specialization. Find the class it is specialized from.
      lastId.clearTemplateIdentifiers();
      id.pop();
      id.push(lastId);

      ///@todo Make sure the searched class is in the same namespace
      QList<Declaration*> decls = currentContext()->findDeclarations(id, m_editor->findPosition(name->start_token, KDevelop::EditorIntegrator::FrontEdge) );

      if( !decls.isEmpty() )
      {
        if( decls.count() > 1 )
          kDebug(9007) << "Found multiple declarations of specialization-base" << id.toString() << "for" << declaration->toString();

        foreach( Declaration* decl, decls )
          if( TemplateDeclaration* baseTemplateDecl = dynamic_cast<TemplateDeclaration*>(decl) )
            templateDecl->setSpecializedFrom(baseTemplateDecl);

        if( !templateDecl->specializedFrom() )
          kDebug(9007) << "Could not find valid specialization-base" << id.toString() << "for" << declaration->toString();
      }
    } else {
      kDebug(9007) << "Specialization of non-template class" << declaration->toString();
    }

  }

  declaration->setComment(m_lastComment);
  m_lastComment = QString();

  setEncountered(declaration);

  m_declarationStack.push(declaration);

  return declaration;
}

void DeclarationBuilder::classTypeOpened(AbstractType::Ptr type) {
  //We override this so we can get the class-declaration into a usable state(with filled type) earlier
    DUChainWriteLocker lock(DUChain::lock());

    IdentifiedType* idType = dynamic_cast<IdentifiedType*>(type.data());

    if( idType && idType->declaration() == 0 ) //When the given type has no declaration yet, assume we are declaring it now
        idType->setDeclaration( currentDeclaration() );

    currentDeclaration()->setType(type);
}

void DeclarationBuilder::closeDeclaration()
{
  if (lastType() || m_lastContext) {
    DUChainWriteLocker lock(DUChain::lock());

    if(lastType()) {
      IdentifiedType* idType = dynamic_cast<IdentifiedType*>(lastType().data());
      DelayedType* delayed = dynamic_cast<DelayedType*>(lastType().data());

      //When the given type has no declaration yet, assume we are declaring it now.
      //If the type is a delayed type, it is a searched type, and not a declared one, so don't set the declaration then.
      if( idType && idType->declaration() == 0 && !delayed )
          idType->setDeclaration( currentDeclaration() );

      //If the type is not identified, it is an instance-declaration too, because those types have no type-declarations.
      if( (((!idType) || (idType && idType->declaration() != currentDeclaration())) && !currentDeclaration()->isTypeAlias() && !currentDeclaration()->isForwardDeclaration() ) )
        currentDeclaration()->setKind(Declaration::Instance);
      else
        currentDeclaration()->setKind(Declaration::Type);

      currentDeclaration()->setType(lastType());
    }
    if(m_lastContext && (m_lastContext->type() == DUContext::Class || m_lastContext->type() == DUContext::Other || m_lastContext->type() == DUContext::Function || m_lastContext->type() == DUContext::Template ) )
    {
      currentDeclaration()->setInternalContext(m_lastContext);
      
      if( currentDeclaration()->textRange().start() == currentDeclaration()->textRange().end() )
        kDebug() << "Warning: Range was invalidated" << endl;
      
      m_lastContext = 0;
    }
  }



  //kDebug(9007) << k_funcinfo << "Mangled declaration:" << currentDeclaration()->mangledIdentifier();

  m_declarationStack.pop();
}

void DeclarationBuilder::abortDeclaration()
{
  m_declarationStack.pop();
}

void DeclarationBuilder::visitTypedef(TypedefAST *def)
{
  parseComments(def->comments);
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
  AbstractFunctionDeclaration::FunctionSpecifiers specs = 0;

  if (function_specifiers) {
    const ListNode<std::size_t> *it = function_specifiers->toFront();
    const ListNode<std::size_t> *end = it;
    do {
      int kind = m_editor->parseSession()->token_stream->kind(it->element);
      switch (kind) {
        case Token_inline:
          specs |= AbstractFunctionDeclaration::InlineSpecifier;
          break;
        case Token_virtual:
          specs |= AbstractFunctionDeclaration::VirtualSpecifier;
          break;
        case Token_explicit:
          specs |= AbstractFunctionDeclaration::ExplicitSpecifier;
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
    AbstractFunctionDeclaration* function = dynamic_cast<AbstractFunctionDeclaration*>(currentDeclaration());
    Q_ASSERT(function);

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
