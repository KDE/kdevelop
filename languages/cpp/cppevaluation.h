/***************************************************************************
   begin                : Sat Jul 21 2001
   copyright            : (C) 2001 by Victor R�er
   email                : victor_roeder@gmx.de
   copyright            : (C) 2002,2003 by Roberto Raggi
   email                : roberto@kdevelop.org
   copyright            : (C) 2005 by Adam Treat
   email                : manyoso@yahoo.com
   copyright            : (C) 2006 by David Nolden
   email                : david.nolden.kdevelop@art-master.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef CPPEVALUATION_H
#define CPPEVALUATION_H

#include <qvaluelist.h>

#include "expressioninfo.h"
#include "simpletype.h"
#include "declarationinfo.h"

class SimpleContext;

namespace CppEvaluation {

template<class To, class From>
  extern QValueList<To> convertList( const QValueList<From>& from );

extern QString nameFromType( SimpleType t );

class Operator;

struct OperatorIdentification {
  QValueList<QString> innerParams; /** Inner parameters of the operator( for the vec["hello"] the "hello" ) */
  int start, end; /** Range the operator occupies */
  bool found;
  Operator* op; ///Can be 0 !
  
  OperatorIdentification() : start(0), end(0), found(false), op(0) {
  }
  
  operator bool() {
    return found;
  }
};


class EvaluationResult
{
public:
  EvaluationResult& operator = ( const EvaluationResult& rhs ) {
    resultType = rhs.resultType;
    sourceVariable = rhs.sourceVariable;
    expr = rhs.expr;
    return *this;
  }
  
  EvaluationResult( const EvaluationResult& rhs ) : resultType( rhs.resultType), expr( rhs.expr ), sourceVariable( rhs.sourceVariable ) {
  }
  
  SimpleTypeImpl::SimpleTypeImpl::LocateResult resultType; ///The resulting type
  
  ExpressionInfo expr; ///Information about the expression that was processed
  
  DeclarationInfo sourceVariable; ///If the type comes from a variable, this stores Information about it
  
    ///should be removed
  EvaluationResult( SimpleType rhs ) {
    if( rhs.get() != 0 )
      resultType = rhs->desc();
  }
  
  EvaluationResult( SimpleTypeImpl::LocateResult tp = TypeDesc(), DeclarationInfo var = DeclarationInfo() ): resultType( tp ), sourceVariable( var ) {
  }
  
	/*operator TypeDesc () const {
    return (TypeDesc)resultType;
  }*/
  
    ///This must be removed
  operator SimpleType() const {
    if( resultType->resolved() ) {
      return SimpleType( resultType->resolved() );
    } else {
      return SimpleType( new SimpleTypeImpl( (TypeDesc)resultType ) );
    }
  }
  
  TypeDesc* operator -> () {
    return &resultType.desc();
  }
  
  operator SimpleTypeImpl::LocateResult () const {
    return resultType;
  }
  
  operator bool() const {
    return (bool)resultType;
  }
};



class Operator {
public:
  enum BindingSide {
    Neutral = 0,
      Left = 1,
      Right = 2
  };
  enum Type {
    Unary = 1,
      Binary = 2,
      Ternary = 3
  };
  
  virtual ~Operator() {
  }
  
  virtual int priority() = 0;
  
  virtual Type type() = 0;
  virtual int paramCount() = 0;
  
    ///"binding" means that the operator needs the evaluated type of the expression on that side
    ///The types of all bound sides will later be sent in the "params"-list of the apply-function
  virtual BindingSide binding() = 0;  ///The side to which the operator binds
  
    ///When this returns true, the ident-structure must be filled correctly
  virtual OperatorIdentification identify( QString& str ) = 0;
  
    ///params
  virtual EvaluationResult apply( QValueList<EvaluationResult> params, QValueList<EvaluationResult> innerParams ) = 0;
  
  virtual QString name() = 0;
  
protected:
  void log( const QString& msg );
  QString printTypeList( QValueList<EvaluationResult>& lst );
};


class OperatorSet {
private:
  typedef QValueList< Operator* > OperatorList;
  OperatorList m_operators;
public:
  OperatorSet() {
  }
  
  ~OperatorSet();
  
  void registerOperator( Operator* op ) {
    m_operators << op;
  }
  
  OperatorIdentification identifyOperator( const QString& str_ , Operator::BindingSide allowedBindings = (Operator::BindingSide) (Operator::Left | Operator::Right | Operator::Neutral) );
  
};
extern OperatorSet AllOperators;


template <class OperatorType>
  class RegisterOperator {
  public:
    RegisterOperator( OperatorSet& set ) {
      set.registerOperator( new OperatorType() );
    }
    ~RegisterOperator() {
    }
  };


class UnaryOperator : public Operator{
public:
  UnaryOperator( int priority , QString identString, QString description, Operator::BindingSide binding ) : Operator(), m_priority( priority ), m_identString( identString ), m_name( description ), m_binding( binding )  {
  }
  
  virtual int priority() {    return m_priority;    }
  
  virtual Operator::Type type() {   return Operator::Unary;   }
  
  virtual Operator::BindingSide binding() {   return m_binding;   }
  
  virtual int paramCount() {  return 1; }
  
  virtual OperatorIdentification identify( QString& str );
  
  virtual EvaluationResult unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& innerParams ) = 0;
  
  virtual bool checkParams( const QValueList<EvaluationResult>& params ) {
    return !params.isEmpty() && params[0];
  }
  
  virtual EvaluationResult apply( QValueList<EvaluationResult> params, QValueList<EvaluationResult> innerParams );
  
  virtual QString name() {
    return m_name;
  }

private:
  int m_priority;
  QString m_identString;
  QString m_name;
  Operator::BindingSide m_binding;
protected:
  
  QString identString() const {
    return m_identString;
  }
    
};


class NestedTypeOperator : public UnaryOperator {
public:
  NestedTypeOperator() : UnaryOperator( 18, "::", "nested-type-operator", Operator::Left ) {
  }
  
  virtual EvaluationResult unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& /*innerParams*/ ) {
    return param;
  }
};

//RegisterOperator< NestedTypeOperator > NestedTypeReg( AllOperators ); ///This registers the operator to the list of all operators

class DotOperator : public UnaryOperator {
public:
  DotOperator() : UnaryOperator( 17, ".", "dot-operator", Operator::Left ) {
  }
  
  virtual EvaluationResult unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& /*innerParams*/ ) {
    return param;
  }
};

class ArrowOperator : public UnaryOperator{
public:
  ArrowOperator() : UnaryOperator( 17, "->", "arrow-operator", Operator::Left ) {
  }
    
  virtual EvaluationResult unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& innerParams );  
};


class StarOperator : public UnaryOperator{
public:
  StarOperator() : UnaryOperator( 15, "*", "star-operator", Operator::Right ) { ///Normally this should have a priority of 16, but that would need changes to the expression-parsing-loop
  }
  
  virtual EvaluationResult unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& /*innerParams*/ );
};


class AddressOperator : public UnaryOperator{
public:
  AddressOperator() : UnaryOperator( 16, "&", "address-operator", Operator::Right ) {
  }
  
  virtual EvaluationResult unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& /*innerParams*/ );
};


class UnaryParenOperator : public UnaryOperator {
public:
    ///Identstring should be both parens, for Example "[]" or "()"
  UnaryParenOperator( int priority , QString identString, QString description, Operator::BindingSide binding ) : UnaryOperator( priority, identString, description, binding ) {
  }
  
  virtual OperatorIdentification identify( QString& str );
};

class IndexOperator : public UnaryParenOperator {
public:
  IndexOperator() : UnaryParenOperator( 17, "[]", "index-operator", Operator::Left ) {
  }
  
  virtual EvaluationResult unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& innerParams );
};


class ParenOperator : public UnaryParenOperator {
public:
  ParenOperator() : UnaryParenOperator( 16, "()", "paren-operator", Operator::Left ) {
  }
  
  virtual bool checkParams( const QValueList<EvaluationResult>& params ) {
    return !params.isEmpty();
  }
  
  virtual EvaluationResult unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& innerParams );
};

//This is used in CppCodeCompletion::evaluateExpression(..)
class ExpressionEvaluation {
private:
  CppCodeCompletion* m_data;
  SimpleContext* m_ctx;
  ExpressionInfo m_expr;
  bool m_global;
  OperatorSet& m_operators;
  
public:
  ExpressionEvaluation( CppCodeCompletion* data, ExpressionInfo expr, OperatorSet& operators, SimpleContext* ctx = 0 );
  
  EvaluationResult evaluate();
  
private:
    /**
    recursion-method:
    1. Find the rightmost operator with the lowest priority, split the expression
  
    vector[ (*it)->position ]().
    */
  virtual EvaluationResult evaluateExpressionInternal( QString expr, EvaluationResult scope, SimpleContext * ctx, SimpleContext* innerCtx , bool canBeTypeExpression = false );
  
    ///This does the simplest work
  EvaluationResult evaluateAtomicExpression( QStringList exprList, EvaluationResult scope, SimpleContext * ctx  = 0, bool canBeTypeExpression = false );
};


}

#endif
// kate: indent-mode csands; tab-width 4;
