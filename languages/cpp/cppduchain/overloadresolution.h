/*
   Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

#include <language/duchain/declaration.h>
#include <language/duchain/duchainpointer.h>
#include <QList>
#include "cppduchainexport.h"
#include <language/duchain/identifier.h>

namespace KDevelop {
  class DUContext;
  class Declaration;
}

namespace Cpp {
using namespace KDevelop;
  class ViableFunction;

/**
 * Models overloaded function resolution
 * The du-chain must be locked for the whole lifetime of this object.
 * @todo Give the 'instance' info with each declaration
 * */
class KDEVCPPDUCHAIN_EXPORT OverloadResolver {
  public:

    struct Parameter {
      Parameter() : type(0), lValue(false), declaration(NULL) {
      }
      Parameter( AbstractType::Ptr t, bool isLValue, Declaration * decl = NULL) : type(t), lValue( isLValue ), declaration(decl) {
      }
      AbstractType::Ptr type;
      bool lValue;
      IndexedDeclaration declaration; // if the parameter value is a function name, ADL needs to know it

      ///duchain must be locked
      QString toString() const {
        QString ret;
        if(lValue)
          ret += "lvalue ";
        if(type)
          ret += type->toString();
        else
          ret += "<notype>";
        if (declaration.data()) {
          ret += " (refs declaration ";
          ret += declaration.data()->toString();
          ret += ")";
        }
        return ret;
      }
    };

    struct ParameterList {
      QList<Parameter> parameters;

      ParameterList() {
      }

      ParameterList( const QList<Parameter>& params ) : parameters(params) {
      }

      ParameterList( const Parameter& param ) {
        parameters << param;
      }

      ParameterList( AbstractType::Ptr param, bool isLValue ) {
        parameters << Parameter(param, isLValue);
      }
    };

    enum Constness {
      // only const methods are viable
      Const,
      // both, const and non-const methods are viable,
      // but non-const methods are preferred
      NonConst,
      // both const and non-const methods are viable
      Unknown
    };

    /**
     * @param container The container in which to search for the functions. If it is a class, base-classes will be respected too.
     * @param topContext The top-context of the file where where code-completion/parsing is started from. Needed to resolve forward-declarations.
     * @param constness If set, constness of class methods is taken into account
     * @param forceIsInstance If this is true, every class given to this overload-resolver as an object the function-call is applied on is considered
     *                        an instance of that object, rather then the type itself. This means that "operator()" will be used instead of a constructor.
     * */
    OverloadResolver( DUContextPointer context, TopDUContextPointer topContext, Constness constness = Unknown, bool forceIsInstance = false );

    /**
     * Resolve one function with the given name that matches the given parameters.
     *
     * When classes are found under the given name, their constructors will be considered as functions.
     *
     * @warning du-chain must be locked
     * @param params The parameters
     * @param functionName name of the function
     * @param noUserDefinedConversion should be true when no user-defined conversions are allowed for parameters
     * */
    Declaration* resolve( const ParameterList& params, const QualifiedIdentifier& functionName, bool noUserDefinedConversion = false );

    /**
     * The worst conversion-rank achieved while matching the parameters of the last overload-resolution.
     * It is valued like the results of TypeConversion::implicitConversion(..)
     * */
    uint worstConversionRank();

    /**
     * Tries to find a constructor of the class represented by the current context
     * that matches the given parameter-list
     * @warning du-chain must be locked
     * @param implicit When this is true, constructors with the keyword "explicit" are ignored
     * @param noUserDefinedConversion When this is true, user-defined conversions(constructor- or conversion-function conversion) are not allowed while matching the parameters
     * */
    Declaration* resolveConstructor( const ParameterList& params, bool implicit = false, bool noUserDefinedConversion = false );

    /**
     * Tries to choose the correct function out of a given list of function-declarations.
     * If one of those declarations is a class-declaration, it will be substituted by its constructors.
     * If one of those declarations is an instance of a class, it will be substituted with its operator()
     * functions. Constant references to class-instances are treated correctly.
     *
     * @warning du-chain must be locked

     * @param params parameters to match
     * @param declarations list of declarations
     * @param noUserDefinedConversion should be true if user-defined conversions(conversion-operators and constructor-conversion) are not allowed when matching the parameters
     * @param doADL should be true if one needs to try the ADL lookup as well
     * */
    Declaration* resolveList( const ParameterList& params, const QList<Declaration*>& declarations, bool noUserDefinedConversion = false );

    /**
     * Matches the given functions with the given parameters. Only does partial matching, by considering only those parameters that were
     * actually given, if @param partial is given.
     *
     * The main difference of this call to resolveList(..) is that it for each given declaration, it allows a list of parameters that are prepended.
     *
     * Returns a sorted list containing all given declarations.
     *
     * @warning du-chain must be locked
     *
     * @return List of all given functions. The list is sorted by viability(the first item is most viable). Non-viable functions are also included.
     * */
    QList< ViableFunction > resolveListOffsetted( const ParameterList& params, const QList<QPair<OverloadResolver::ParameterList, Declaration*> >& declarations, bool partial );

    /**
     * Matches the given functions with the given parameters. Only does partial matching, by considering only those parameters that were
     * actually given, if @param partial is given.
     *
     * The main difference of this call to resolveList(..) is that it for each given declaration, it allows a list of parameters that are prepended.
     *
     * (this is more efficient than resolveListOffsetted)
     *
     * @warning du-chain must be locked
     *
     * @return only the most viable function
     * */
    ViableFunction resolveListViable( const ParameterList& params, const QList<QPair<OverloadResolver::ParameterList, Declaration*> >& declarations, bool partial = false );
    
    /**
     * This extracts the template-parameters. It does not do any actual checking whether non-template types are equal or match each other.
     * @param argumentType The type actually given
     * @param parameterType The type argumentType should be matched to. Once CppTemplateParameterType's are encountered here, they will be instantiated in instantiatedTypes
     * @param keepValue If this is true, and a matched type is a constant integral type, it is kept as that exact type including value. Else, the constant integral type is converted
     *                  to the matching value-less integral type. This is typically needed for template-parameter instantiation, which keepValue=false is needed for implicit argument instantiation.
     * @return false if the matching failed
     * */
    uint matchParameterTypes(const AbstractType::Ptr& argumentType, const AbstractType::Ptr& parameterType, QMap<IndexedString, AbstractType::Ptr>& instantiatedTypes, bool keepValue = false) const;

    /**
     * Computes the declarations of functions that could be an Argument-Dependent Lookup match for the given declarations and parameters.
     * The returned declarations might not exist. The function computes all names that would need to be checked based on the
     * parameters' namespaces and types.
     * @param params Current list of parameters
     * @param declarations List of function call declarations
     * @return List of function declarations available for overload resolution with ADL
     */
    QList<Declaration *> computeADLCandidates( const ParameterList& params, const QualifiedIdentifier& identifier );
    
  private:
    ///Replace class-instances with operator() functions, and pure classes with their constructors
    void expandDeclarations( const QList<Declaration*>& from, QSet<Declaration*>& to );
    void expandDeclarations( const QList<QPair<OverloadResolver::ParameterList, Declaration*> >& from, QHash<Declaration*, OverloadResolver::ParameterList>& to );
    ///Returns zero if applying failed. Returns the given declaration if it isn't a template function.
    Declaration* applyImplicitTemplateParameters( const ParameterList& params, Declaration* declaration ) const;

    uint matchParameterTypes(AbstractType::Ptr argumentType, const IndexedTypeIdentifier& parameterType, QMap<IndexedString, AbstractType::Ptr>& instantiatedTypes, bool keepValue) const;
    uint matchParameterTypes(AbstractType::Ptr argumentType, const Identifier& parameterType, QMap<IndexedString, AbstractType::Ptr>& instantiatedTypes, bool keepValue) const;

    DUContextPointer m_context;
    TopDUContextPointer m_topContext;
    uint m_worstConversionRank;
    bool m_forceIsInstance;
    Constness m_constness;
};

}

/**
 * {q,k}Debug() stream operator: Writes the parameter to the debug output.
 */
KDEVCPPDUCHAIN_EXPORT QDebug operator<<(QDebug s, const Cpp::OverloadResolver::Parameter& param);

/**
 * {q,k}Debug() stream operator: Writes the parameters to the debug output.
 */
KDEVCPPDUCHAIN_EXPORT QDebug operator<<(QDebug s, const Cpp::OverloadResolver::ParameterList& params);

#endif
