/* This file is part of KDevelop
    Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2003-2004 Alexander Dymo <cloudtemple@mksat.net>

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
;

#if 0
template <class Pred>
void findFunctionDefinitions( Pred pred, FunctionDefinitionList & lst )
{
    FileList fileList = m_store->fileList();
    for( FileList::Iterator it=fileList.begin(); it!=fileList.end(); ++it )
	findFunctionDefinitions( pred, model_cast<NamespaceDom>(*it), lst );
}
#endif

template <class Pred> void findFunctionDeclarations( Pred pred, const FileList& fileList, FunctionList & lst );
template <class Pred> void findFunctionDeclarations( Pred pred, const NamespaceDom& ns, FunctionList & lst );
template <class Pred> void findFunctionDeclarations( Pred pred, const NamespaceList& namespaceList, FunctionList & lst );
template <class Pred> void findFunctionDeclarations( Pred pred, const ClassList& classList, FunctionList & lst );
template <class Pred> void findFunctionDeclarations( Pred pred, const FunctionList& functionDefinitionList, FunctionList & lst );
template <class Pred> void findFunctionDeclarations( Pred pred, const ClassDom& klass, FunctionList & lst );
template <class Pred> void findFunctionDeclarations( Pred pred, const FunctionDefinitionDom& fun, FunctionList & lst );
template <class Pred> void findFunctionDeclarations( Pred pred, FunctionList & lst );


template <class Pred>
void findFunctionDeclarations( Pred pred, const FileList& fileList, FunctionList & lst )
{
    for( FileList::ConstIterator it=fileList.begin(); it!=fileList.end(); ++it )
	findFunctionDeclarations( pred, model_cast<NamespaceDom>(*it), lst );
}

template <class Pred>
void findFunctionDeclarations( Pred pred, const NamespaceDom& ns, FunctionList & lst )
{
    findFunctionDeclarations( pred, ns->namespaceList(), lst );
    findFunctionDeclarations( pred, ns->classList(), lst );
    findFunctionDeclarations( pred, ns->functionList(), lst );
}

template <class Pred>
void findFunctionDeclarations( Pred pred, const NamespaceList& namespaceList, FunctionList & lst )
{
    for( NamespaceList::ConstIterator it=namespaceList.begin(); it!=namespaceList.end(); ++it )
	findFunctionDeclarations( pred, *it, lst );
}

template <class Pred>
void findFunctionDeclarations( Pred pred, const ClassList& classList, FunctionList & lst )
{
    for( ClassList::ConstIterator it=classList.begin(); it!=classList.end(); ++it )
	findFunctionDeclarations( pred, *it, lst );
}

template <class Pred>
void findFunctionDeclarations( Pred pred, const FunctionList& functionList, FunctionList & lst )
{
    for( FunctionList::ConstIterator it=functionList.begin(); it!=functionList.end(); ++it )
	findFunctionDeclarations( pred, *it, lst );
}

template <class Pred>
void findFunctionDeclarations( Pred pred, const ClassDom& klass, FunctionList & lst )
{
    findFunctionDeclarations( pred, klass->classList(), lst );
    findFunctionDeclarations( pred, klass->functionList(), lst );
}

template <class Pred>
void findFunctionDeclarations( Pred pred, const FunctionDom& fun, FunctionList & lst )
{
    if( pred(fun) )
	lst << fun;
}


struct Scope{
    ClassDom klass;
    NamespaceDom ns;
};

struct AllFunctions{
    QMap<FunctionDom, Scope> relations;
    FunctionList functionList;
};
struct AllFunctionDefinitions{
    QMap<FunctionDefinitionDom, Scope> relations;
    FunctionDefinitionList functionList;
};

namespace Functions{
void processClasses(FunctionList &list, const ClassDom dom);
void processNamespaces(FunctionList &list, const NamespaceDom dom);
void processClasses(FunctionList &list, const ClassDom dom, QMap<FunctionDom, Scope> &relations);
void processClasses(FunctionList &list, const ClassDom dom, QMap<FunctionDom, Scope> &relations, const NamespaceDom &nsdom);
void processNamespaces(FunctionList &list, const NamespaceDom dom, QMap<FunctionDom, Scope> &relations);
}
namespace FunctionDefinitions{
void processClasses(FunctionDefinitionList &list, const ClassDom dom);
void processNamespaces(FunctionDefinitionList &list, const NamespaceDom dom);
void processClasses(FunctionDefinitionList &list, const ClassDom dom, QMap<FunctionDefinitionDom, Scope> &relations);
void processClasses(FunctionDefinitionList &list, const ClassDom dom, QMap<FunctionDefinitionDom, Scope> &relations, const NamespaceDom &nsdom);
void processNamespaces(FunctionDefinitionList &list, const NamespaceDom dom, QMap<FunctionDefinitionDom, Scope> &relations);
}

/**
 * Compare a declaration and a defintion of a function.
 * @param dec declaration
 * @param def definition
 * @return true, if dec is the declaration of the functiondefinition def, false otherwise
 * @author Jonas Jacobi <j.jacobi@gmx.de>
 */
bool compareDeclarationToDefinition(const FunctionDom& dec, const FunctionDefinitionDom& def);

/**
 * Predicate for use with findFunctionDefintions. Searches for a defintion matching a declaration.
 * @author Jonas Jacobi
 */
class PredDefinitionMatchesDeclaration{
public:
	PredDefinitionMatchesDeclaration(const FunctionDom& func) : m_declaration(func){};
	bool operator() (const FunctionDefinitionDom& def) const
	{
		return compareDeclarationToDefinition(m_declaration, def);
	}
	
private:
	const FunctionDom m_declaration;
};

FunctionList allFunctions(const FileDom &dom);
AllFunctions allFunctionsDetailed(const FileDom &dom);
AllFunctionDefinitions allFunctionDefinitionsDetailed(const FileDom &dom);
}

#endif // __CODEMODEL_UTILS_H
