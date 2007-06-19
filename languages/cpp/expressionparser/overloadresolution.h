/* This file is part of KDevelop
    Copyright (C) 2007 David Nolden [david.nolden.kdevelop  art-master.de]

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

#ifndef OVERLOADRESOLUTION_H
#define OVERLOADRESOLUTION_H

#include <typesystem.h>
#include "typeconversion.h"

class CppFunctionType;

namespace KDevelop {
  class DUContext;
}

namespace Cpp {
using namespace KDevelop;
/**
 * Models overloaded function resolution
 * du-chain must be locked whenever this class is used
 * */
class OverloadResolver {
  public:

    struct ParameterList {
      QList<AbstractType*> parameterTypes;

      ParameterList( AbstractType* param ) {
        parameterTypes << param;
      }
    };

    /**
     * @param container The container in which to search for the functions. If it is a class, base-classes will be respected too.
     * */
    OverloadResolver( DUContext* context );

    CppFunctionType* resolve( const ParameterList& params );

    /**
     * The worst conversion-rank achieved while matching the parameters of the last overload-resolution
     * */
    ConversionRank worstConversionRank();
    
    /**
     * Tries to find a constructor that matches the given parameter-list
     * @param implicit When this is true, constructors with the keyword "explicit" are ignored
     * @param noUserDefinedConversion When this is true, user-defined conversions(constructor- or conversion-function conversion) are not allowed while matching the parameters
     * */
    CppFunctionType* resolveConstructor( const ParameterList& params, bool implicit = false, bool noUserDefinedConversion = false );
  private:
    DUContext* m_context;
    ConversionRank m_worstConversionRank;
};

}

#endif
