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

#include "codemodel_utils.h"

namespace CodeModelUtils
{

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
