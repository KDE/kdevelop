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
#include "cpptypes.h"
#include <language/duchain/types/constantintegraltype.h>
#include "parser/tokens.h"

using namespace KDevelop;

/** A helper-class for evaluating constant binary expressions under different types(int, float, etc.) */
template<class Type>
struct ConstantBinaryExpressionEvaluator {

  Type endValue;

  uint type;
  uint modifier;

  /**
   * Writes the results into endValue, type, and modifier.
   * */
  ConstantBinaryExpressionEvaluator( uint _type, uint _modifier, quint16 tokenKind, ConstantIntegralType* left, ConstantIntegralType* right ) {
    endValue = 0;

    type = _type;
    modifier = _modifier;

    evaluateSpecialTokens(tokenKind, left, right);

    switch( tokenKind ) {
      case '+':
        endValue = left->ConstantIntegralType::value<Type>() + right->ConstantIntegralType::value<Type>();
      break;
      case '-':
        endValue = left->ConstantIntegralType::value<Type>() - right->ConstantIntegralType::value<Type>();
      break;
      case '*':
          endValue = left->ConstantIntegralType::value<Type>() * right->ConstantIntegralType::value<Type>();
      break;
      case '/':
        if(right->ConstantIntegralType::value<Type>())
          endValue = left->ConstantIntegralType::value<Type>() / right->ConstantIntegralType::value<Type>();
        else
          kDebug() << "bad division operator" << left->ConstantIntegralType::value<Type>() << "/" << right->ConstantIntegralType::value<Type>();
      break;
      case '=':
        endValue = right->ConstantIntegralType::value<Type>();
      break;
      case '<':
        endValue = left->ConstantIntegralType::value<Type>() < right->ConstantIntegralType::value<Type>();
        type = IntegralType::TypeBoolean;
      break;
      case '>':
        endValue = left->ConstantIntegralType::value<Type>() > right->ConstantIntegralType::value<Type>();
        type = IntegralType::TypeBoolean;
      break;
      case Token_eq:
        endValue = left->ConstantIntegralType::value<Type>() == right->ConstantIntegralType::value<Type>();
        type = IntegralType::TypeBoolean;
      break;
      case Token_not_eq:
        endValue = left->ConstantIntegralType::value<Type>() != right->ConstantIntegralType::value<Type>();
        type = IntegralType::TypeBoolean;
      break;
      case Token_leq:
        endValue = left->ConstantIntegralType::value<Type>() <= right->ConstantIntegralType::value<Type>();
        type = IntegralType::TypeBoolean;
      break;
      case Token_geq:
        endValue = left->ConstantIntegralType::value<Type>() >= right->ConstantIntegralType::value<Type>();
        type = IntegralType::TypeBoolean;
      break;
    }
  }

  //This function is used to disable some operators on bool and double values
  void evaluateSpecialTokens( quint16 tokenKind, ConstantIntegralType* left, ConstantIntegralType* right ) {
    switch( tokenKind ) {
      case '%':
        if(right->ConstantIntegralType::value<Type>())
          endValue = left->ConstantIntegralType::value<Type>() % right->ConstantIntegralType::value<Type>();
        else
          kDebug() << "bad modulo operator" << left->ConstantIntegralType::value<Type>() << "%" << right->ConstantIntegralType::value<Type>();
      break;
      case '^':
        endValue = left->ConstantIntegralType::value<Type>() ^ right->ConstantIntegralType::value<Type>();
      break;
      case '&':
        endValue = left->ConstantIntegralType::value<Type>() & right->ConstantIntegralType::value<Type>();
      break;
      case '|':
        endValue = left->ConstantIntegralType::value<Type>() | right->ConstantIntegralType::value<Type>();
      break;
      case Token_leftshift:
        endValue = left->ConstantIntegralType::value<Type>() << right->ConstantIntegralType::value<Type>();
      break;
      case Token_rightshift:
        endValue = left->ConstantIntegralType::value<Type>() >> right->ConstantIntegralType::value<Type>();
      break;
      case Token_and:
        endValue = left->ConstantIntegralType::value<Type>() && right->ConstantIntegralType::value<Type>();
        type = IntegralType::TypeBoolean;
      break;
      case Token_or:
        endValue = left->ConstantIntegralType::value<Type>() || right->ConstantIntegralType::value<Type>();
        type = IntegralType::TypeBoolean;
      break;
    }
  }

  AbstractType::Ptr createType() {
    ConstantIntegralType::Ptr ret( new ConstantIntegralType(type) );
    ret->setModifiers(ret->modifiers() & modifier);
    ret->ConstantIntegralType::setValue<Type>( endValue );
    return AbstractType::Ptr::staticCast(ret);
  }
};

template<>
void ConstantBinaryExpressionEvaluator<double>::evaluateSpecialTokens( quint16 tokenKind, ConstantIntegralType* left, ConstantIntegralType* right ) {
  Q_UNUSED(tokenKind);
  Q_UNUSED(left);
  Q_UNUSED(right);
}

template<>
void ConstantBinaryExpressionEvaluator<float>::evaluateSpecialTokens( quint16 tokenKind, ConstantIntegralType* left, ConstantIntegralType* right ) {
  Q_UNUSED(tokenKind);
  Q_UNUSED(left);
  Q_UNUSED(right);
}



AbstractType::Ptr binaryOperatorReturnType(AbstractType::Ptr left, AbstractType::Ptr right, quint16 tokenKind) {

  if(!left || !right)
    return AbstractType::Ptr();

  IntegralType* leftIntegral = dynamic_cast<IntegralType*>(left.unsafeData());
  IntegralType* rightIntegral = dynamic_cast<IntegralType*>(right.unsafeData());
  PointerType* leftPointer = dynamic_cast<PointerType*>(right.unsafeData());

  AbstractType::Ptr ret;

  //Constantly evaluate integral expressions
  ConstantIntegralType* leftConstantIntegral = dynamic_cast<ConstantIntegralType*>(left.unsafeData());
  ConstantIntegralType* rightConstantIntegral = dynamic_cast<ConstantIntegralType*>(right.unsafeData());

  if(leftIntegral && rightIntegral) {
    if(tokenKind == '+' || tokenKind == '-' || tokenKind == '*' || tokenKind == '/' || tokenKind == '%' || tokenKind == '^' || tokenKind == '&' || tokenKind == '|' || tokenKind == '~' || tokenKind == Token_leftshift || tokenKind == Token_rightshift) {
      if(moreExpressiveThan(leftIntegral, rightIntegral))
        ret = left;
      else
        ret = right;
    }

    if(tokenKind == '<' || tokenKind == '>' || tokenKind == Token_eq || tokenKind == Token_not_eq || tokenKind == Token_leq || tokenKind == Token_geq || tokenKind == Token_not_eq || tokenKind == Token_and || tokenKind == Token_or)
      ret = AbstractType::Ptr(new IntegralType(IntegralType::TypeBoolean));
  }

  if(leftPointer && rightIntegral && (tokenKind == '+' || tokenKind == '-'))
    ret = left;

  IntegralType* retIntegral = dynamic_cast<IntegralType*>(ret.unsafeData());

  ///We have determined the resulting type now. If both sides are constant, also evaluate the resulting value.
  if(ret && retIntegral && leftConstantIntegral && rightConstantIntegral) {
    switch( retIntegral->dataType() ) {
      case IntegralType::TypeFloat:
      {
        ConstantBinaryExpressionEvaluator<float> evaluator( retIntegral->dataType(), retIntegral->modifiers(), tokenKind, leftConstantIntegral, rightConstantIntegral );
        return evaluator.createType();
      }
      case IntegralType::TypeDouble:
      {
        ConstantBinaryExpressionEvaluator<double> evaluator( retIntegral->dataType(), retIntegral->modifiers(), tokenKind, leftConstantIntegral, rightConstantIntegral );
        return evaluator.createType();
      }
      default:
        if( leftConstantIntegral->modifiers() & AbstractType::UnsignedModifier ) {
          ConstantBinaryExpressionEvaluator<quint64> evaluator( retIntegral->dataType(), retIntegral->modifiers(), tokenKind, leftConstantIntegral, rightConstantIntegral);
          return evaluator.createType();
        } else {
          ConstantBinaryExpressionEvaluator<qint64> evaluator( retIntegral->dataType(), retIntegral->modifiers(), tokenKind, leftConstantIntegral, rightConstantIntegral);
          return evaluator.createType();
        }
        break;
    }
  }
  return ret;
}

