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
#include <language/duchain/classmemberdeclaration.h>
#include "jsontesthelpers.h"

/**
 * JSON Object Specification:
 *   DeclTestObject: Mapping of (string) declaration test names to values
 *   TypeTestObject: Mapping of (string) type test names to values
 *   CtxtTestObject: Mapping of (string) context test names to values
 *
 * Quick Reference:
 *   useCount : int
 *   useRanges : string array
 *   identifier : string
 *   qualifiedIdentifier : string
 *   internalContext : CtxtTestObject
 *   internalFunctionContext : CtxtTestObject
 *   type : TypeTestObject
 *   unaliasedType : TypeTestObject
 *   targetType : TypeTestObject
 *   returnType : TypeTestObject
 *   isAbstract : bool
 *   isVirtual : bool
 *   isStatic : bool
 *   declaration : DeclTestObject
 *   definition : DeclTestObject
 *   identifiedTypeDeclaration : DeclTestObject
 *   null : bool
 *   defaultParameter : string
 *   toString : string
 *   range : string
 *   kind : string
 *   isDeprecated : bool
 *   isDefinition : bool
 */

namespace KDevelop
{
  template<>
  QString TestSuite<Declaration*>::objectInformation(Declaration *decl)
  {
    VERIFY_NOT_NULL(decl);
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
///JSON type: string array
///@returns whether the declaration's ranges match the given value
DeclarationTest(useRanges)
{
  QStringList ranges;
  foreach(const QList<RangeInRevision>& useRanges, decl->uses()) {
    foreach(const RangeInRevision &range, useRanges) {
      ranges << rangeStr(range);
    }
  }
  const QStringList testValues = value.toStringList();
  return ranges == testValues ? SUCCESS
    : QString("Declaration's use ranges (\"%1\") don't match test data (\"%2\").").arg(ranges.join(", ")).arg(testValues.join(", "));
}
///JSON type: string
///@returns whether the declaration's identifier matches the given value
DeclarationTest(identifier)
{
  VERIFY_NOT_NULL(decl);
  return compareValues(decl->identifier().toString(), value, "Declaration's identifier");
}
///JSON type: string
///@returns whether the declaration's qualified identifier matches the given value
DeclarationTest(qualifiedIdentifier)
{
  VERIFY_NOT_NULL(decl);
  return compareValues(decl->qualifiedIdentifier().toString(), value, "Declaration's qualified identifier");
}
///JSON type: CtxtTestObject
///@returns whether the tests for the declaration's internal context pass
DeclarationTest(internalContext)
{
  VERIFY_NOT_NULL(decl);
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
  VERIFY_NOT_NULL(decl);
  return testObject(decl->abstractType(), value, "Declaration's type");
}
///JSON type: TypeTestObject
///@returns whether the tests for the declaration's unaliased type pass (TypeUtils::unaliasedType)
DeclarationTest(unaliasedType)
{
  VERIFY_NOT_NULL(decl);
  return testObject(TypeUtils::unAliasedType(decl->abstractType()), value, "Declaration's unaliased type");
}
///JSON type: TypeTestObject
///@returns whether the tests for the declaration's target type pass (TypeUtils::targetType)
DeclarationTest(targetType)
{
  VERIFY_NOT_NULL(decl);
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
///JSON type: DeclTestObject
///@returns The IdentifiedType's declaration
DeclarationTest(identifiedTypeDeclaration)
{
  const QString UN_ID_ERROR = "Unable to identify declaration of type \"%1\".";
  AbstractType::Ptr type = TypeUtils::targetType(decl->abstractType(), decl->topContext());
  IdentifiedType* idType = dynamic_cast<IdentifiedType*>(type.data());
  Declaration* idDecl = idType ? idType->declaration(decl->topContext()) : 0;
  if (!idDecl)
    return UN_ID_ERROR.arg(type->toString());

  return testObject(idDecl, value, "IdentifiedType's declaration");
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

///JSON type: bool
///@returns whether the (function) declaration's isAbstract matches the given value
DeclarationTest(isAbstract)
{
  const QString NOT_A_FUNCTION = "Non-class-member declaration cannot be abstract.";
  auto *absFuncDecl = dynamic_cast<ClassMemberDeclaration*>(decl);
  if (!absFuncDecl)
      return NOT_A_FUNCTION;

  return compareValues(absFuncDecl->isAbstract(), value, "Declaration's isAbstract");
}
///JSON type: bool
///@returns whether the (class-member) declaration's isStatic matches the given value
DeclarationTest(isStatic)
{
  const QString NOT_A_MEMBER = "Non-class-member declaration cannot be static.";
  auto memberDecl = dynamic_cast<ClassMemberDeclaration*>(decl);
  if (!memberDecl)
      return NOT_A_MEMBER;

  return compareValues(memberDecl->isStatic(), value, "Declaration's isStatic");
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
  Declaration *defDecl = nullptr;
  if (def)
    defDecl = def->declaration(decl->topContext());
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
  VERIFY_NOT_NULL(decl);
  return compareValues(decl->toString(), value, "Declaration's toString");
}

///JSON type: string
///@returns stringified declaration range
DeclarationTest(range)
{
  VERIFY_NOT_NULL(decl);
  return compareValues(rangeStr(decl->range()), value, "Declaration's range");
}

///JSON type: string
///@returns stringified declaration kind
DeclarationTest(kind)
{
  VERIFY_NOT_NULL(decl);
  QString kind;
  switch (decl->kind()) {
    case KDevelop::Declaration::Alias:
      kind = "Alias";
      break;
    case KDevelop::Declaration::Import:
      kind = "Import";
      break;
    case KDevelop::Declaration::Instance:
      kind = "Instance";
      break;
    case KDevelop::Declaration::Namespace:
      kind = "Namespace";
      break;
    case KDevelop::Declaration::NamespaceAlias:
      kind = "NamespaceAlias";
      break;
    case KDevelop::Declaration::Type:
      kind = "Type";
      break;
  }
  return compareValues(kind, value, "Declaration's kind");
}

///JSON type: bool
///@returns whether the declaration's isDeprecated matches the given value
DeclarationTest(isDeprecated)
{
  VERIFY_NOT_NULL(decl);
  return compareValues(decl->isDeprecated(), value, "Declaration's isDeprecated");
}

///JSON type: bool
///@returns whether the declaration's isDefinition matches the given value
DeclarationTest(isDefinition)
{
  VERIFY_NOT_NULL(decl);
  return compareValues(decl->isDefinition(), value, "Declaration's isDefinition");
}

}

}

#endif //KDEVPLATFORM_JSONDECLARATIONTESTS_H
