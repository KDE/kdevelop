/* This file is part of KDevelop
    Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>

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

#ifndef CODEMODEL_FWD_H
#define CODEMODEL_FWD_H

#include "kdevsharedptr.h"

#include <QtCore/QList>

// forward declarations
class CodeModel;
class _ArgumentModelItem;
class _ClassModelItem;
class _CodeModelItem;
class _EnumModelItem;
class _EnumeratorModelItem;
class _FileModelItem;
class _FunctionDefinitionModelItem;
class _FunctionModelItem;
class _NamespaceModelItem;
class _ScopeModelItem;
class _TemplateModelItem;
class _TemplateParameterModelItem;
class _TypeAliasModelItem;
class _VariableModelItem;
class _MemberModelItem;

typedef KDevelop::SharedPtr<_ArgumentModelItem> ArgumentModelItem;
typedef KDevelop::SharedPtr<_ClassModelItem> ClassModelItem;
typedef KDevelop::SharedPtr<_CodeModelItem> CodeModelItem;
typedef KDevelop::SharedPtr<_EnumModelItem> EnumModelItem;
typedef KDevelop::SharedPtr<_EnumeratorModelItem> EnumeratorModelItem;
typedef KDevelop::SharedPtr<_FileModelItem> FileModelItem;
typedef KDevelop::SharedPtr<_FunctionDefinitionModelItem> FunctionDefinitionModelItem;
typedef KDevelop::SharedPtr<_FunctionModelItem> FunctionModelItem;
typedef KDevelop::SharedPtr<_NamespaceModelItem> NamespaceModelItem;
typedef KDevelop::SharedPtr<_ScopeModelItem> ScopeModelItem;
typedef KDevelop::SharedPtr<_TemplateModelItem> TemplateModelItem;
typedef KDevelop::SharedPtr<_TemplateParameterModelItem> TemplateParameterModelItem;
typedef KDevelop::SharedPtr<_TypeAliasModelItem> TypeAliasModelItem;
typedef KDevelop::SharedPtr<_VariableModelItem> VariableModelItem;
typedef KDevelop::SharedPtr<_MemberModelItem> MemberModelItem;

typedef QList<ArgumentModelItem> ArgumentList;
typedef QList<ClassModelItem> ClassList;
typedef QList<CodeModelItem> CodeList;
typedef QList<CodeModelItem> ItemList;
typedef QList<EnumModelItem> EnumList;
typedef QList<EnumeratorModelItem> EnumeratorList;
typedef QList<FileModelItem> FileList;
typedef QList<FunctionDefinitionModelItem> FunctionDefinitionList;
typedef QList<FunctionModelItem> FunctionList;
typedef QList<NamespaceModelItem> NamespaceList;
typedef QList<ScopeModelItem> ScopeList;
typedef QList<TemplateModelItem> TemplateList;
typedef QList<TemplateParameterModelItem> TemplateParameterList;
typedef QList<TypeAliasModelItem> TypeAliasList;
typedef QList<VariableModelItem> VariableList;
typedef QList<MemberModelItem> MemberList;

#endif // CODEMODEL_FWD_H
