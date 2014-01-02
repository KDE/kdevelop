/* This file is part of KDevelop
   Copyright 2012 Olivier de Gaalon <olivier.jg@gmail.com>

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

#ifndef KDEVPLATFORM_JSONDECLARATIONTESTS_H
#define KDEVPLATFORM_JSONDECLARATIONTESTS_H

#include "language/duchain/ducontext.h"
#include "language/duchain/declaration.h"
#include "language/duchain/indexeddeclaration.h"
#include "language/duchain/identifier.h"
#include "language/duchain/abstractfunctiondeclaration.h"
#include "language/duchain/types/typeutils.h"
#include "language/duchain/types/identifiedtype.h"
#include <language/duchain/types/functiontype.h>
#include "language/duchain/duchain.h"
#include "language/duchain/functiondefinition.h"
#include "language/duchain/definitions.h"
#include "jsontesthelpers.h"

/**
 * JSON Object Specification:
 *   DeclTestObject: Mapping of (string) declaration test names to values
 *   TypeTestObject: Mapping of (string) type test names to values
 *   CtxtTestObject: Mapping of (string) context test names to values
 *
 * Quick Reference:
 *   useCount : int
 *   identifier : string
 *   qualifiedIdentifier : string
 *   internalContext : CtxtTestObject
 *   internalFunctionContext : CtxtTestObject
 *   type : TypeTestObject
 *   unaliasedType : TypeTestObject
 *   targetType : TypeTestObject
 *   identifiedTypeQid : string
 *   isVirtual : bool
 *   declaration : DeclTestObject
 *   definition : DeclTestObject
 *   null : bool
 *   defaultParameter : string
 *   toString : string
 *   range : string
 */

namespace KDevelop
{
  template<>
  QString TestSuite<Declaration*>::objectInformation(Declaration *decl)
  {
    if (!decl)
        return "(null declaration)";
    return QString("(Declaration on line %1 in %2)")
        .arg(decl->range().start.line + 1)
        .arg(decl->topContext()->url().str());
  }

namespace DeclarationTests
{

using namespace JsonTestHelpers;

///JSON type: int
///@returns whether the declaration's number of uses matches the given value
DeclarationTest(useCount)
{
  int uses = 0;
  foreach(const QList<RangeInRevision>& useRanges, decl->uses()) {
    uses += useRanges.size();
  }
  return compareValues(uses, value, "Declaration's use count ");
}
///JSON type: string
///@returns whether the declaration's identifier matches the given value
DeclarationTest(identifier)
{
  return compareValues(decl->identifier().toString(), value, "Declaration's identifier");
}
///JSON type: string
///@returns whether the declaration's qualified identifier matches the given value
DeclarationTest(qualifiedIdentifier)
{
  return compareValues(decl->qualifiedIdentifier().toString(), value, "Declaration's qualified identifier");
}
///JSON type: CtxtTestObject
///@returns whether the tests for the declaration's internal context pass
DeclarationTest(internalContext)
{
  return testObject(decl->internalContext(), value, "Declaration's internal context");
}
///JSON type: CtxtTestObject
///@returns whether the tests for the declaration's internal function context pass
DeclarationTest(internalFunctionContext)
{
  const QString NO_INTERNAL_CTXT = "%1 has no internal function context.";
  AbstractFunctionDeclaration *absFuncDecl = dynamic_cast<AbstractFunctionDeclaration*>(decl);
  if (!absFuncDecl || !absFuncDecl->internalFunctionContext())
    return NO_INTERNAL_CTXT.arg(decl->qualifiedIdentifier().toString());
  return testObject(absFuncDecl->internalFunctionContext(), value, "Declaration's internal function context");
}
/*FIXME: The type functions need some renaming and moving around
 * Some (all?) functions from cpp's TypeUtils should be moved to the kdevplatform type utils
 * targetType is exactly like realType except it also tosses pointers
 * shortenTypeForViewing should go to type utils (and it's broken, it places const to the left of all *'s when it should be left or right of the type)
 * UnaliasedType seems to be unable to understand aliases involving templates, perhaps a cpp version is in order
 */
///JSON type: TypeTestObject
///@returns whether the tests for the declaration's type pass
DeclarationTest(type)
{
  return testObject(decl->abstractType(), value, "Declaration's type");
}
///JSON type: TypeTestObject
///@returns whether the tests for the declaration's unaliased type pass (TypeUtils::unaliasedType)
DeclarationTest(unaliasedType)
{
  return testObject(TypeUtils::unAliasedType(decl->abstractType()), value, "Declaration's unaliased type");
}
///JSON type: TypeTestObject
///@returns whether the tests for the declaration's target type pass (TypeUtils::targetType)
DeclarationTest(targetType)
{
  return testObject(TypeUtils::targetType(decl->abstractType(), decl->topContext()), value, "Declaration's target type");
}
///JSON type: TestTypeObject
///@returns the
DeclarationTest(returnType)
{
  FunctionType::Ptr functionType = decl->abstractType().cast<FunctionType>();
  AbstractType::Ptr returnType;
  if (functionType) {
    returnType = functionType->returnType();
  }
  return testObject(returnType, value, "Declaration's return type");
}
///JSON type: string
///@returns whether the declaration's type's declaration can be identified and if it's qualified identifier matches the given value
DeclarationTest(identifiedTypeQid)
{
  VERIFY_TYPE(QString);
  const QString UN_ID_ERROR = "Unable to identify declaration of type \"%1\".";
  AbstractType::Ptr type = decl->abstractType();
  IdentifiedType* idType = dynamic_cast<IdentifiedType*>(type.unsafeData());
  Declaration* idDecl = idType ? idType->declaration(decl->topContext()) : 0;
  if (!idDecl)
    return UN_ID_ERROR.arg(type->toString());

  return compareValues(idDecl->qualifiedIdentifier().toString(), value, "Declaration's identified type");
}
///JSON type: bool
///@returns whether the (function) declaration's isVirtual matches the given value
DeclarationTest(isVirtual)
{
  const QString NOT_A_FUNCTION = "Non-function declaration cannot be virtual.";
  AbstractFunctionDeclaration *absFuncDecl = dynamic_cast<AbstractFunctionDeclaration*>(decl);
  if (!absFuncDecl)
      return NOT_A_FUNCTION;

  return compareValues(absFuncDecl->isVirtual(), value, "Declaration's isVirtual");
}
///JSON type: DeclTestObject
///@returns whether the tests for the function declaration's definition pass
DeclarationTest(definition)
{
  KDevVarLengthArray<IndexedDeclaration> definitions = DUChain::definitions()->definitions(decl->id());
  Declaration *declDef  = 0;
  if (definitions.size())
    declDef = definitions.at(0).declaration();
  return testObject(declDef, value, "Declaration's definition");
}
///JSON type: DeclTestObject
///@returns whether the tests for the function definition's declaration pass
DeclarationTest(declaration)
{
  FunctionDefinition *def = dynamic_cast<FunctionDefinition*>(decl);
  Declaration *defDecl = def->declaration(decl->topContext());
  return testObject(defDecl, value, "Definition's declaration");
}
///JSON type: bool
///@returns whether the declaration's nullity matches the given value
DeclarationTest(null)
{
  return compareValues(decl == 0, value, "Declaration's nullity");
}
///JSON type: bool
///@returns whether the declaration's default parameter matches the given value
DeclarationTest(defaultParameter)
{
  const QString NOT_IN_FUNC_CTXT = "Asked for a default parameter for a declaration outside of a function context.";
  const QString OWNER_NOT_FUNC = "Function context not owned by function declaration (what on earth did you do?).";
  DUContext *context = decl->context();
  if (!context || context->type() != DUContext::Function)
    return NOT_IN_FUNC_CTXT;
  AbstractFunctionDeclaration *funcDecl = dynamic_cast<AbstractFunctionDeclaration*>(context->owner());
  if (!funcDecl)
    return OWNER_NOT_FUNC;
  int argIndex = context->localDeclarations().indexOf(decl);
  return compareValues(funcDecl->defaultParameterForArgument(argIndex).str(), value, "Declaration's default parameter");
}

///JSON type: string
///@returns stringified declaration
DeclarationTest(toString)
{
  return compareValues(decl->toString(), value, "Declaration's toString");
}

///JSON type: string
///@returns stringified declaration
DeclarationTest(range)
{
  auto range = decl->range();
  QString string = QString("[(%1, %2), (%3, %4)]")
    .arg(range.start.line)
    .arg(range.start.column)
    .arg(range.end.line)
    .arg(range.end.column);
  return compareValues(string, value, "Declaration's toString");
}

}

}

#endif //KDEVPLATFORM_JSONDECLARATIONTESTS_H
