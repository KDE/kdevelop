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

#include "cppevaluation.h"
namespace CppEvaluation {

OperatorSet AllOperators;

///These lines register the operators to the list of all operators
RegisterOperator< DotOperator > DotReg( AllOperators );
RegisterOperator< ArrowOperator > ArrowReg( AllOperators );
RegisterOperator< StarOperator > StarReg( AllOperators );
RegisterOperator< AddressOperator > AddressReg( AllOperators );
RegisterOperator< IndexOperator > IndexReg( AllOperators );
RegisterOperator< ParenOperator > ParenReg( AllOperators );

template<class To, class From>  
QValueList<To> convertList( const QValueList<From>& from ) {
  QValueList<To> ret;
  for( typename QValueList<From>::const_iterator it = from.begin(); it != from.end(); ++it ) {
    ret << (To)*it;
  }
  return ret;
}

OperatorIdentification UnaryOperator::identify( QString& str ) {
  OperatorIdentification ret;
  if( str.startsWith( m_identString ) ) {
    ret.start = 0;
    ret.end = m_identString.length();
    ret.found = true;
    ret.op = this;
  }
  return ret;
}



EvaluationResult UnaryOperator::apply( QValueList<EvaluationResult> params, QValueList<EvaluationResult> innerParams ) {
  if( !checkParams( params ) ) {
  log( QString("parameter-check failed: %1 params: ").arg( params.size() ) + printTypeList( params ) );
    return EvaluationResult();
  } else {
    EvaluationResult t = unaryApply( params.front(), innerParams );
    if( !t ) {
      if( params.front() )
        log( "could not apply \"" + name() + "\" to \"" + nameFromType( params.front() ) + "\"");
      else
        log( "operator \"" + name() + "\" applied on \"" + nameFromType( params.front() )  + "\": returning unresolved type \"" + nameFromType( t ) + "\"");
    }
    return t;
  }
}

EvaluationResult ArrowOperator::unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& innerParams ) {
  if( param->pointerDepth() > 0 ) {
    param->decreasePointerDepth();
    return param;
  } else {
    if( param->resolved() ) {
      return param->resolved()->applyOperator( SimpleTypeImpl::ArrowOp , convertList<LocateResult, EvaluationResult>(innerParams) );
    } else {
      kdDebug( 9007 ) << "failed to apply arrow-operator to unresolved type" << endl;
      return EvaluationResult();
    }
  };
}

EvaluationResult StarOperator::unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& /*innerParams*/ ) {
  if( param->pointerDepth() > 0 ) {
    param->decreasePointerDepth();
    return param;
  } else {
    if( param->resolved() ) {
      return param->resolved()->applyOperator( SimpleTypeImpl::StarOp );
    } else {
      kdDebug( 9007 ) << "failed to apply star-operator to unresolved type" << endl;
      return EvaluationResult();
    }
  };
}


EvaluationResult AddressOperator::unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& /*innerParams*/ ) {
  param->setPointerDepth( param->pointerDepth() + 1 );
  return param;
}


OperatorIdentification UnaryParenOperator::identify( QString& str ) {
  OperatorIdentification ret;
  if( str.startsWith( QString( identString()[0] ) ) ) {
    ret.start = 0;
    ret.end = findClose( str, 0 );
    if( ret.end == -1 ) {
      ret.found = false;
      ret.end = 0;
    } else {
      if( str[ret.end] == identString()[1] ) {
        ret.found = true;
        ret.end += 1;
        ret.op = this;
        
                    ///Try to extract the parameter-strings.
        ParamIterator it( identString(), str.mid( ret.start, ret.end - ret.start ) );
        
        while( it ) {
          ret.innerParams << (*it).stripWhiteSpace();
          
          ++it;
        }
        
      } else {
        ret.end = 0;
      }
    }
  }
  return ret;
}

EvaluationResult IndexOperator::unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& innerParams ) {
  if( param->pointerDepth() > 0 ) {
    param->decreasePointerDepth();
    return param;
  } else {
    if( param->resolved() ) {
      return param->resolved()->applyOperator( SimpleTypeImpl::IndexOp, convertList<LocateResult>( innerParams ) );
    } else {
      kdDebug( 9007 ) << "failed to apply index-operator to unresolved type" << endl;
      return EvaluationResult();
    }
  };
}

EvaluationResult ParenOperator::unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& innerParams ) {
  if( param ) {
    if( param->resolved() ) {
      return param->resolved()->applyOperator( SimpleTypeImpl::ParenOp, convertList<LocateResult>(innerParams) );
    } else {
      kdDebug( 9007 ) << "failed to apply paren-operator to unresolved type" << endl;
      return EvaluationResult();
    }
    
  } else {
    return innerParams[0];
  }
}

}

// kate: indent-mode csands; tab-width 4;
