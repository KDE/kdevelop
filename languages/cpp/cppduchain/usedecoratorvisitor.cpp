/* This file is part of KDevelop
    Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

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

#include "usedecoratorvisitor.h"
#include <parsesession.h>
#include <lexer.h>
#include <tokens.h>
#include <language/duchain/indexedstring.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/use.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/types/delayedtype.h>
#include <language/duchain/types/referencetype.h>
#include <util/pushvalue.h>
#include <language/checks/dataaccessrepository.h>

using namespace KDevelop;

QString nodeToString(const ParseSession* s, AST* node)
{
  QString ret;
  if(!node) { return "<null>";}
  
  for(uint i=node->start_token; i<node->end_token; i++) {
    ret += ' '+s->token_stream->token(i).symbolString();
  }
  return ret;
}

AbstractType::Ptr constructReadOnlyType()
{
  static AbstractType::Ptr ntype(new DelayedType);
  return ntype;
}

ReferenceType::Ptr constructReferenceType()
{
  static AbstractType::Ptr ntype(new DelayedType);
  static ReferenceType::Ptr reftype(new ReferenceType);
  reftype->setBaseType(ntype);
  
  return reftype;
}

#define LOCKDUCHAIN     DUChainWriteLocker lock(DUChain::lock())
typedef PushPositiveValue<DUContext*> PushPositiveContext;

UseDecoratorVisitor::UseDecoratorVisitor(const ParseSession* session, DataAccessRepository* repo)
  : m_session(session), m_defaultFlags(DataAccess::Read), m_mods(repo)
{
  Q_ASSERT(repo);
}

void UseDecoratorVisitor::run(AST* node)
{
  visit(node);
}

void UseDecoratorVisitor::visitUnqualifiedName(UnqualifiedNameAST* node)
{
  PushPositiveContext pushContext( m_currentContext, node->ducontext );
  //Type exctraction
  AbstractType::Ptr type;
  if(!m_callStack.isEmpty())
    type = m_callStack.top().at(m_argStack.top());
 
  DUContext* ctx = m_currentContext;
  IndexedString id = m_session->token_stream->token(node->id).symbol();
//   qDebug() << "found name" << id.str() << (type ? type->toString() : "no type") << ctx;
  
  if(type && ctx) {
    LOCKDUCHAIN;
    //Use extraction
    CursorInRevision cursor = m_session->positionAt( m_session->token_stream->position(node->start_token) );
    
    //use rw computation
    KDevelop::DataAccess::DataAccessFlags f = m_defaultFlags;
    if(type->whichType()==AbstractType::TypeReference && !(type.cast<ReferenceType>() && type.cast<ReferenceType>()->baseType()->modifiers() & AbstractType::ConstModifier)) {
      f |= DataAccess::Write;
    }
//     qDebug() << "adding..." << f << nodeToString(m_session, node);
    m_mods->addModification(cursor, f);
  }
  
  //further visit
  visit(node->operator_id);
  visitNodes(this, node->template_arguments);
}

void UseDecoratorVisitor::visitFunctionCall(FunctionCallAST* node)
{
  PushPositiveContext pushContext( m_currentContext, node->ducontext );
  IndexedString id = m_session->token_stream->token(node->start_token).symbol();
  
  FunctionType::Ptr type = m_session->typeFromCallAst(node);
  qDebug() << "function call" << id.str() << (type ? type->toString() : "caca");
  
  if(type) {
    m_callStack.push(type->arguments());
    m_argStack.push(0);
    DefaultVisitor::visitFunctionCall(node);
    m_argStack.pop();
    m_callStack.pop();
  } else {
    kDebug() << "couldn't find the type for " << nodeToString(m_session, node);
  }
}


void UseDecoratorVisitor::visitNewExpression(NewExpressionAST* node)
{
    PushPositiveContext pushContext( m_currentContext, node->ducontext );

    IndexedString id = m_session->token_stream->token(node->start_token).symbol();
  
    FunctionType::Ptr type = m_session->typeFromCallAst(node);
    qDebug() << "new constructor call" << id.str() << (type ? type->toString() : "caca");
    QList<AbstractType::Ptr> args;
    if(type)
      args = type->arguments();
    else {
      kDebug() << "couldn't find the type for " << node << nodeToString(m_session, node);
      
      args.append(constructReadOnlyType());
    }
    
    m_callStack.push(args);
    m_argStack.push(0);
    visit(node->expression);
    visit(node->type_id);
    visit(node->new_initializer->expression);
//     visit(node->new_type_id);
    m_argStack.pop();
    m_callStack.pop();
}

void UseDecoratorVisitor::visitBinaryExpression(BinaryExpressionAST* node)
{
  //we have two use cases here: the , parameter where we only want to advance in case we're in a function call argument list
  //or it's an operator expression and we want to visit the two sides of the expression.
  
  qDebug() << "BinaryExpression" << m_session->token_stream->token(node->op).symbolString()
                << nodeToString(m_session, node)
                << m_session->positionAt( m_session->token_stream->position(node->start_token) );
  
  PushPositiveContext pushContext( m_currentContext, node->ducontext );
  FunctionType::Ptr optype = m_session->typeFromCallAst(node);
  Token optoken = m_session->token_stream->token(node->op);
  bool isFunctionArguments = optoken.kind==',';
  
  QList< AbstractType::Ptr > args;
  m_defaultFlags = DataAccess::Read;
  if(optype) {
    args = optype->arguments();
    if(args.size()==1) { //if there's just the last argument, create a delayed type of the first with the proper flags
      AbstractType::Ptr ntype(new DelayedType);
      ntype->setModifiers(optype->modifiers());
      args.prepend(ntype);
    }
  } else if(!isFunctionArguments) {
    static AbstractType::Ptr ntype(new DelayedType);
    if(optoken.kind=='=') {
      static ReferenceType::Ptr reftype(new ReferenceType);
      reftype->setBaseType(ntype);
      
      args += reftype.cast<AbstractType>();
      m_defaultFlags = 0;
    } else
      args += ntype;
    args += ntype;
  }
  
  if(!args.isEmpty()) {
    m_callStack.push(args);
    m_argStack.push(0);
  }
  
  visit(node->left_expression);
  
  //argstack can be empty in cases like "int a,b,c;"
  if(!m_argStack.isEmpty() && (optype || isFunctionArguments)) {
    ++m_argStack.top();
    qDebug() << "advancing parameter" << m_argStack.top() << "over" << m_callStack.top().size();
    Q_ASSERT(m_callStack.top().size()>m_argStack.top());
  }
  m_defaultFlags = DataAccess::Read;
  
  visit(node->right_expression);
  
  if(!args.isEmpty()) {
    m_argStack.pop();
    m_callStack.pop();
  }
}

void UseDecoratorVisitor::visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST* node)
{
    PushPositiveContext pushContext( m_currentContext, node->ducontext );
//     visit(node->expression);
    visit(node->declaration);
}

void UseDecoratorVisitor::visitUnaryExpression(UnaryExpressionAST* node)
{
  PushPositiveContext pushContext( m_currentContext, node->ducontext );
  FunctionType::Ptr optype = m_session->typeFromCallAst(node);
  Token optoken = m_session->token_stream->token(node->op);
  
  QList< AbstractType::Ptr > args;
  if(optype) {
    args = optype->arguments();
    if(args.isEmpty()) { //if there's no argument, create a delayed type of the first with the proper flags
      AbstractType::Ptr ntype(new DelayedType);
      ntype->setModifiers(optype->modifiers());
      if(optype->modifiers()&AbstractType::ConstModifier) {
        ReferenceType::Ptr reftype(new ReferenceType);
        reftype->setBaseType(ntype);
        
        args.append(reftype.cast<AbstractType>());
      } else
        args.append(ntype);
    }
  } else {
    if(optoken.kind==Token_incr || optoken.kind==Token_decr) {
      
      args.append(constructReferenceType().cast<AbstractType>());
    } else {
      static AbstractType::Ptr ntype(new DelayedType);
      args.append(ntype.cast<AbstractType>());
    }
  }
  
  m_callStack.push(args);
  m_argStack.push(0);
  
  PushValue<KDevelop::DataAccess::DataAccessFlags> v(m_defaultFlags, DataAccess::Read);
  visit(node->expression);
  
  m_argStack.pop();
  m_callStack.pop();
}

void UseDecoratorVisitor::visitMemInitializer(MemInitializerAST* node)
{
  PushPositiveContext pushContext( m_currentContext, node->ducontext );
  PushValue<KDevelop::DataAccess::DataAccessFlags> v(m_defaultFlags, DataAccess::Write);
  
  static AbstractType::Ptr ntype(new DelayedType);
  m_callStack.push(QList< AbstractType::Ptr >() << ntype);
  m_argStack.push(0);
  
  visit(node->initializer_id);
  
  m_defaultFlags = DataAccess::Read;
  FunctionType::Ptr optype = m_session->typeFromCallAst(node);
  if(optype) {
    m_callStack.pop();
    m_callStack.push(optype->arguments());
  }
  m_argStack.top()=0;
  
  visit(node->expression);
  
  m_argStack.pop();
  m_callStack.pop();
}

void UseDecoratorVisitor::visitConditionalExpression(ConditionalExpressionAST* node)
{
  PushPositiveContext pushContext( m_currentContext, node->ducontext );
  PushValue<KDevelop::DataAccess::DataAccessFlags> v(m_defaultFlags, DataAccess::Read);
  
  static AbstractType::Ptr ntype(new DelayedType);
  m_callStack.push(QList< AbstractType::Ptr >() << ntype);
  m_argStack.push(0);
  
  visit(node->condition);
  visit(node->left_expression);
  visit(node->right_expression);
  
  m_argStack.pop();
  m_callStack.pop();
}

template <class _Tp>
void visitNodesBackwardsRec(Visitor *v, const ListNode<_Tp> *it,  const ListNode<_Tp>* end)
{
  if(it != end)
    visitNodesBackwardsRec(v, it->next, end);
  v->visit(it->element);
}

template <class _Tp>
void visitNodesBackwards(Visitor *v, const ListNode<_Tp> *nodes)
{
  if (!nodes)
    return;

  const ListNode<_Tp>
    *it = nodes->toFront(),
    *end = it;
  
  visitNodesBackwardsRec(v, it->next, end);
}


void UseDecoratorVisitor::visitPostfixExpression(PostfixExpressionAST* node)
{
//   qDebug() << "visit: PostfixExpression" << nodeToString(m_session, node);
  PushPositiveContext pushContext( m_currentContext, node->ducontext );
  PushValue<KDevelop::DataAccess::DataAccessFlags> v(m_defaultFlags, DataAccess::Read);
  
  static AbstractType::Ptr ntype(new DelayedType);
  
  m_callStack.push(QList< AbstractType::Ptr >() << ntype);
  m_argStack.push(0);
  
  visit(node->type_specifier); //visit specifiers
  visitNodesBackwards(this, node->sub_expressions); //visit subexpressions, like ++ in (e++) and .b in (e.b)
  visit(node->expression);
  
  m_argStack.pop();
  m_callStack.pop();
}

void UseDecoratorVisitor::visitIncrDecrExpression(IncrDecrExpressionAST* node)
{
  PushPositiveContext pushContext( m_currentContext, node->ducontext );
  PushValue<KDevelop::DataAccess::DataAccessFlags> v(m_defaultFlags, DataAccess::Read);
  FunctionType::Ptr optype = m_session->typeFromCallAst(node);
  
  if(optype)
    m_callStack.top()=optype->arguments();
  else
    m_callStack.top()=(QList< AbstractType::Ptr >() << constructReferenceType().cast<AbstractType>());
  
  m_argStack.top() = 0;
}

void UseDecoratorVisitor::visitClassMemberAccess(ClassMemberAccessAST* node)
{
  PushPositiveContext pushContext( m_currentContext, node->ducontext );
  PushValue<KDevelop::DataAccess::DataAccessFlags> v(m_defaultFlags, DataAccess::Read);
  FunctionType::Ptr optype = m_session->typeFromCallAst(node);
  bool modif = optype ? optype->modifiers()&FunctionType::ConstModifier : m_session->token_stream->token(node->op).kind!=Token_arrow;
  
  qDebug() << "class member access" << nodeToString(m_session, node) << modif << optype << (node->kind);
  m_callStack.top()=(QList< AbstractType::Ptr >() << (modif ? constructReferenceType().cast<AbstractType>() : constructReadOnlyType()));
  
  m_argStack.top() = 0;
}

void UseDecoratorVisitor::visitInitDeclarator(InitDeclaratorAST* node)
{
  PushPositiveContext pushContext( m_currentContext, node->ducontext );
  PushValue<KDevelop::DataAccess::DataAccessFlags> v(m_defaultFlags, DataAccess::Read);
  
  visit(node->declarator);
  if(node->initializer) {
    visit(node->initializer);
    
    CursorInRevision cursor = m_session->positionAt( m_session->token_stream->position(node->declarator->start_token) );
    m_mods->addModification(cursor, DataAccess::Write);
  }
}

#define IMPL_DEFAULT_VISIT(a) \
void UseDecoratorVisitor::visit##a(a##AST* node)\
{\
    PushPositiveContext pushContext( m_currentContext, node->ducontext );\
/*qDebug() << "visit: " #a << nodeToString(m_session, node);*/\
    DefaultVisitor::visit##a (node);\
}

# define IMPL_DEFAULT_VISIT_WITH_FLAGS(a, b) \
void UseDecoratorVisitor::visit##a(a##AST* node)\
{\
  PushPositiveContext pushContext( m_currentContext, node->ducontext );\
  PushValue<KDevelop::DataAccess::DataAccessFlags> v(m_defaultFlags, b);\
/*   qDebug() << "visit: " #a << nodeToString(m_session, node);*/\
  static AbstractType::Ptr ntype(new DelayedType);\
  \
  m_callStack.push(QList< AbstractType::Ptr >() << ntype);\
  m_argStack.push(0);\
  \
  DefaultVisitor::visit##a(node);\
  \
  m_argStack.pop();\
  m_callStack.pop();\
}

IMPL_DEFAULT_VISIT_WITH_FLAGS(Condition, DataAccess::Read)
IMPL_DEFAULT_VISIT_WITH_FLAGS(DeleteExpression, DataAccess::Read)
IMPL_DEFAULT_VISIT_WITH_FLAGS(ReturnStatement, DataAccess::Read)

IMPL_DEFAULT_VISIT(CastExpression)
IMPL_DEFAULT_VISIT(CompoundStatement)
IMPL_DEFAULT_VISIT(CppCastExpression)
IMPL_DEFAULT_VISIT(DeclarationStatement)
IMPL_DEFAULT_VISIT(Declarator)
IMPL_DEFAULT_VISIT(ElaboratedTypeSpecifier)
IMPL_DEFAULT_VISIT(ExpressionStatement)
IMPL_DEFAULT_VISIT(Name)
IMPL_DEFAULT_VISIT(NewDeclarator)
IMPL_DEFAULT_VISIT(PrimaryExpression)
IMPL_DEFAULT_VISIT(PtrOperator)
IMPL_DEFAULT_VISIT(SimpleDeclaration)
IMPL_DEFAULT_VISIT(SimpleTypeSpecifier)
IMPL_DEFAULT_VISIT(SizeofExpression)
IMPL_DEFAULT_VISIT(StringLiteral)
IMPL_DEFAULT_VISIT(SubscriptExpression)
IMPL_DEFAULT_VISIT(ThrowExpression)
IMPL_DEFAULT_VISIT(TranslationUnit)
IMPL_DEFAULT_VISIT(TypeId)
IMPL_DEFAULT_VISIT(SignalSlotExpression);
