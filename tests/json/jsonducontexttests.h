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

#ifndef KDEVPLATFORM_JSONDUCONTEXTTESTS_H
#define KDEVPLATFORM_JSONDUCONTEXTTESTS_H

#include "language/duchain/ducontext.h"
#include "jsontesthelpers.h"

/**
 * JSON Object Specification:
 * FindDeclObject: Mapping of (string) search ids to DeclTestObjects
 * IndexDeclObject: Mapping of (string) declaration position indexes to DeclTestObjects
 * IndexCtxtObject: Mapping of (string) context position indexes to CtxtTestObjects
 *
 * Quick Reference:
 *   findDeclarations : FindDeclObject
 *   declarations : IndexDeclObject
 *   childCount : int
 *   localDeclarationCount : int
 *   type : string
 *   null : bool
 *   owner : DeclTestObject
 *   importedParents : IndexCtxtObject
 */

namespace KDevelop
{

namespace DUContextTests
{

using namespace JsonTestHelpers;

///JSON type: FindDeclObject
///@returns whether each declaration can be found and passes its tests
ContextTest(findDeclarations)
{
  VERIFY_TYPE(QVariantMap);
  QString INVALID_ERROR = "Attempted to test invalid context.";
  QString NOT_FOUND_ERROR = "Could not find declaration \"%1\".";
  QString DECL_ERROR = "Declaration found with \"%1\" did not pass tests.";
  if (!ctxt)
    return INVALID_ERROR;
  QVariantMap findDecls = value.toMap();
  for (QVariantMap::iterator it = findDecls.begin(); it != findDecls.end(); ++it)
  {
    QualifiedIdentifier searchId(it.key());
    QList<Declaration*> ret = ctxt->findDeclarations(searchId, CursorInRevision::invalid());
    if (!ret.size())
      return NOT_FOUND_ERROR.arg(it.key());

    if (!runTests(it.value().toMap(), ret.first()))
      return DECL_ERROR.arg(it.key());
  }
  return SUCCESS;
}
///JSON type: IndexDeclObject
///@returns whether a declaration exists at each index and each declaration passes its tests
ContextTest(declarations)
{
  VERIFY_TYPE(QVariantMap);
  QString INVALID_ERROR = "Attempted to test invalid context.";
  QString NOT_FOUND_ERROR = "No declaration at index \"%1\".";
  QString DECL_ERROR = "Declaration at index \"%1\" did not pass tests.";
  if (!ctxt)
    return INVALID_ERROR;
  QVariantMap findDecls = value.toMap();
  for (QVariantMap::iterator it = findDecls.begin(); it != findDecls.end(); ++it)
  {
    int index = it.key().toInt();
    QVector<Declaration*> decls = ctxt->localDeclarations(0);
    if (decls.size() <= index)
        return NOT_FOUND_ERROR;

    if (!runTests(it.value().toMap(), decls.at(index)))
      return DECL_ERROR.arg(it.key());
  }
  return SUCCESS;
}
///JSON type: int
///@returns whether the number of child contexts matches the given value
ContextTest(childCount)
{
  return compareValues(ctxt->childContexts().size(), value, "Context's child count");
}
///JSON type: int
///@returns whether the number of local declarations matches the given value
ContextTest(localDeclarationCount)
{
  return compareValues(ctxt->localDeclarations().size(), value, "Context's local declaration count");
}
///JSON type: string
///@returns whether the context's type matches the given value
ContextTest(type)
{
  QString contextTypeString;
  switch(ctxt->type())
  {
  case DUContext::Class: contextTypeString = "Class"; break;
  case DUContext::Enum: contextTypeString = "Enum"; break;
  case DUContext::Namespace: contextTypeString = "Namespace"; break;
  case DUContext::Function: contextTypeString = "Function"; break;
  case DUContext::Template: contextTypeString = "Template"; break;
  case DUContext::Global: contextTypeString = "Global"; break;
  case DUContext::Helper: contextTypeString = "Helper"; break;
  case DUContext::Other: contextTypeString = "Other"; break;
  }
  return compareValues(contextTypeString, value, "Context's type");
}
///JSON type: bool
///@returns whether the context's nullity matches the given value
ContextTest(null)
{
  return compareValues(ctxt == 0, value, "Context's nullity");
}

//JSON type: DeclTestObject
///@returns the context's owner
ContextTest(owner)
{
    return testObject(ctxt->owner(), value, "Context's owner");
}

///JSON type: IndexCtxtObject
///@returns whether a context exists at each index and each context passes its tests
ContextTest(importedParents)
{
    VERIFY_TYPE(QVariantMap);
    QString INVALID_ERROR = "Attempted to test invalid context.";
    QString NOT_FOUND_ERROR = "No imported context at index \"%1\".";
    QString CONTEXT_ERROR = "Context at index \"%1\" did not pass tests.";
    if (!ctxt)
        return INVALID_ERROR;
    QVariantMap findDecls = value.toMap();
    for (QVariantMap::iterator it = findDecls.begin(); it != findDecls.end(); ++it)
    {
        int index = it.key().toInt();
        QVector<DUContext::Import> imports = ctxt->importedParentContexts();
        if (imports.size() <= index)
            return NOT_FOUND_ERROR;

        if (!runTests(it.value().toMap(), imports.at(index).context(ctxt->topContext())))
            return CONTEXT_ERROR.arg(it.key());
    }
    return SUCCESS;
}

}

}

#endif //KDEVPLATFORM_JSONDUCONTEXTTESTS_H
