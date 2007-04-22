/***************************************************************************
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

#include "verify.h"
#include <sstream>

ExpressionError::ExpressionError( const std::string& fun, int l, const std::string& f, const std::string& exp, const std::string& realExp ) : function( fun ), line( l ), file( f ), expression( exp ), realExpression( realExp ) {}

std::string ExpressionError::what() const {
  std::ostringstream os;
  os << "DynamicTextError in " << function << ", " << file << ":" << line;
  if ( !expression.empty() )
    os << ", expression \"" << expression << "\" failed";
  if ( !realExpression.empty() )
    os << ", values: " << realExpression;
  return os.str();
}

// kate: space-indent on; indent-width 2; tab-width 2; replace-tabs on
