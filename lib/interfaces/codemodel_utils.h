/* This file is part of KDevelop
    Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef _CODEMODEL_UTILS_H_
#define _CODEMODEL_UTILS_H_

#include "codemodel.h"

namespace CodeModelUtils
{

template <class Pred> void findFunctionDefinitions( Pred pred, const FileList& fileList, FunctionDefinitionList & lst );
template <class Pred> void findFunctionDefinitions( Pred pred, const NamespaceDom& ns, FunctionDefinitionList & lst );
template <class Pred> void findFunctionDefinitions( Pred pred, const NamespaceList& namespaceList, FunctionDefinitionList & lst );
template <class Pred> void findFunctionDefinitions( Pred pred, const ClassList& classList, FunctionDefinitionList & lst );
template <class Pred> void findFunctionDefinitions( Pred pred, const FunctionDefinitionList& functionDefinitionList, FunctionDefinitionList & lst );
template <class Pred> void findFunctionDefinitions( Pred pred, const ClassDom& klass, FunctionDefinitionList & lst );
template <class Pred> void findFunctionDefinitions( Pred pred, const FunctionDefinitionDom& fun, FunctionDefinitionList & lst );
template <class Pred> void findFunctionDefinitions( Pred pred, FunctionDefinitionList & lst );


template <class Pred>
void findFunctionDefinitions( Pred pred, const FileList& fileList, FunctionDefinitionList & lst )
{
    for( FileList::ConstIterator it=fileList.begin(); it!=fileList.end(); ++it )
	findFunctionDefinitions( pred, model_cast<NamespaceDom>(*it), lst );
}

template <class Pred>
void findFunctionDefinitions( Pred pred, const NamespaceDom& ns, FunctionDefinitionList & lst )
{
    findFunctionDefinitions( pred, ns->namespaceList(), lst );
    findFunctionDefinitions( pred, ns->classList(), lst );
    findFunctionDefinitions( pred, ns->functionDefinitionList(), lst );
}

template <class Pred>
void findFunctionDefinitions( Pred pred, const NamespaceList& namespaceList, FunctionDefinitionList & lst )
{
    for( NamespaceList::ConstIterator it=namespaceList.begin(); it!=namespaceList.end(); ++it )
	findFunctionDefinitions( pred, *it, lst );
}

template <class Pred>
void findFunctionDefinitions( Pred pred, const ClassList& classList, FunctionDefinitionList & lst )
{
    for( ClassList::ConstIterator it=classList.begin(); it!=classList.end(); ++it )
	findFunctionDefinitions( pred, *it, lst );
}

template <class Pred>
void findFunctionDefinitions( Pred pred, const FunctionDefinitionList& functionDefinitionList, FunctionDefinitionList & lst )
{
    for( FunctionDefinitionList::ConstIterator it=functionDefinitionList.begin(); it!=functionDefinitionList.end(); ++it )
	findFunctionDefinitions( pred, *it, lst );
}

template <class Pred>
void findFunctionDefinitions( Pred pred, const ClassDom& klass, FunctionDefinitionList & lst )
{
    findFunctionDefinitions( pred, klass->classList(), lst );
    findFunctionDefinitions( pred, klass->functionDefinitionList(), lst );
}

template <class Pred>
void findFunctionDefinitions( Pred pred, const FunctionDefinitionDom& fun, FunctionDefinitionList & lst )
{
    if( pred(fun) )
	lst << fun;
}

template <class Pred>
void findFunctionDefinitions( Pred pred, FunctionDefinitionList & lst )
{
    FileList fileList = m_store->fileList();
    for( FileList::Iterator it=fileList.begin(); it!=fileList.end(); ++it )
	findFunctionDefinitions( pred, model_cast<NamespaceDom>(*it), lst );
}


void processClasses(FunctionList &list, const ClassDom dom)
{
    const ClassList cllist = dom->classList();
    for (ClassList::ConstIterator it = cllist.begin(); it != cllist.end(); ++it)
    {
        processClasses(list, *it);
    }

    const FunctionList fnlist = dom->functionList();
    for (FunctionList::ConstIterator it = fnlist.begin(); it != fnlist.end(); ++it)
    {
        list.append(*it);
    }
}

void processNamespaces(FunctionList &list, const NamespaceDom dom)
{
    const NamespaceList nslist = dom->namespaceList();
    for (NamespaceList::ConstIterator it = nslist.begin(); it != nslist.end(); ++it)
    {
        processNamespaces(list, *it);
    }
    const ClassList cllist = dom->classList();
    for (ClassList::ConstIterator it = cllist.begin(); it != cllist.end(); ++it)
    {
        processClasses(list, *it);
    }

    const FunctionList fnlist = dom->functionList();
    for (FunctionList::ConstIterator it = fnlist.begin(); it != fnlist.end(); ++it)
    {
        list.append(*it);
    }
}

FunctionList allFunctions(const FileDom &dom)
{
    FunctionList list;

    const NamespaceList nslist = dom->namespaceList();
    for (NamespaceList::ConstIterator it = nslist.begin(); it != nslist.end(); ++it)
    {
        processNamespaces(list, *it);
    }

    const ClassList cllist = dom->classList();
    for (ClassList::ConstIterator it = cllist.begin(); it != cllist.end(); ++it)
    {
        processClasses(list, *it);
    }

    const FunctionList fnlist = dom->functionList();
    for (FunctionList::ConstIterator it = fnlist.begin(); it != fnlist.end(); ++it)
    {
        list.append(*it);
    }

    return list;
}

}

#endif // __CODEMODEL_UTILS_H
