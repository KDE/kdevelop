/* This file is part of KDevelop
    Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2003-2004 Alexander Dymo <adymo@kdevelop.org>
    Copyright (C) 2004 Jonas Jacobi<j.jacobi@gmx.de>

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
#ifndef CODEMODEL_UTILS_H
#define CODEMODEL_UTILS_H

#include "codemodel.h"

/**
@file codemodel_utils.h
Utility functions and classes for the CodeModel.
*/

/**
@class Pred
The predicate. 
Pred is not a real class, it is only a template parameter used in @ref CodeModelUtils functions.

<b>How to create the predicate:</b>@n
Predicate is simply a class that have 
@code bool operator() (predicateArgument) @endcode.
The return value of that operator is the result of a predicate.

For example we want to find all function definitions with a particular name.
We can use @ref CodeModelUtils::findFunctionDefinitions functions that require
you to write a predicate for function definition DOM's.
This can be done with following code:
@code
class MyPred{
public:
    MyPred(const QString &name): m_name(name) {}

    bool operator() (const FunctionDefinitionDom &def) const
    {
        return def->name() == m_name;
    }
    
private:
    QString m_name;
};
@endcode
*/


/**Namespace which contains utility functions and classes for the CodeModel.*/
namespace CodeModelUtils
{

/**Finds function definitions which match given predicate in files. 
    
Predicate can be considered as a condition. If it is true then the function definition is
added to the result list otherwise it is skipped.
@see Pred class documentation for a detailed description on how to create and use predicates.
    
@param pred Predicate which is applied to a function definition before it is returned.
@param fileList The list of files to find function definitions in.
@param lst The reference to a list of function definitions. Will be filled by this function.*/
template <class Pred> void findFunctionDefinitions( Pred pred, const FileList& fileList, FunctionDefinitionList & lst );

/**Finds function definitions which match given predicate in the namespace. 
    
Predicate can be considered as a condition. If it is true then the function definition is
added to the result list otherwise it is skipped.
@see Pred class documentation for a detailed description on how to create and use predicates.
    
@param pred Predicate which is applied to a function definition before it is returned.
@param ns The namespace to find function definitions in.
@param lst The reference to a list of function definitions. Will be filled by this function.*/
template <class Pred> void findFunctionDefinitions( Pred pred, const NamespaceDom& ns, FunctionDefinitionList & lst );

/**Finds function definitions which match given predicate in namespaces. 
    
Predicate can be considered as a condition. If it is true then the function definition is
added to the result list otherwise it is skipped.
@see Pred class documentation for a detailed description on how to create and use predicates.
    
@param pred Predicate which is applied to a function definition before it is returned.
@param namespaceList The list of namespaces to find function definitions in.
@param lst The reference to a list of function definitions. Will be filled by this function.*/
template <class Pred> void findFunctionDefinitions( Pred pred, const NamespaceList& namespaceList, FunctionDefinitionList & lst );

/**Finds function definitions which match given predicate in classes. 
    
Predicate can be considered as a condition. If it is true then the function definition is
added to the result list otherwise it is skipped.
@see Pred class documentation for a detailed description on how to create and use predicates.
    
@param pred Predicate which is applied to a function definition before it is returned.
@param classList The list of classes to find function definitions in.
@param lst The reference to a list of function definitions. Will be filled by this function.*/
template <class Pred> void findFunctionDefinitions( Pred pred, const ClassList& classList, FunctionDefinitionList & lst );

/**Finds function definitions which match given predicate in the list of function definitions. 
    
Predicate can be considered as a condition. If it is true then the function definition is
added to the result list otherwise it is skipped.
@see Pred class documentation for a detailed description on how to create and use predicates.
    
@param pred Predicate which is applied to a function definition before it is returned.
@param functionDefinitionList The list of function definitions to find function definitions in.
@param lst The reference to a list of function definitions. Will be filled by this function.*/
template <class Pred> void findFunctionDefinitions( Pred pred, const FunctionDefinitionList& functionDefinitionList, FunctionDefinitionList & lst );

/**Finds function definitions which match given predicate in the class. 
    
Predicate can be considered as a condition. If it is true then the function definition is
added to the result list otherwise it is skipped.
@see Pred class documentation for a detailed description on how to create and use predicates.
    
@param pred Predicate which is applied to a function definition before it is returned.
@param klass The class to find function definitions in.
@param lst The reference to a list of function definitions. Will be filled by this function.*/
template <class Pred> void findFunctionDefinitions( Pred pred, const ClassDom& klass, FunctionDefinitionList & lst );

/**Applies a predicate to a function definition. 
    
Predicate can be considered as a condition. If it is true then the function definition is
added to the result list otherwise it is skipped.
@see Pred class documentation for a detailed description on how to create and use predicates.
    
@param pred Predicate which is applied to a function definition before it is returned.
@param fun The function definition.
@param lst The reference to a list of function definitions. Will be filled by this function.*/
template <class Pred> void findFunctionDefinitions( Pred pred, const FunctionDefinitionDom& fun, FunctionDefinitionList & lst );

/**Finds function declarations which match given predicate in files. 
    
Predicate can be considered as a condition. If it is true then the function declaration is
added to the result list otherwise it is skipped.
@see Pred class documentation for a detailed description on how to create and use predicates.
    
@param pred Predicate which is applied to a function declaration before it is returned.
@param fileList The list of files to find function declarations in.
@param lst The reference to a list of function declarations. Will be filled by this function.*/
template <class Pred> void findFunctionDeclarations( Pred pred, const FileList& fileList, FunctionList & lst );

/**Finds function declarations which match given predicate in the namespace. 
    
Predicate can be considered as a condition. If it is true then the function declaration is
added to the result list otherwise it is skipped.
@see Pred class documentation for a detailed description on how to create and use predicates.
    
@param pred Predicate which is applied to a function declaration before it is returned.
@param ns The namespace to find function declarations in.
@param lst The reference to a list of function declarations. Will be filled by this function.*/
template <class Pred> void findFunctionDeclarations( Pred pred, const NamespaceDom& ns, FunctionList & lst );

/**Finds function declarations which match given predicate in namespaces. 
    
Predicate can be considered as a condition. If it is true then the function declaration is
added to the result list otherwise it is skipped.
@see Pred class documentation for a detailed description on how to create and use predicates.
    
@param pred Predicate which is applied to a function declaration before it is returned.
@param namespaceList The list of namespaces to find function declarations in.
@param lst The reference to a list of function declarations. Will be filled by this function.*/
template <class Pred> void findFunctionDeclarations( Pred pred, const NamespaceList& namespaceList, FunctionList & lst );

/**Finds function declarations which match given predicate in classes. 
    
Predicate can be considered as a condition. If it is true then the function declaration is
added to the result list otherwise it is skipped.
@see Pred class documentation for a detailed description on how to create and use predicates.
    
@param pred Predicate which is applied to a function declaration before it is returned.
@param classList The list of classes to find function declarations in.
@param lst The reference to a list of function declarations. Will be filled by this function.*/
template <class Pred> void findFunctionDeclarations( Pred pred, const ClassList& classList, FunctionList & lst );

/**Finds function declarations which match given predicate in the list of function declarations. 
    
Predicate can be considered as a condition. If it is true then the function declaration is
added to the result list otherwise it is skipped.
@see Pred class documentation for a detailed description on how to create and use predicates.
    
@param pred Predicate which is applied to a function declaration before it is returned.
@param functionList The list of function declarations to find function declarations in.
@param lst The reference to a list of function declarations. Will be filled by this function.*/
template <class Pred> void findFunctionDeclarations( Pred pred, const FunctionList& functionList, FunctionList & lst );

/**Finds function declarations which match given predicate in the class. 
    
Predicate can be considered as a condition. If it is true then the function declaration is
added to the result list otherwise it is skipped.
@see Pred class documentation for a detailed description on how to create and use predicates.
    
@param pred Predicate which is applied to a function declaration before it is returned.
@param klass The class to find function declarations in.
@param lst The reference to a list of function declarations. Will be filled by this function.*/
template <class Pred> void findFunctionDeclarations( Pred pred, const ClassDom& klass, FunctionList & lst );

/**Applies a predicate to a function declaration. 
    
Predicate can be considered as a condition. If it is true then the function declaration is
added to the result list otherwise it is skipped.
@see Pred class documentation for a detailed description on how to create and use predicates.
    
@param pred Predicate which is applied to a function declaration before it is returned.
@param fun The function declaration.
@param lst The reference to a list of function declarations. Will be filled by this function.*/
template <class Pred> void findFunctionDeclarations( Pred pred, const FunctionDom& fun, FunctionList & lst );


//implementations of function templates defined above:

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

/**
 * Find a class by its position in a file(position inside the part of the file, where the class is declared).
 * In the case of nested classes the innermost class which is declared at/around the provided position.
 * @param nameSpace A namespace to search for the class
 * @param line a linenumber inside the class declaration
 * @param col the colum of line.
 * @return The innermost class, which is declared at/around position defined with line / col, or 0 if no class is found
 * @author Jonas Jacobi <j.jacobi@gmx.de>
 */
ClassDom findClassByPosition( NamespaceModel* nameSpace, int line, int col );

/**
 * Same as above, just searches inside a class instead of a namespace. 
 */
ClassDom findClassByPosition( ClassModel* aClass, int line, int col );

/**
 * Finds the last occurrence (line of file wise) of a method inside a class declaration with specific access specificer.
 * This can be used e.g. to find a position to new methods to the class
 * @param aClass class to search for method
 * @param access the access specifier with which methods are searched for
 * @return The last line a Method with access specifier access is found, 
 * or -1 if no method with that access specifier was found
 * @author Jonas Jacobi <j.jacobi@gmx.de>
 */
int findLastMethodLine( ClassDom aClass, CodeModelItem::Access access );

/**
 * Same as above, but finds a membervariable instead of a method
 */
int findLastVariableLine( ClassDom aClass, CodeModelItem::Access access );
	
/**
 * Get the string representation of an accesss pecifier
 * @param access access specifier to get a string representation of.
 * @return string representation of access (e.g. "public")
 * @author Jonas Jacobi <j.jacobi@gmx.de>
 */
QString accessSpecifierToString( CodeModelItem::Access access );

}

#endif
