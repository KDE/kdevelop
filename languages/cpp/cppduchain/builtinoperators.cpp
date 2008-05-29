/* This file is part of KDevelop
    Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include "builtinoperators.h"
#include <typesystem.h>
#include "cpptypes.h"
#include "typerepository.h"
#include "parser/tokens.h"


/** A helper-class for evaluating constant binary expressions under different types(int, float, etc.) */
template<class Type>
struct ConstantBinaryExpressionEvaluator {

  Type endValue;

  CppIntegralType::IntegralTypes type;
  CppIntegralType::TypeModifiers modifier;

  /**
   * Writes the results into endValue, type, and modifier.
   * */
  ConstantBinaryExpressionEvaluator( CppIntegralType::IntegralTypes _type, CppIntegralType::TypeModifiers _modifier, int tokenKind, CppConstantIntegralType* left, CppConstantIntegralType* right ) {
    endValue = 0;
    
    type = _type;
    modifier = _modifier;

    evaluateSpecialTokens(tokenKind, left, right);
    
    switch( tokenKind ) {
      case '+':
        endValue = left->CppConstantIntegralType::value<Type>() + right->CppConstantIntegralType::value<Type>();
      break;
      case '-':
        endValue = left->CppConstantIntegralType::value<Type>() - right->CppConstantIntegralType::value<Type>();
      break;
      case '*':
        endValue = left->CppConstantIntegralType::value<Type>() * right->CppConstantIntegralType::value<Type>();
      break;
      case '/':
        endValue = left->CppConstantIntegralType::value<Type>() / right->CppConstantIntegralType::value<Type>();
      break;
      case '=':
        endValue = right->CppConstantIntegralType::value<Type>();
      break;
      case '<':
        endValue = left->CppConstantIntegralType::value<Type>() < right->CppConstantIntegralType::value<Type>();
        type = CppIntegralType::TypeBool;
      break;
      case '>':
        endValue = left->CppConstantIntegralType::value<Type>() > right->CppConstantIntegralType::value<Type>();
        type = CppIntegralType::TypeBool;
      break;
      case Token_assign:
        endValue = right->CppConstantIntegralType::value<Type>();
      break;
      case Token_eq:
        endValue = left->CppConstantIntegralType::value<Type>() == right->CppConstantIntegralType::value<Type>();
        type = CppIntegralType::TypeBool;
      break;
      case Token_not_eq:
        endValue = left->CppConstantIntegralType::value<Type>() != right->CppConstantIntegralType::value<Type>();
        type = CppIntegralType::TypeBool;
      break;
      case Token_leq:
        endValue = left->CppConstantIntegralType::value<Type>() <= right->CppConstantIntegralType::value<Type>();
        type = CppIntegralType::TypeBool;
      break;
      case Token_geq:
        endValue = left->CppConstantIntegralType::value<Type>() >= right->CppConstantIntegralType::value<Type>();
        type = CppIntegralType::TypeBool;
      break;
    }
  }

  //This function is used to disable some operators on bool and double values
  void evaluateSpecialTokens( int tokenKind, CppConstantIntegralType* left, CppConstantIntegralType* right ) {
    switch( tokenKind ) {
      case '%':
        endValue = left->CppConstantIntegralType::value<Type>() % right->CppConstantIntegralType::value<Type>();
      break;
      case '^':
        endValue = left->CppConstantIntegralType::value<Type>() ^ right->CppConstantIntegralType::value<Type>();
      break;
      case '&':
        endValue = left->CppConstantIntegralType::value<Type>() & right->CppConstantIntegralType::value<Type>();
      break;
      case '|':
        endValue = left->CppConstantIntegralType::value<Type>() | right->CppConstantIntegralType::value<Type>();
      break;
      case Token_shift:
        ///@todo shift-direction?
        endValue = left->CppConstantIntegralType::value<Type>() << right->CppConstantIntegralType::value<Type>();
      break;
      case Token_and:
        endValue = left->CppConstantIntegralType::value<Type>() && right->CppConstantIntegralType::value<Type>();
        type = CppIntegralType::TypeBool;
      break;
      case Token_or:
        endValue = left->CppConstantIntegralType::value<Type>() || right->CppConstantIntegralType::value<Type>();
        type = CppIntegralType::TypeBool;
      break;
    }
  }
  
  KDevelop::AbstractType::Ptr createType() {
    KDevelop::AbstractType::Ptr ret = TypeRepository::self()->registerType( KDevelop::AbstractType::Ptr(new CppConstantIntegralType(type, modifier)) );
    static_cast<CppConstantIntegralType*>(ret.data())->CppConstantIntegralType::setValue<Type>( endValue );
    return ret;
  }
};

template<>
void ConstantBinaryExpressionEvaluator<double>::evaluateSpecialTokens( int tokenKind, CppConstantIntegralType* left, CppConstantIntegralType* right ) {
}

template<>
void ConstantBinaryExpressionEvaluator<float>::evaluateSpecialTokens( int tokenKind, CppConstantIntegralType* left, CppConstantIntegralType* right ) {
}



KDevelop::AbstractType::Ptr binaryOperatorReturnType(KDevelop::AbstractType::Ptr left, KDevelop::AbstractType::Ptr right, int tokenKind) {

  if(!left || !right)
    return KDevelop::AbstractType::Ptr();
  
  CppIntegralType* leftIntegral = dynamic_cast<CppIntegralType*>(left.data());
  CppIntegralType* rightIntegral = dynamic_cast<CppIntegralType*>(right.data());
  CppPointerType* leftPointer = dynamic_cast<CppPointerType*>(right.data());

  KDevelop::AbstractType::Ptr ret;

  //Constantly evaluate integral expressions
  CppConstantIntegralType* leftConstantIntegral = dynamic_cast<CppConstantIntegralType*>(left.data());
  CppConstantIntegralType* rightConstantIntegral = dynamic_cast<CppConstantIntegralType*>(right.data());

  if(leftIntegral && rightIntegral) {
    if(tokenKind == '+' || tokenKind == '-' || tokenKind == '*' || tokenKind == '/' || tokenKind == '%' || tokenKind == '^' || tokenKind == '&' || tokenKind == '|' || tokenKind == '~' || tokenKind == Token_shift) {
      if(leftIntegral->moreExpressiveThan(rightIntegral))
        ret = left;
      else
        ret = right;
    }
    
    if(tokenKind == '<' || tokenKind == '>' || tokenKind == Token_eq || tokenKind == Token_not_eq || tokenKind == Token_leq || tokenKind == Token_geq || tokenKind == Token_not_eq || tokenKind == Token_and || tokenKind == Token_or)
      ret = KDevelop::AbstractType::Ptr(TypeRepository::self()->integral(CppIntegralType::TypeBool, CppIntegralType::ModifierNone).data());
  }
  
  if(leftPointer && rightIntegral && (tokenKind == '+' || tokenKind == '-'))
    ret = left;

  CppIntegralType* retIntegral = dynamic_cast<CppIntegralType*>(ret.data());

  ///We have determined the resulting type now. If both sides are constant, also evaluate the resulting value.
  if(ret && retIntegral && leftConstantIntegral && rightConstantIntegral) {
    switch( retIntegral->integralType() ) {
      case CppIntegralType::TypeFloat:
      {
        ConstantBinaryExpressionEvaluator<float> evaluator( retIntegral->integralType(), retIntegral->typeModifiers(), tokenKind, leftConstantIntegral, rightConstantIntegral );
        return evaluator.createType();
      }
      case CppIntegralType::TypeDouble:
      {
        ConstantBinaryExpressionEvaluator<double> evaluator( retIntegral->integralType(), retIntegral->typeModifiers(), tokenKind, leftConstantIntegral, rightConstantIntegral );
        return evaluator.createType();
      }
      default:
        if( leftConstantIntegral->typeModifiers() & CppIntegralType::ModifierUnsigned ) {
          ConstantBinaryExpressionEvaluator<quint64> evaluator( retIntegral->integralType(), retIntegral->typeModifiers(), tokenKind, leftConstantIntegral, rightConstantIntegral);
          return evaluator.createType();
        } else {
          ConstantBinaryExpressionEvaluator<qint64> evaluator( retIntegral->integralType(), retIntegral->typeModifiers(), tokenKind, leftConstantIntegral, rightConstantIntegral);
          return evaluator.createType();
        }
        break;
    }
  }
  return ret;
}

