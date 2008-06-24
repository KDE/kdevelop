/* This file is part of KDevelop
    Copyright 2006-2007 Hamish Rodda <rodda@kde.org>
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

#include "declarationbuilder.h"

#include <QByteArray>
#include <typeinfo>

#include "templatedeclaration.h"

#include <ktexteditor/smartrange.h>
#include <ktexteditor/smartinterface.h>

#include "parser/type_compiler.h"
#include "parser/commentformatter.h"

#include <forwarddeclaration.h>
#include <duchain.h>
#include <duchainlock.h>
#include <identifiedtype.h>
#include <namespacealiasdeclaration.h>
#include <aliasdeclaration.h>
#include <util/pushvalue.h>

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

#include "cppdebughelper.h"

//#define DEBUG_UPDATE_MATCHING

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

bool DeclarationBuilder::changeWasSignificant() const
{
  ///@todo Also set m_changeWasSignificant if publically visible declarations were removed(needs interaction with abstractcontextbuilder)
  return m_changeWasSignificant;
}

DeclarationBuilder::DeclarationBuilder (ParseSession* session)
  : DeclarationBuilderBase(), m_inTypedef(false), m_changeWasSignificant(false)
{
  setEditor(new CppEditorIntegrator(session), true);
}

DeclarationBuilder::DeclarationBuilder (CppEditorIntegrator* editor)
  : DeclarationBuilderBase(), m_inTypedef(false), m_changeWasSignificant(false)
{
  setEditor(editor, false);
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
  DeclarationBuilderBase::visitTemplateParameter(ast);
  if( ast->type_parameter && ast->type_parameter->name ) {
    ///@todo deal with all the other stuff the AST may contain
    TemplateParameterDeclaration* decl = openDeclaration<TemplateParameterDeclaration>(ast->type_parameter->name, ast);
    
    DUChainWriteLocker lock(DUChain::lock());
    decl->setAbstractType(lastType());
    if( decl->type<CppTemplateParameterType>() ) {
      decl->type<CppTemplateParameterType>()->setDeclaration(decl);
    } else {
      kDebug(9007) << "bad last type";
    }

    if( ast->type_parameter && ast->type_parameter->type_id ) {
      //Extract default type-parameter
      QualifiedIdentifier defaultParam;

      QString str;
      ///Only record the strings, because these expressions may depend on template-parameters and thus must be evaluated later
      str += stringFromSessionTokens( editor()->parseSession(), ast->type_parameter->type_id->start_token, ast->type_parameter->type_id->end_token );

      defaultParam = QualifiedIdentifier(str);

      decl->setDefaultParameter(defaultParam);
    }

    if( ast->parameter_declaration ) {
      //Extract default parameters(not tested)
      QualifiedIdentifier defaultParam;
      if( ast->parameter_declaration->type_specifier ) {
        TypeCompiler tc(editor()->parseSession());
        tc.run(ast->parameter_declaration->type_specifier);
        defaultParam = tc.identifier();
      } else {
        QString str;
        ///Only record the strings, because these expressions may depend on template-parameters and thus must be evaluated later
        if( ast->parameter_declaration->declarator )
          str += stringFromSessionTokens( editor()->parseSession(), ast->parameter_declaration->start_token, ast->parameter_declaration->end_token );
        if( ast->parameter_declaration->expression )
          str += stringFromSessionTokens( editor()->parseSession(), ast->parameter_declaration->expression->start_token, ast->parameter_declaration->expression->end_token );
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
  setComment(CommentFormatter::formatComment(comments, editor()->parseSession()));
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
    Declaration* decl = openFunctionDeclaration(node->id, node);
    
    if( !m_functionDefinedStack.isEmpty() ) {
        DUChainWriteLocker lock(DUChain::lock());
        decl->setDeclarationIsDefinition( (bool)m_functionDefinedStack.top() );
    }

    applyFunctionSpecifiers();
  } else {
    openDefinition(node->id, node, node->id == 0);
  }

  applyStorageSpecifiers();

  DeclarationBuilderBase::visitDeclarator(node);

  if (node->parameter_declaration_clause) {
    if (!m_functionDefinedStack.isEmpty() && m_functionDefinedStack.top() && node->id) {

      DUChainWriteLocker lock(DUChain::lock());
      //We have to search for the fully qualified identifier, so we always get the correct class
      QualifiedIdentifier id = currentContext()->scopeIdentifier(false) + identifierForNode(node->id);
      id.setExplicitlyGlobal(true);

      if (id.count() > 1 ||
          (m_inFunctionDefinition && (currentContext()->type() == DUContext::Namespace || currentContext()->type() == DUContext::Global))) {
        SimpleCursor pos = currentDeclaration()->range().start;//editor()->findPosition(m_functionDefinedStack.top(), KDevelop::EditorIntegrator::FrontEdge);
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

            dec->setDefinition(currentDeclaration());
            found = true;
            break;
          }
          if(found)
            break;
        }
      }
    }
  }

  closeDeclaration();

  node->parameter_declaration_clause = parameter_declaration_clause_backup;
}

ForwardDeclaration * DeclarationBuilder::openForwardDeclaration(NameAST * name, AST * range)
{
  return openDeclaration<ForwardDeclaration>(name, range);
}

template<class Type>
Type hasTemplateContext( const QList<Type>& contexts ) {
  foreach( const Type& context, contexts )
    if( context->type() == KDevelop::DUContext::Template )
      return context;
  return Type(0);
}

template<class Type>
Type hasTemplateContext( const QVector<Type>& contexts ) {
  foreach( const Type& context, contexts )
    if( context->type() == KDevelop::DUContext::Template )
      return context;

  return Type(0);
}

//Check whether the given context is a template-context by checking whether it imports a template-parameter context
KDevelop::DUContext* isTemplateContext( KDevelop::DUContext* context ) {
  return hasTemplateContext( context->importedParentContexts() ).data();
}

template<class T>
T* DeclarationBuilder::openDeclaration(NameAST* name, AST* rangeNode, const Identifier& customName, bool collapseRange)
{
  DUChainWriteLocker lock(DUChain::lock());
  
  if( KDevelop::DUContext* templateCtx = hasTemplateContext(m_importedParentContexts) ) {
    Cpp::SpecialTemplateDeclaration<T>* ret = openDeclarationReal<Cpp::SpecialTemplateDeclaration<T> >( name, rangeNode, customName, collapseRange );
    ret->setTemplateParameterContext(templateCtx);
    return ret;
  } else{
    return openDeclarationReal<T>( name, rangeNode, customName, collapseRange );
  }
}

template<class T>
T* DeclarationBuilder::openDeclarationReal(NameAST* name, AST* rangeNode, const Identifier& customName, bool collapseRange)
{
  SimpleRange newRange;
  if(name) {
    std::size_t start = name->unqualified_name->start_token;
    std::size_t end = name->unqualified_name->end_token;
    
    //We must exclude the tilde. Else we may get totally messed up ranges when the name of a destructor is renamed in a macro
    if(name->unqualified_name->tilde)
      start = name->unqualified_name->tilde+1;
    
    newRange = editor()->findRange(start, end);
  }else{
    newRange = editor()->findRange(rangeNode);
  }
  
  if(collapseRange)
    newRange.end = newRange.start;

  Identifier localId = customName;

  if (name) {
    TypeSpecifierAST* typeSpecifier = 0; //Additional type-specifier for example the return-type of a cast operator. This is not catched by the type builder.
    QualifiedIdentifier id = identifierForNode(name, &typeSpecifier);
    
    static Identifier castIdentifier("operator{...cast...}");
    
    if( typeSpecifier && id.last() == castIdentifier ) {
      if( typeSpecifier->kind == AST::Kind_SimpleTypeSpecifier )
        visitSimpleTypeSpecifier( static_cast<SimpleTypeSpecifierAST*>( typeSpecifier ) );
    }
    if(localId.isEmpty())
      localId = id.last();
  }

  T* declaration = 0;

  if (recompiling()) {
    // Seek a matching declaration

    // Translate cursor to take into account any changes the user may have made since the text was retrieved
    QMutexLocker smartLock(editor()->smartMutex());
    SimpleRange translated = editor()->translate(newRange);

#ifdef DEBUG_UPDATE_MATCHING
    kDebug() << "checking" << localId.toString() << "range" << translated.textRange();
#endif

    ///@todo maybe order the declarations within ducontext and change here back to walking the indices, because that's easier to debug and faster
    foreach( Declaration* dec, currentContext()->allLocalDeclarations(localId) ) {

      if( wasEncountered(dec) )
        continue;

#ifdef DEBUG_UPDATE_MATCHING
      if( !(typeid(*dec) == typeid(T)) )
        kDebug() << "typeid mismatch:" << typeid(*dec).name() << typeid(T).name();
      
      if (!(dec->range() == translated))
        kDebug() << "range mismatch" << dec->range().textRange() << translated.textRange();
      
      if(!(localId == dec->identifier()))
        kDebug() << "id mismatch" << dec->identifier().toString() << localId.toString();
#endif
      
        //This works because dec->textRange() is taken from a smart-range. This means that now both ranges are translated to the current document-revision.
      if (dec->range() == translated &&
          localId == dec->identifier() &&
          typeid(*dec) == typeid(T)
         )
      {
        // Match
        declaration = dynamic_cast<T*>(dec);
        break;
      }
    }
  }
#ifdef DEBUG_UPDATE_MATCHING
  if(declaration)
    kDebug() << "found match for" << localId.toString();
  else
    kDebug() << "nothing found for" << localId.toString();
#endif

  if (!declaration) {
    if(currentContext()->inSymbolTable())
      m_changeWasSignificant = true; //We are adding a declaration that comes into the symbol table, so mark the change significant
/*    if( recompiling() )
      kDebug(9007) << "creating new declaration while recompiling: " << localId << "(" << newRange.textRange() << ")";*/
    SmartRange* prior = editor()->currentRange();

    ///We don't want to move the parent range around if the context is collapsed, so we find a parent range that can hold this range.
    QVarLengthArray<SmartRange*, 5> backup;
    while(editor()->currentRange() && !editor()->currentRange()->contains(newRange.textRange())) {
      backup.append(editor()->currentRange());
      editor()->exitCurrentRange();
    }
    
    if(prior && !editor()->currentRange()) {
      editor()->setCurrentRange(backup[backup.count()-1]);
      backup.resize(backup.size()-1);
    }
    
    SmartRange* range = editor()->createRange(newRange.textRange());

    editor()->exitCurrentRange();
    
    for(int a = backup.size()-1; a >= 0; --a)
      editor()->setCurrentRange(backup[a]);

    Q_ASSERT(editor()->currentRange() == prior);

    declaration = new T(editor()->currentUrl(), newRange, currentContext());
    declaration->setSmartRange(range);
    declaration->setIdentifier(localId);
  }

  //Clear some settings
  AbstractFunctionDeclaration* funDecl = dynamic_cast<AbstractFunctionDeclaration*>(declaration);
  if(funDecl)
    funDecl->clearDefaultParameters();
  
  declaration->setDeclarationIsDefinition(false); //May be set later

  declaration->setIsTypeAlias(m_inTypedef);

  if( localId.templateIdentifiersCount() ) {
    TemplateDeclaration* templateDecl = dynamic_cast<TemplateDeclaration*>(declaration);
    if( declaration && templateDecl ) {
      ///This is a template-specialization. Find the class it is specialized from.
      localId.clearTemplateIdentifiers();

      ///@todo Make sure the searched class is in the same namespace
      QList<Declaration*> decls = currentContext()->findDeclarations(QualifiedIdentifier(localId), editor()->findPosition(name->start_token, KDevelop::EditorIntegrator::FrontEdge) );

      if( !decls.isEmpty() )
      {
        if( decls.count() > 1 )
          kDebug(9007) << "Found multiple declarations of specialization-base" << localId.toString() << "for" << declaration->toString();

        foreach( Declaration* decl, decls )
          if( TemplateDeclaration* baseTemplateDecl = dynamic_cast<TemplateDeclaration*>(decl) )
            templateDecl->setSpecializedFrom(baseTemplateDecl);

        if( !templateDecl->specializedFrom() )
          kDebug(9007) << "Could not find valid specialization-base" << localId.toString() << "for" << declaration->toString();
      }
    } else {
      kDebug(9007) << "Specialization of non-template class" << declaration->toString();
    }

  }

  declaration->setComment(comment());
  clearComment();

  setEncountered(declaration);

  openDeclarationInternal(declaration);

  return declaration;
}

Declaration* DeclarationBuilder::openDefinition(NameAST* name, AST* rangeNode, bool collapseRange)
{
  Declaration* ret = openNormalDeclaration(name, rangeNode, KDevelop::Identifier(), collapseRange);
  
  DUChainWriteLocker lock(DUChain::lock());
  ret->setDeclarationIsDefinition(true);
  return ret;
}

Declaration* DeclarationBuilder::openNormalDeclaration(NameAST* name, AST* rangeNode, const Identifier& customName, bool collapseRange) {
  if(currentContext()->type() == DUContext::Class) {
    ClassMemberDeclaration* mem = openDeclaration<ClassMemberDeclaration>(name, rangeNode, customName, collapseRange);
    
    DUChainWriteLocker lock(DUChain::lock());
    mem->setAccessPolicy(currentAccessPolicy());
    return mem;
  } else if(currentContext()->type() == DUContext::Template) {
    return openDeclaration<TemplateParameterDeclaration>(name, rangeNode, customName, collapseRange);
  } else {
    return openDeclaration<Declaration>(name, rangeNode, customName, collapseRange);
  }
}

Declaration* DeclarationBuilder::openFunctionDeclaration(NameAST* name, AST* rangeNode) {
  
   QualifiedIdentifier id = identifierForNode(name);
   Identifier localId = id.last(); //This also copies the template arguments
   if(id.count() > 1) {
     //Merge the scope of the declaration, and put them tog. Add semicolons instead of the ::, so you can see it's not a qualified identifier.
     //Else the declarations could be confused with global functions.
     //This is done before the actual search, so there are no name-clashes while searching the class for a constructor.
     
     QString newId = id.last().identifier();
     for(int a = id.count()-2; a >= 0; --a)
       newId = id.at(a).identifier() + ";;" + newId;
 
     localId.setIdentifier(newId);
   }
  
  if(currentContext()->type() == DUContext::Class) {
    ClassFunctionDeclaration* fun = openDeclaration<ClassFunctionDeclaration>(name, rangeNode, localId);
    fun->setAccessPolicy(currentAccessPolicy());
    return fun;
  } else {
    return openDeclaration<FunctionDeclaration>(name, rangeNode, localId);
  }
}

void DeclarationBuilder::classTypeOpened(AbstractType::Ptr type) {
  //We override this so we can get the class-declaration into a usable state(with filled type) earlier
    DUChainWriteLocker lock(DUChain::lock());

    IdentifiedType* idType = dynamic_cast<IdentifiedType*>(type.data());

    if( idType && idType->declaration() == 0 ) //When the given type has no declaration yet, assume we are declaring it now
        idType->setDeclaration( currentDeclaration() );

    currentDeclaration()->setType(type);
}

void DeclarationBuilder::closeDeclaration(bool forceInstance)
{
  if (lastType()) {
    DUChainWriteLocker lock(DUChain::lock());

    IdentifiedType* idType = dynamic_cast<IdentifiedType*>(lastType().data());
    DelayedType* delayed = dynamic_cast<DelayedType*>(lastType().data());

    //When the given type has no declaration yet, assume we are declaring it now.
    //If the type is a delayed type, it is a searched type, and not a declared one, so don't set the declaration then.
    if( !forceInstance && idType && idType->declaration() == 0 && !delayed )
        idType->setDeclaration( currentDeclaration() );

    if(currentDeclaration()->kind() != Declaration::NamespaceAlias && currentDeclaration()->kind() != Declaration::Alias) {
      //If the type is not identified, it is an instance-declaration too, because those types have no type-declarations.
      if( (((!idType) || (idType && idType->declaration() != currentDeclaration())) && !currentDeclaration()->isTypeAlias() && !currentDeclaration()->isForwardDeclaration() ) || dynamic_cast<AbstractFunctionDeclaration*>(currentDeclaration()) || forceInstance )
        currentDeclaration()->setKind(Declaration::Instance);
      else
        currentDeclaration()->setKind(Declaration::Type);
    }

    currentDeclaration()->setType(lastType());
  }

  eventuallyAssignInternalContext();

  //kDebug(9007) << "Mangled declaration:" << currentDeclaration()->mangledIdentifier();

  DeclarationBuilderBase::closeDeclaration();
}

void DeclarationBuilder::visitTypedef(TypedefAST *def)
{
  parseComments(def->comments);
  
  PushValue<bool> setInTypedef(m_inTypedef, true);
  
  DeclarationBuilderBase::visitTypedef(def);
}

void DeclarationBuilder::visitEnumSpecifier(EnumSpecifierAST* node)
{
  openDefinition(node->name, node, node->name == 0);

  DeclarationBuilderBase::visitEnumSpecifier(node);

  closeDeclaration();
}

void DeclarationBuilder::visitEnumerator(EnumeratorAST* node)
{
  //Ugly hack: Since we want the identifier only to have the range of the id(not
  //the assigned expression), we change the range of the node temporarily
  size_t oldEndToken = node->end_token;
  node->end_token = node->id + 1;

  Identifier id(editor()->parseSession()->token_stream->token(node->id).symbol());
  openNormalDeclaration(0, node, id);

  node->end_token = oldEndToken;

  DeclarationBuilderBase::visitEnumerator(node);

  CppEnumeratorType* enumeratorType = dynamic_cast<CppEnumeratorType*>(lastType().data());
  
  if(ClassMemberDeclaration* classMember = dynamic_cast<ClassMemberDeclaration*>(currentDeclaration())) {
    DUChainWriteLocker lock(DUChain::lock());
    classMember->setStatic(true);
  }
  
  closeDeclaration(true);
  
  if(enumeratorType) {
    DUChainWriteLocker lock(DUChain::lock());
    enumeratorType->setDeclaration(currentDeclaration());
  }else{
    kWarning() << "not assigned enumerator type";
  }
}

void DeclarationBuilder::visitClassSpecifier(ClassSpecifierAST *node)
{
  PushValue<bool> setNotInTypedef(m_inTypedef, false);

  /**Open helper contexts around the class, so the qualified identifier matches.
   * Example: "class MyClass::RealClass{}"
   * Will create one helper-context named "MyClass" around RealClass
   * */
  
  SimpleCursor pos = editor()->findPosition(node->start_token, KDevelop::EditorIntegrator::FrontEdge);

  QualifiedIdentifier id;
  if( node->name ) {
    id = identifierForNode(node->name);
    openPrefixContext(node, id, pos);
  }

  openDefinition(node->name, node, node->name == 0);

  int kind = editor()->parseSession()->token_stream->kind(node->class_key);
  if (kind == Token_struct || kind == Token_union)
    m_accessPolicyStack.push(Declaration::Public);
  else
    m_accessPolicyStack.push(Declaration::Private);

  DeclarationBuilderBase::visitClassSpecifier(node);

  eventuallyAssignInternalContext();

  if( node->name ) {
    ///Copy template default-parameters from the forward-declaration to the real declaration if possible
    DUChainWriteLocker lock(DUChain::lock());

    QList<Declaration*> declarations = Cpp::findDeclarationsSameLevel(currentContext(), id.last(), pos);

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

                  const QVector<Declaration*>& forwardList = forwardTemplateContext->localDeclarations();
                  const QVector<Declaration*>& realList = currentTemplateContext->localDeclarations();

                  QVector<Declaration*>::const_iterator forwardIt = forwardList.begin();
                  QVector<Declaration*>::const_iterator realIt = realList.begin();

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

void DeclarationBuilder::visitUsing(UsingAST * node)
{
  DeclarationBuilderBase::visitUsing(node);

  QualifiedIdentifier id = identifierForNode(node->name);
  
  ///@todo only use the last name component as range
  AliasDeclaration* decl = openDeclaration<AliasDeclaration>(0, node->name ? (AST*)node->name : (AST*)node, id.last());
  {
    DUChainReadLocker lock(DUChain::lock());
    
    SimpleCursor pos = editor()->findPosition(node->start_token, KDevelop::EditorIntegrator::FrontEdge);
    QList<Declaration*> declarations = currentContext()->findDeclarations(id, pos);
    if(!declarations.isEmpty()) {
      decl->setAliasedDeclaration(DeclarationPointer(declarations[0]));
    }else{
      kDebug() << "Aliased declaration not found:" << id.toString();
    }
  }

  closeDeclaration();
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

  if( compilingContexts() ) {
    NamespaceAliasDeclaration* decl = openDeclaration<NamespaceAliasDeclaration>(0, node, globalImportIdentifier);
    {
      DUChainWriteLocker lock(DUChain::lock());
      decl->setImportIdentifier( resolveNamespaceIdentifier(identifierForNode(node->name), currentDeclaration()->range().start) );
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

  if( compilingContexts() ) {
    NamespaceAliasDeclaration* decl = openDeclaration<NamespaceAliasDeclaration>(0, node, Identifier(editor()->parseSession()->token_stream->token(node->namespace_name).symbol()));
    {
      DUChainWriteLocker lock(DUChain::lock());
      decl->setImportIdentifier( resolveNamespaceIdentifier(identifierForNode(node->alias_name), currentDeclaration()->range().start) );
    }
    closeDeclaration();
  }
}

void DeclarationBuilder::visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST* node)
{
  int kind = editor()->parseSession()->token_stream->kind(node->type);

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
    QualifiedIdentifier id = identifierForNode(node->name);

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
      SimpleCursor pos = editor()->findPosition(node->start_token, KDevelop::EditorIntegrator::FrontEdge);

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
            injectType(AbstractType::Ptr(declarations.first()->abstractType().data()));
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
      int kind = editor()->parseSession()->token_stream->kind(it->element);
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
      int kind = editor()->parseSession()->token_stream->kind(it->element);
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
      int kind = editor()->parseSession()->token_stream->kind(it->element);
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
      //Fill default-parameters
      QString defaultParam;
      for( size_t token = node->expression->start_token; token != node->expression->end_token; ++token )
        defaultParam += editor()->tokenToString(token);


      function->addDefaultParameter(defaultParam);
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
    {
      DUChainReadLocker lock(DUChain::lock());
      if(currentContext()->type() == DUContext::Other) //Cannot declare a function in a code-context
        return false; ///@todo create warning/error
    }
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
