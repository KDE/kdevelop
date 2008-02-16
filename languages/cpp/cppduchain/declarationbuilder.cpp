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
#include <namespacealiasdeclaration.h>

#include "cppeditorintegrator.h"
#include "name_compiler.h"
#include "classfunctiondeclaration.h"
#include "functiondeclaration.h"
#include "templateparameterdeclaration.h"
#include "type_compiler.h"
#include "tokens.h"
#include "parsesession.h"
#include "cpptypes.h"
#include "cppduchain.h"

using namespace KTextEditor;
using namespace KDevelop;
using namespace Cpp;

void copyCppClass( const CppClassType* from, CppClassType* to )
{
  to->clear();
  to->setClassType(from->classType());
  to->setDeclaration(from->declaration());
  to->setCV(from->cv());
  
  foreach( const CppClassType::BaseClassInstance& base, from->baseClasses() )
    to->addBaseClass(base);
  
  foreach( const AbstractType::Ptr& element, from->elements() )
    to->addElement(element);

  to->close();
}

///Returns the context assigned to the given declaration that contains the template-parameters, if available. Else zero.
DUContext* getTemplateContext(Declaration* decl) {
  DUContext* internal = decl->internalContext();
  if( !internal )
    return 0;
  foreach( DUContextPointer ctx, internal->importedParentContexts() ) {
    if( ctx )
      if( ctx->type() == DUContext::Template )
        return ctx.data();
  }
  return 0;
}

DeclarationBuilder::DeclarationBuilder (ParseSession* session)
  : DeclarationBuilderBase(session), m_inTypedef(false)
{
}

DeclarationBuilder::DeclarationBuilder (CppEditorIntegrator* editor)
  : DeclarationBuilderBase(editor), m_inTypedef(false)
{
}

TopDUContext* DeclarationBuilder::buildDeclarations(const Cpp::EnvironmentFilePointer& file, AST *node, IncludeFileList* includes, const TopDUContextPointer& updateContext, bool removeOldImports)
{
  TopDUContext* top = buildContexts(file, node, includes, updateContext, removeOldImports);

  Q_ASSERT(m_accessPolicyStack.isEmpty());
  Q_ASSERT(m_functionDefinedStack.isEmpty());

  return top;
}

DUContext* DeclarationBuilder::buildSubDeclarations(const HashedString& url, AST *node, KDevelop::DUContext* parent) {
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

    if( ast->type_parameter && ast->type_parameter->type_id ) {
      //Extract default type-parameter
      QualifiedIdentifier defaultParam;
      
      QString str;
      ///Only record the strings, because these expressions may depend on template-parameters and thus must be evaluated later
      str += stringFromSessionTokens( m_editor->parseSession(), ast->type_parameter->type_id->start_token, ast->type_parameter->type_id->end_token );

      defaultParam = QualifiedIdentifier(str);

      decl->setDefaultParameter(defaultParam);
    }
    
    if( ast->parameter_declaration ) {
      //Extract default parameters(not tested)
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
    closeDeclaration();
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
  //need to make backup because we may temporarily change it
  ParameterDeclarationClauseAST* parameter_declaration_clause_backup = node->parameter_declaration_clause;
  
  ///@todo this should be solved more elegantly within parser and AST
  if (node->parameter_declaration_clause) {
    //Check if all parameter declarations are valid. If not, this is a misunderstood variable declaration.
    if(!checkParameterDeclarationClause(node->parameter_declaration_clause))
      node->parameter_declaration_clause = 0;
  }
  if (node->parameter_declaration_clause) {
    openDeclaration(node->id, node, true);

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
        DUChainWriteLocker lock(DUChain::lock());
        if (id.count() > 1 ||
            (m_inFunctionDefinition && (currentContext()->type() == DUContext::Namespace || currentContext()->type() == DUContext::Global))) {
          SimpleCursor pos = currentDeclaration()->range().start;//m_editor->findPosition(m_functionDefinedStack.top(), KDevelop::EditorIntegrator::FrontEdge);

          //kDebug(9007) << "Searching for declaration of" << id;
          // TODO: potentially excessive locking
          
          QList<Declaration*> declarations = currentContext()->findDeclarations(id, pos, AbstractType::Ptr(), 0, DUContext::OnlyFunctions);

          CppFunctionType::Ptr currentFunction = CppFunctionType::Ptr(dynamic_cast<CppFunctionType*>(lastType().data()));
          int functionArgumentCount = 0;
          if(currentFunction)
            functionArgumentCount = currentFunction->arguments().count();
          
          for( int cycle = 0; cycle < 3; cycle++ ) {
            bool found = false;
            ///We do 2 cycles: In the first cycle, we want an exact match. In the second, we accept approximate matches.
            foreach (Declaration* dec, declarations) {
              if (dec->isForwardDeclaration())
                continue;
              if(dec == currentDeclaration() || dec->isDefinition())
                continue;
              //Compare signatures of function-declarations:
              if(dec->abstractType() == lastType()
                 || (dec->abstractType() && lastType() && dec->abstractType()->equals(lastType().data())))
              {
                //The declaration-type matches this definition, good.
              }else{
                if(cycle == 0) {
                  //First cycle, only accept exact matches
                  continue;
                }else if(cycle == 1){
                  //Second cycle, match by argument-count
                  CppFunctionType::Ptr matchFunction = dec->type<CppFunctionType>();
                  if(currentFunction && matchFunction && currentFunction->arguments().count() == functionArgumentCount ) {
                    //We have a match
                  }else{
                    continue;
                  }
                }else if(cycle == 2){
                  //Accept any match, so just continue
                }
                if(dec->definition() && wasEncountered(dec->definition()))
                  continue; //Do not steal declarations
              }

              Declaration* oldDec = currentDeclaration();
              abortDeclaration();
              Definition* def = new Definition(m_editor->currentUrl(), oldDec->range(), currentContext());
              def->setSmartRange(oldDec->takeRange());
              setEncountered(def);
              delete oldDec;
              dec->setDefinition(def);
              
              if( m_lastContext && !m_lastContext->owner() )
                def->setInternalContext(m_lastContext);
              m_lastContext = 0;

              found = true;
              break;
            }
            if(found) {
              node->parameter_declaration_clause = parameter_declaration_clause_backup;
              return;
            }
          }
          //We do not want unresolved definitions to hide declarations.
          //As declarations are named by Identifiers, not by QualifiedIdentifiers,
          //they would be named by only one part of their scope, and thus be wrong anyway.
          Declaration* oldDec = currentDeclaration();
          abortDeclaration();
          delete oldDec;
          kDebug(9007) << "No declaration found for definition " << id << ", discarding definition";

          node->parameter_declaration_clause = parameter_declaration_clause_backup;
          return;
        }
      }
    }
  }

  closeDeclaration();

  node->parameter_declaration_clause = parameter_declaration_clause_backup;
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
DeclarationType* DeclarationBuilder::specialDeclaration( KTextEditor::SmartRange* smartRange, const KDevelop::SimpleRange& range )
{
    if( KDevelop::DUContext* ctx = hasTemplateContext(m_importedParentContexts) ) {
      Cpp::SpecialTemplateDeclaration<DeclarationType>* ret = new Cpp::SpecialTemplateDeclaration<DeclarationType>(m_editor->currentUrl(), range, currentContext());
      ret->setSmartRange(smartRange);
      ret->setTemplateParameterContext(ctx);
      return ret;
    } else{
      DeclarationType* ret = new DeclarationType(m_editor->currentUrl(), range, currentContext());
      ret->setSmartRange(smartRange);
      return ret;
    }
}

template<class DeclarationType>
DeclarationType* DeclarationBuilder::specialDeclaration( KTextEditor::SmartRange* smartRange, const SimpleRange& range, int scope )
{
    if( KDevelop::DUContext* ctx = hasTemplateContext(m_importedParentContexts) ) {
      Cpp::SpecialTemplateDeclaration<DeclarationType>* ret = new Cpp::SpecialTemplateDeclaration<DeclarationType>(m_editor->currentUrl(), range, (KDevelop::Declaration::Scope)scope, currentContext());
      ret->setSmartRange(smartRange);
      ret->setTemplateParameterContext(ctx);
      return ret;
    } else{
      DeclarationType* ret = new DeclarationType(m_editor->currentUrl(), range, (KDevelop::Declaration::Scope)scope, currentContext());
      ret->setSmartRange(smartRange);
      return ret;
    }
}

Declaration* DeclarationBuilder::openDeclaration(NameAST* name, AST* rangeNode, bool isFunction, bool isForward, bool isDefinition, bool isNamespaceAlias, const Identifier& customName)
{
  DUChainWriteLocker lock(DUChain::lock());

  if( isFunction && !m_functionDefinedStack.isEmpty() )
        isDefinition |= (bool)m_functionDefinedStack.top();
  
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

  
  SimpleRange newRange = m_editor->findRange(name ? static_cast<AST*>(name) : rangeNode);
  
  if(newRange.start >= newRange.end)
    kWarning(9007) << "Range collapsed";

  QualifiedIdentifier id;

  if (name) {
    TypeSpecifierAST* typeSpecifier = 0; //Additional type-specifier for example the return-type of a cast operator
    id = identifierForName(name, &typeSpecifier);
    if( typeSpecifier && id == QualifiedIdentifier("operator{...cast...}") ) {
      if( typeSpecifier->kind == AST::Kind_SimpleTypeSpecifier )
        visitSimpleTypeSpecifier( static_cast<SimpleTypeSpecifierAST*>( typeSpecifier ) );
    }
  } else {
    id = QualifiedIdentifier(customName);
  }

  Identifier lastId;
  if( !id.isEmpty() )
    lastId = id.last();

  Declaration* declaration = 0;

  if (recompiling()) {
    // Seek a matching declaration
    QMutexLocker lock(m_editor->smart() ? m_editor->smart()->smartMutex() : 0);

    // Translate cursor to take into account any changes the user may have made since the text was retrieved
    SimpleRange translated = newRange;
    if (m_editor->smart())
      translated = SimpleRange(m_editor->smart()->translateFromRevision(translated.textRange()));

    foreach( Declaration* dec, currentContext()->allLocalDeclarations(lastId) ) {

      if( wasEncountered(dec) )
        continue;

      //This works because dec->textRange() is taken from a smart-range. This means that now both ranges are translated to the current document-revision.
      if (dec->range() == translated &&
          dec->scope() == scope &&
          ((id.isEmpty() && dec->identifier().toString().isEmpty()) || (!id.isEmpty() && lastId == dec->identifier())) &&
           dec->isDefinition() == isDefinition &&
          dec->isTypeAlias() == m_inTypedef &&
          ( ((!hasTemplateContext(m_importedParentContexts) && !dynamic_cast<TemplateDeclaration*>(dec)) ||
             hasTemplateContext(m_importedParentContexts) && dynamic_cast<TemplateDeclaration*>(dec) ) )
         )
      {
        if(isNamespaceAlias && !dynamic_cast<NamespaceAliasDeclaration*>(dec)) {
          continue;
        } else if (isForward && !dynamic_cast<ForwardDeclaration*>(dec)) {
          continue;
        } else if (isFunction) {
          if (scope == Declaration::ClassScope) {
            if (!dynamic_cast<ClassFunctionDeclaration*>(dec))
              continue;
          } else if (!dynamic_cast<AbstractFunctionDeclaration*>(dec)) {
            continue;
          }

        } else if (scope == Declaration::ClassScope) {
          if (!dynamic_cast<ClassMemberDeclaration*>(dec))
            continue;
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
/*    if( recompiling() )
      kDebug(9007) << "creating new declaration while recompiling: " << lastId << "(" << newRange << ")";*/
    SmartRange* prior = m_editor->currentRange();
    SmartRange* range = m_editor->createRange(newRange.textRange());
    
    m_editor->exitCurrentRange();
  //Q_ASSERT(range->start() != range->end());
    
    Q_ASSERT(m_editor->currentRange() == prior);

    if( isNamespaceAlias ) {
      declaration = new NamespaceAliasDeclaration(m_editor->currentUrl(), newRange, scope, currentContext());
      declaration->setSmartRange(range);
      declaration->setIdentifier(customName);
    } else if (isForward) {
      declaration = specialDeclaration<ForwardDeclaration>(range, newRange, scope);

    } else if (isFunction) {
      if (scope == Declaration::ClassScope) {
        declaration = specialDeclaration<ClassFunctionDeclaration>( range, newRange );
      } else {
        declaration = specialDeclaration<FunctionDeclaration>(range, newRange, scope );
      }
    } else if (scope == Declaration::ClassScope) {
        declaration = specialDeclaration<ClassMemberDeclaration>( range, newRange );
    } else if( currentContext()->type() == DUContext::Template ) {
      //This is a template-parameter.
      declaration = new TemplateParameterDeclaration( m_editor->currentUrl(), newRange, currentContext() );
      declaration->setSmartRange(range);
    } else {
      declaration = specialDeclaration<Declaration>( range, newRange, scope );
    }

    if (!isNamespaceAlias) {
      // FIXME this can happen if we're defining a staticly declared variable
      //Q_ASSERT(m_nameCompiler->identifier().count() == 1);
/*      if(id.isEmpty())
        kWarning() << "empty id";*/
      declaration->setIdentifier(id.last());
    }
    
    declaration->setDeclarationIsDefinition(isDefinition);

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

  if( m_inTypedef )
    declaration->setIsTypeAlias(true);

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

void DeclarationBuilder::eventuallyAssignInternalContext()
{
  if (m_lastContext) {
    DUChainWriteLocker lock(DUChain::lock());

    if( dynamic_cast<ClassFunctionDeclaration*>(currentDeclaration()) )
      Q_ASSERT( !static_cast<ClassFunctionDeclaration*>(currentDeclaration())->isConstructor() || currentDeclaration()->context()->type() == DUContext::Class );
    
    if(m_lastContext && (m_lastContext->type() == DUContext::Class || m_lastContext->type() == DUContext::Other || m_lastContext->type() == DUContext::Function || m_lastContext->type() == DUContext::Template ) )
    {
      if( !m_lastContext->owner() || (!wasEncountered(m_lastContext->owner()->asDeclaration()) && !wasEncountered(m_lastContext->owner()->asDefinition()) ) ) { //if the context is already internalContext of another declaration, leave it alone
        currentDeclaration()->setInternalContext(m_lastContext);
        
        if( currentDeclaration()->range().start >= currentDeclaration()->range().end )
          kDebug(9007) << "Warning: Range was invalidated";
        
        m_lastContext = 0;
      }
    }
  }
}

void DeclarationBuilder::closeDeclaration()
{
  if (lastType()) {
    DUChainWriteLocker lock(DUChain::lock());

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

  eventuallyAssignInternalContext();

  //kDebug(9007) << "Mangled declaration:" << currentDeclaration()->mangledIdentifier();

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

void DeclarationBuilder::visitEnumSpecifier(EnumSpecifierAST* node)
{
  openDefinition(node->name, node);

  DeclarationBuilderBase::visitEnumSpecifier(node);

  closeDeclaration();
}

void DeclarationBuilder::visitEnumerator(EnumeratorAST* node)
{
  Identifier id(m_editor->parseSession()->token_stream->token(node->id).symbol());
  DeclarationBuilder::openDeclaration(0, node, false, false, true, false, id);

  DeclarationBuilderBase::visitEnumerator(node);

  closeDeclaration();
}

void DeclarationBuilder::visitClassSpecifier(ClassSpecifierAST *node)
{
  bool m_wasInTypedef = m_inTypedef;
  m_inTypedef = false;

  /**Open helper contexts around the class, so the qualified identifier matches.
   * Example: "class MyClass::RealClass{}"
   * Will create one helper-context named "MyClass" around RealClass
   * */

  QualifiedIdentifier id;
  if( node->name ) {
    id = identifierForName(node->name);
    ///@todo Make decision: Would it be better to allow giving declarations qualified identifiers? Then we wouldn't need to do this.
    openPrefixContext(node, id);
  }
  
  openDefinition(node->name, node);

  int kind = m_editor->parseSession()->token_stream->kind(node->class_key);
  if (kind == Token_struct || kind == Token_union)
    m_accessPolicyStack.push(Declaration::Public);
  else
    m_accessPolicyStack.push(Declaration::Private);

  DeclarationBuilderBase::visitClassSpecifier(node);

  eventuallyAssignInternalContext();
  
  if( node->name ) {
    ///Copy template default-parameters from the forward-declaration to the real declaration if possible
    DUChainWriteLocker lock(DUChain::lock());

    SimpleCursor pos = m_editor->findPosition(node->start_token, KDevelop::EditorIntegrator::FrontEdge);

    QList<Declaration*> declarations = Cpp::findDeclarationsSameLevel(currentContext(), id, pos);

    AbstractType::Ptr newLastType;
    
    foreach( Declaration* decl, declarations ) {
      if( decl->abstractType()) {
        ForwardDeclaration* forward =  dynamic_cast<ForwardDeclaration*>(decl);
        if( forward ) {
          {
            KDevelop::DUContext* forwardTemplateContext = forward->internalContext();
            if( forwardTemplateContext && forwardTemplateContext->type() == DUContext::Template ) {
              
              KDevelop::DUContext* currentTemplateContext = getTemplateContext(currentDeclaration());
              if( (bool)forwardTemplateContext != (bool)currentTemplateContext ) {
                kDebug(9007) << "Template-contexts of forward- and real declaration do not match: " << currentTemplateContext << getTemplateContext(currentDeclaration()) << currentDeclaration()->internalContext() << forwardTemplateContext << currentDeclaration()->internalContext()->importedParentContexts().count();
              } else if( forwardTemplateContext && currentTemplateContext ) {
                if( forwardTemplateContext->localDeclarations().count() != currentTemplateContext->localDeclarations().count() ) {
                } else {
                  
                  const QList<Declaration*>& forwardList = forwardTemplateContext->localDeclarations();
                  const QList<Declaration*>& realList = currentTemplateContext->localDeclarations();
                  
                  QList<Declaration*>::const_iterator forwardIt = forwardList.begin();
                  QList<Declaration*>::const_iterator realIt = realList.begin();
                    
                  for( ; forwardIt != forwardList.end(); ++forwardIt, ++realIt ) {
                    TemplateParameterDeclaration* forwardParamDecl = dynamic_cast<TemplateParameterDeclaration*>(*forwardIt);
                    TemplateParameterDeclaration* realParamDecl = dynamic_cast<TemplateParameterDeclaration*>(*realIt);
                    if( forwardParamDecl && realParamDecl ) {
                      if( !forwardParamDecl->defaultParameter().isEmpty() )
                        realParamDecl->setDefaultParameter(forwardParamDecl->defaultParameter());
                    }
                  }
                }
              }
            }
          }

          //Update instantiations in case of template forward-declarations
//           SpecialTemplateDeclaration<ForwardDeclaration>* templateForward = dynamic_cast<SpecialTemplateDeclaration<ForwardDeclaration>* > (decl);
//           SpecialTemplateDeclaration<Declaration>* currentTemplate = dynamic_cast<SpecialTemplateDeclaration<Declaration>* >  (currentDeclaration());
// 
//           if( templateForward && currentTemplate )
//           {
//             //Change the types of all the forward-template instantiations
//             TemplateDeclaration::InstantiationsHash instantiations = templateForward->instantiations();
// 
//             for( TemplateDeclaration::InstantiationsHash::iterator it = instantiations.begin(); it != instantiations.end(); ++it )
//             {
//               Declaration* realInstance = currentTemplate->instantiate(it.key().args, ImportTrace());
//               Declaration* forwardInstance = dynamic_cast<Declaration*>(*it);
//               //Now change the type of forwardInstance so it matches the type of realInstance
//               CppClassType::Ptr realClass = realInstance->type<CppClassType>();
//               CppClassType::Ptr forwardClass = forwardInstance->type<CppClassType>();
// 
//               if( realClass && forwardClass ) {
//                 //Copy the class from real into the forward-declaration's instance
//                 copyCppClass(realClass.data(), forwardClass.data());
//               } else {
//                 kDebug(9007) << "Bad types involved in formward-declaration";
//               }
//             }
//           }//templateForward && currentTemplate
        }
      }
    }//foreach

    if( newLastType )
      setLastType(newLastType);
  }//node-name
  
  closeDeclaration();

  if(node->name)
    closePrefixContext(id);

  m_accessPolicyStack.pop();
  m_inTypedef = m_wasInTypedef;
}

QualifiedIdentifier DeclarationBuilder::resolveNamespaceIdentifier(const QualifiedIdentifier& identifier, const SimpleCursor& position)
{
  QList<DUContext*> contexts = currentContext()->findContexts(DUContext::Namespace, identifier, position);
  if( contexts.isEmpty() ) {
    //Failed to resolve namespace
    kDebug(9007) << "Failed to resolve namespace \"" << identifier << "\"";
    QualifiedIdentifier ret = identifier;
    ret.setExplicitlyGlobal(true);
    Q_ASSERT(ret.count());
    return ret;
  } else {
    QualifiedIdentifier ret = contexts.first()->scopeIdentifier(true);
    Q_ASSERT(ret.count());
    ret.setExplicitlyGlobal(true);
    return ret;
  }
}


void DeclarationBuilder::visitUsingDirective(UsingDirectiveAST * node)
{
  DeclarationBuilderBase::visitUsingDirective(node);

  {
    DUChainReadLocker lock(DUChain::lock());
    if( currentContext()->type() != DUContext::Namespace && currentContext()->type() != DUContext::Global ) {
      ///@todo report problem
      kDebug(9007) << "Namespace-import used in non-global scope";
      return;
    }
  }
  
  if( m_compilingContexts ) {
    openDeclaration(0, node, false, false, false, true, globalImportIdentifier);
    {
      DUChainWriteLocker lock(DUChain::lock());
      Q_ASSERT(dynamic_cast<NamespaceAliasDeclaration*>(currentDeclaration()));
      static_cast<NamespaceAliasDeclaration*>(currentDeclaration())->setImportIdentifier( resolveNamespaceIdentifier(identifierForName(node->name), currentDeclaration()->range().start) );
    }
    closeDeclaration();
  }
}

void DeclarationBuilder::visitNamespaceAliasDefinition(NamespaceAliasDefinitionAST* node)
{
  DeclarationBuilderBase::visitNamespaceAliasDefinition(node);

  {
    DUChainReadLocker lock(DUChain::lock());
    if( currentContext()->type() != DUContext::Namespace && currentContext()->type() != DUContext::Global ) {
      ///@todo report problem
      kDebug(9007) << "Namespace-alias used in non-global scope";
    }
  }
  
  if( m_compilingContexts ) {
    openDeclaration(0, node, false, false, false, true, Identifier(m_editor->parseSession()->token_stream->token(node->namespace_name).symbol()));
    {
      DUChainWriteLocker lock(DUChain::lock());
      Q_ASSERT(dynamic_cast<NamespaceAliasDeclaration*>(currentDeclaration()));
      static_cast<NamespaceAliasDeclaration*>(currentDeclaration())->setImportIdentifier( resolveNamespaceIdentifier(identifierForName(node->alias_name), currentDeclaration()->range().start) );
    }
    closeDeclaration();
  }
}

void DeclarationBuilder::visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST* node)
{
  int kind = m_editor->parseSession()->token_stream->kind(node->type);
  
  if( kind == Token_typename ) {
    //typename is completely handled by the type-builder
    DeclarationBuilderBase::visitElaboratedTypeSpecifier(node);
    return;
  }
  
  //For now completely ignore friend-class specifiers, because those currently are wrongly parsed as forward-declarations.
  if( !m_storageSpecifiers.isEmpty() && (m_storageSpecifiers.top() & ClassMemberDeclaration::FriendSpecifier) )
    return;
  
  bool openedDeclaration = false;

  if (node->name) {
    QualifiedIdentifier id = identifierForName(node->name);

    bool forwardDeclarationGlobal = false;

    if(m_declarationHasInitDeclarators) {
      /**This is an elaborated type-specifier
       *
       * See iso c++ draft 3.3.4 for details.
       * Said shortly it means:
       * - Search for an existing declaration of the type. If it is found,
       *   it will be used, and we don't need to create a declaration.
       * - If it is not found, create a forward-declaration in the global/namespace scope.
       * - @todo While searching for the existing declarations, non-fitting overloaded names should be ignored.
       * */

      ///@todo think how this interacts with re-using duchains. In some cases a forward-declaration should still be created.
      QList<Declaration*> declarations;
      SimpleCursor pos = m_editor->findPosition(node->start_token, KDevelop::EditorIntegrator::FrontEdge);

      {
        DUChainReadLocker lock(DUChain::lock());

        declarations = currentContext()->findDeclarations( id, pos);

        if(declarations.isEmpty()) {
          //We haven't found a declaration, so insert a forward-declaration in the global scope
          forwardDeclarationGlobal = true;
        }else{
          //We have found a declaration. Do not create a new one, instead use the declarations type.
          if(declarations.first()->abstractType()) {
            //This belongs into the type-builder, but it's much easier to do here, since we already have all the information
            ///@todo See above, only search for fitting declarations(of structure/enum/class/union type)
            injectType(AbstractType::Ptr(declarations.first()->abstractType().data()), node);
            return;
          }else{
            kDebug(9007) << "Error: Bad declaration";
          }
        }
      }
    }
    
    // Create forward declaration
    switch (kind) {
      case Token_class:
      case Token_struct:
      case Token_union:
      case Token_enum:
      
        if(forwardDeclarationGlobal) {
          //Open the global context, so it is currentContext() and we can insert the forward-declaration there

          DUContext* globalCtx;
          {
            DUChainReadLocker lock(DUChain::lock());
            globalCtx = currentContext(); 
            while(globalCtx && globalCtx->type() != DUContext::Global && globalCtx->type() != DUContext::Namespace)
              globalCtx = globalCtx->parentContext();
            Q_ASSERT(globalCtx);
          }
          openContext(globalCtx);
        }
        
        openForwardDeclaration(node->name, node);

        if(forwardDeclarationGlobal)
          closeContext();

        openedDeclaration = true;
        break;
    }
  }

  DeclarationBuilderBase::visitElaboratedTypeSpecifier(node);

  if (openedDeclaration) {
/*    DUChainWriteLocker lock(DUChain::lock());
    //Resolve forward-declarations that are declared after the real type was already declared
    Q_ASSERT(dynamic_cast<ForwardDeclaration*>(currentDeclaration()));
    IdentifiedType* idType = dynamic_cast<IdentifiedType*>(lastType().data());
    if(idType && idType->declaration())
      static_cast<ForwardDeclaration*>(currentDeclaration())->setResolved(idType->declaration());*/
    closeDeclaration();
  }
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
  AbstractFunctionDeclaration* function = currentDeclaration<AbstractFunctionDeclaration>();
  
  if( function ) {
    if( node->expression ) {
      //Fill default-parameters and parameter-names
      QString defaultParam;
      for( size_t token = node->expression->start_token; token != node->expression->end_token; ++token )
        defaultParam += m_editor->tokenToString(token);


      function->addDefaultParameter(defaultParam);
    }
    if( node->declarator && node->declarator->id) {
      QString paramName;
      for( size_t token = node->declarator->id->start_token; token != node->declarator->id->end_token; ++token )
        paramName += m_editor->tokenToString(token);


      function->addParameterName(paramName);
    }else{
      function->addParameterName(QString());
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

bool DeclarationBuilder::checkParameterDeclarationClause(ParameterDeclarationClauseAST* clause)
{
    if(!clause || !clause->parameter_declarations)
      return true;
    AbstractType::Ptr oldLastType = lastType();
    
    const ListNode<ParameterDeclarationAST*> *start = clause->parameter_declarations->toFront();
    
    const ListNode<ParameterDeclarationAST*> *it = start;

    bool ret = false;
    
    do {
      ParameterDeclarationAST* ast = it->element;
      if(ast) {
        if(ast->expression || ast->declarator) {
          ret = true; //If one parameter has a default argument or a parameter name, it is surely a parameter
          break;
        }
        
        visit(ast->type_specifier);
        if( lastType() ) {
          //Break on the first valid thing found
          if( lastTypeWasInstance() ) {
            ret = false;
            break;
          }else{
            ret = true;
            break;
          }
        }
      }
      it = it->next;
    } while (it != start);

    setLastType(oldLastType);

    return ret;
}

void DeclarationBuilder::openContext(DUContext * newContext)
{
  DeclarationBuilderBase::openContext(newContext);
}

void DeclarationBuilder::closeContext()
{
  DeclarationBuilderBase::closeContext();
}
