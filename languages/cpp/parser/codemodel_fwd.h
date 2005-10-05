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
class TypeInfo;

typedef KDevSharedPtr<_ArgumentModelItem> ArgumentModelItem;
typedef KDevSharedPtr<_ClassModelItem> ClassModelItem;
typedef KDevSharedPtr<_CodeModelItem> CodeModelItem;
typedef KDevSharedPtr<_EnumModelItem> EnumModelItem;
typedef KDevSharedPtr<_EnumeratorModelItem> EnumeratorModelItem;
typedef KDevSharedPtr<_FileModelItem> FileModelItem;
typedef KDevSharedPtr<_FunctionDefinitionModelItem> FunctionDefinitionModelItem;
typedef KDevSharedPtr<_FunctionModelItem> FunctionModelItem;
typedef KDevSharedPtr<_NamespaceModelItem> NamespaceModelItem;
typedef KDevSharedPtr<_ScopeModelItem> ScopeModelItem;
typedef KDevSharedPtr<_TemplateModelItem> TemplateModelItem;
typedef KDevSharedPtr<_TemplateParameterModelItem> TemplateParameterModelItem;
typedef KDevSharedPtr<_TypeAliasModelItem> TypeAliasModelItem;
typedef KDevSharedPtr<_VariableModelItem> VariableModelItem;
typedef KDevSharedPtr<_MemberModelItem> MemberModelItem;

typedef QList<ArgumentModelItem> ArgumentList;
typedef QList<ClassModelItem> ClassList;
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
