/* This file is part of KDevelop
    Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2003 Alexander Dymo <adymo@kdevelop.org>
    Copyright (C) 2004 Jonas Jacobi <j.jacobi@gmx.de>

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

namespace Functions
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

void processNamespaces( FunctionList & list, const NamespaceDom dom, QMap< FunctionDom, Scope > & relations )
{
    const NamespaceList nslist = dom->namespaceList();
    for (NamespaceList::ConstIterator it = nslist.begin(); it != nslist.end(); ++it)
    {
        processNamespaces(list, *it, relations);
    }
    const ClassList cllist = dom->classList();
    for (ClassList::ConstIterator it = cllist.begin(); it != cllist.end(); ++it)
    {
        processClasses(list, *it, relations, dom);
    }

    const FunctionList fnlist = dom->functionList();
    for (FunctionList::ConstIterator it = fnlist.begin(); it != fnlist.end(); ++it)
    {
        list.append(*it);
        relations[*it].ns = dom;
    }
}

void processClasses( FunctionList & list, const ClassDom dom, QMap< FunctionDom, Scope > & relations )
{
    const ClassList cllist = dom->classList();
    for (ClassList::ConstIterator it = cllist.begin(); it != cllist.end(); ++it)
    {
        processClasses(list, *it, relations);
    }

    const FunctionList fnlist = dom->functionList();
    for (FunctionList::ConstIterator it = fnlist.begin(); it != fnlist.end(); ++it)
    {
        list.append(*it);
        relations[*it].klass = dom;
    }
}

void processClasses( FunctionList & list, const ClassDom dom, QMap< FunctionDom, Scope > & relations, const NamespaceDom & nsdom )
{
    const ClassList cllist = dom->classList();
    for (ClassList::ConstIterator it = cllist.begin(); it != cllist.end(); ++it)
    {
        processClasses(list, *it, relations, nsdom);
    }

    const FunctionList fnlist = dom->functionList();
    for (FunctionList::ConstIterator it = fnlist.begin(); it != fnlist.end(); ++it)
    {
        list.append(*it);
        relations[*it].klass = dom;
        relations[*it].ns = nsdom;
    }
}

} // end of Functions namespace



namespace FunctionDefinitions
{

void processClasses(FunctionDefinitionList &list, const ClassDom dom)
{
    const ClassList cllist = dom->classList();
    for (ClassList::ConstIterator it = cllist.begin(); it != cllist.end(); ++it)
    {
        processClasses(list, *it);
    }

    const FunctionDefinitionList fnlist = dom->functionDefinitionList();
    for (FunctionDefinitionList::ConstIterator it = fnlist.begin(); it != fnlist.end(); ++it)
    {
        list.append(*it);
    }
}

void processNamespaces(FunctionDefinitionList &list, const NamespaceDom dom)
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

    const FunctionDefinitionList fnlist = dom->functionDefinitionList();
    for (FunctionDefinitionList::ConstIterator it = fnlist.begin(); it != fnlist.end(); ++it)
    {
        list.append(*it);
    }
}

void processNamespaces( FunctionDefinitionList & list, const NamespaceDom dom, QMap< FunctionDefinitionDom, Scope > & relations )
{
    const NamespaceList nslist = dom->namespaceList();
    for (NamespaceList::ConstIterator it = nslist.begin(); it != nslist.end(); ++it)
    {
        processNamespaces(list, *it, relations);
    }
    const ClassList cllist = dom->classList();
    for (ClassList::ConstIterator it = cllist.begin(); it != cllist.end(); ++it)
    {
        processClasses(list, *it, relations, dom);
    }

    const FunctionDefinitionList fnlist = dom->functionDefinitionList();
    for (FunctionDefinitionList::ConstIterator it = fnlist.begin(); it != fnlist.end(); ++it)
    {
        list.append(*it);
        relations[*it].ns = dom;
    }
}

void processClasses( FunctionDefinitionList & list, const ClassDom dom, QMap< FunctionDefinitionDom, Scope > & relations )
{
    const ClassList cllist = dom->classList();
    for (ClassList::ConstIterator it = cllist.begin(); it != cllist.end(); ++it)
    {
        processClasses(list, *it, relations);
    }

    const FunctionDefinitionList fnlist = dom->functionDefinitionList();
    for (FunctionDefinitionList::ConstIterator it = fnlist.begin(); it != fnlist.end(); ++it)
    {
        list.append(*it);
        relations[*it].klass = dom;
    }
}

void processClasses( FunctionDefinitionList & list, const ClassDom dom, QMap< FunctionDefinitionDom, Scope > & relations, const NamespaceDom & nsdom )
{
    const ClassList cllist = dom->classList();
    for (ClassList::ConstIterator it = cllist.begin(); it != cllist.end(); ++it)
    {
        processClasses(list, *it, relations, nsdom);
    }

    const FunctionDefinitionList fnlist = dom->functionDefinitionList();
    for (FunctionDefinitionList::ConstIterator it = fnlist.begin(); it != fnlist.end(); ++it)
    {
        list.append(*it);
        relations[*it].klass = dom;
        relations[*it].ns = nsdom;
    }
}

} // end of FunctionDefinitions namespace



FunctionList allFunctions(const FileDom &dom)
{
    using namespace Functions;
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

AllFunctions allFunctionsDetailed( const FileDom & dom )
{
    using namespace Functions;
    AllFunctions list;

    const NamespaceList nslist = dom->namespaceList();
    for (NamespaceList::ConstIterator it = nslist.begin(); it != nslist.end(); ++it)
    {
        processNamespaces(list.functionList, *it, list.relations);
    }

    const ClassList cllist = dom->classList();
    for (ClassList::ConstIterator it = cllist.begin(); it != cllist.end(); ++it)
    {
        processClasses(list.functionList, *it, list.relations);
    }

    const FunctionList fnlist = dom->functionList();
    for (FunctionList::ConstIterator it = fnlist.begin(); it != fnlist.end(); ++it)
    {
        list.functionList.append(*it);
    }

    return list;
}

AllFunctionDefinitions allFunctionDefinitionsDetailed( const FileDom & dom )
{
    using namespace FunctionDefinitions;
    AllFunctionDefinitions list;

    const NamespaceList nslist = dom->namespaceList();
    for (NamespaceList::ConstIterator it = nslist.begin(); it != nslist.end(); ++it)
    {
        processNamespaces(list.functionList, *it, list.relations);
    }

    const ClassList cllist = dom->classList();
    for (ClassList::ConstIterator it = cllist.begin(); it != cllist.end(); ++it)
    {
        processClasses(list.functionList, *it, list.relations);
    }

    const FunctionDefinitionList fnlist = dom->functionDefinitionList();
    for (FunctionDefinitionList::ConstIterator it = fnlist.begin(); it != fnlist.end(); ++it)
    {
        list.functionList.append(*it);
    }

    return list;
}


bool resultTypesFit( const FunctionDom & dec, const FunctionDefinitionDom & def ) {
    if( !def->resultType().contains("::") ) return dec->resultType() == def->resultType();
    QStringList l1 = dec->scope() + QStringList::split("::", dec->resultType() );
    QStringList l2 = QStringList::split("::", def->resultType() );
    
    if( l1.isEmpty() || l2.isEmpty() || l1.back() != l2.back() ) return false;
    
    while( !l1.isEmpty() && !l2.isEmpty() ) {
        if( l1.back() == l2.back() ) {
            l1.pop_back();
            l2.pop_back();
        } else {
            l1.pop_back();
        }
    }
    
    if( l2.isEmpty() ) return true;
}


bool compareDeclarationToDefinition( const FunctionDom & dec, const FunctionDefinitionDom & def )
{
    if (dec->scope() == def->scope() && dec->name() == def->name() && resultTypesFit( dec, def ) && dec->isConstant() == def->isConstant())
	{
		const ArgumentList defList = def->argumentList(), decList = dec->argumentList();
		if (defList.size() != decList.size())
			return false;
		
		size_t n = defList.size();
		for(size_t i = 0; i < n; ++i)
			if (defList[i]->type() != decList[i]->type())
				return false;
		
		return true;
	}
	return false;
}




FunctionList allFunctionsExhaustive(FileDom &dom) {
    PredAmOwner<FunctionDom> ow( dom );
    FunctionList ret;
    
    findFunctionDeclarations( ow, dom->wholeGroup(), ret );
    
    return ret;
}


FunctionDefinitionList allFunctionDefinitionsExhaustive(FileDom &dom) {
    PredAmOwner<FunctionDefinitionDom> ow( dom );
    FunctionDefinitionList ret;
    
    findFunctionDefinitions( ow, dom->wholeGroup(), ret );
    
    return ret;
}



ClassDom findClassByPosition( NamespaceModel* nameSpace, int line, int col )
{
	if (nameSpace == 0)
		return 0;
		
	NamespaceList nsList = nameSpace->namespaceList();
	for (NamespaceList::iterator i = nsList.begin(); i != nsList.end(); ++i)
	{
		ClassDom result = findClassByPosition(*i, line, col);
		if (result != 0)
		return result;
	}
		
	ClassList classes = nameSpace->classList();
	for(ClassList::iterator i = classes.begin(); i != classes.end(); ++i)
	{
		ClassDom result = findClassByPosition( *i, line, col );
		if (result != 0)
		return result;
	}
	
	return 0;
}

ClassDom findClassByPosition( ClassModel* aClass, int line, int col )
{
	if (aClass == 0)
		return 0;
	
	ClassList classes = aClass->classList();
	for(ClassList::iterator i = classes.begin(); i != classes.end(); ++i)
	{
		ClassDom result = findClassByPosition( *i, line, col );
		if (result != 0)
		return result;
	}
		
	int startLine, startCol;
	aClass->getStartPosition(&startLine, &startCol);
	
	if (startLine <= line)
	{
		int endLine, endCol;
		aClass->getEndPosition(&endLine, &endCol);
		if (endLine >= line)
		return (aClass);
	}
	
	return 0;
}

int findLastMethodLine( ClassDom aClass, CodeModelItem::Access access )
{	
	int point = -1;
	
	const FunctionList functionList = aClass->functionList();
	for( FunctionList::ConstIterator it=functionList.begin(); it!=functionList.end(); ++it )
	{
		int funEndLine, funEndColumn;
		(*it)->getEndPosition( &funEndLine, &funEndColumn );
		
		if ((*it)->access() == access && point < funEndLine)
		point = funEndLine;
	}
		
	return point;
}
	
int findLastVariableLine( ClassDom aClass, CodeModelItem::Access access )
{
	int point = -1;
	
	const VariableList varList = aClass->variableList();
	for( VariableList::ConstIterator it= varList.begin(); it!= varList.end(); ++it )
	{
		int varEndLine, varEndColumn;
		(*it)->getEndPosition( &varEndLine, &varEndColumn );
		
		if ((*it)->access() == access && point < varEndLine)
		point = varEndLine;
	}
		
	return point;
}

QString accessSpecifierToString( CodeModelItem::Access access )
{
  switch(access)
  {
	case CodeModelItem::Public: return "public";
	case CodeModelItem::Protected: return "protected";
	case CodeModelItem::Private: return "private";
	default: return "unknown";
  }
}

FunctionDefinitionDom CodeModelHelper::functionDefinitionAt(NamespaceDom ns, int line, int column)
{
    NamespaceList namespaceList = ns->namespaceList();
    NamespaceList::iterator nslEnd = namespaceList.end();

    for (NamespaceList::iterator it=namespaceList.begin(); it!=nslEnd; ++it)
    {
        if (FunctionDefinitionDom def = functionDefinitionAt(*it, line, column))
            return def;
    }

    ClassList classList = ns->classList();
    ClassList::iterator clEnd = classList.end();

    for (ClassList::iterator it=classList.begin(); it!=clEnd; ++it)
    {
        if (FunctionDefinitionDom def = functionDefinitionAt(*it, line, column))
            return def;
    }

    FunctionDefinitionList functionDefinitionList = ns->functionDefinitionList();
    FunctionDefinitionList::iterator fdlEnd = functionDefinitionList.end();

    for (FunctionDefinitionList::iterator it=functionDefinitionList.begin();
            it!=fdlEnd; ++it )
    {
        if (FunctionDefinitionDom def = functionDefinitionAt(*it, line, column))
            return def;
    }

    return FunctionDefinitionDom();
}

FunctionDefinitionDom CodeModelHelper::functionDefinitionAt(ClassDom klass, int line, int column)
{
    ClassList classList = klass->classList();
    ClassList::iterator clEnd = classList.end();

    for (ClassList::iterator it=classList.begin(); it!=clEnd; ++it)
    {
        if (FunctionDefinitionDom def = functionDefinitionAt(*it, line, column))
            return def;
    }

    FunctionDefinitionList functionDefinitionList = klass->functionDefinitionList();
    FunctionDefinitionList::iterator fdlEnd = functionDefinitionList.end();
    for (FunctionDefinitionList::Iterator it=functionDefinitionList.begin();
            it!=fdlEnd; ++it)
    {
        if (FunctionDefinitionDom def = functionDefinitionAt(*it, line, column))
            return def;
    }

    return FunctionDefinitionDom();
}

FunctionDefinitionDom CodeModelHelper::functionDefinitionAt(FunctionDefinitionDom fun, int line, int // column
                                            )
{
    int startLine, startColumn;
    int endLine, endColumn;

    fun->getStartPosition(&startLine, &startColumn);
    fun->getEndPosition(&endLine, &endColumn);

    if (!(line >= startLine && line <= endLine)  || fun->fileName() != m_fileName )
        return FunctionDefinitionDom();

    /*    
    if (line == startLine && column < startColumn)
    return FunctionDefinitionDom();

    if (line == endLine && column > endColumn)
    return FunctionDefinitionDom();*/

    return fun;
}



FunctionDom CodeModelHelper::functionDeclarationAt(NamespaceDom ns, int line, int column)
{
    NamespaceList namespaceList = ns->namespaceList();
    NamespaceList::iterator nsEnd = namespaceList.end();
    for (NamespaceList::iterator it=namespaceList.begin(); it!=nsEnd; ++it)
    {
        if (FunctionDom def = functionDeclarationAt(*it, line, column))
            return def;
    }

    ClassList classList = ns->classList();
    ClassList::iterator clEnd = classList.end();
    for (ClassList::iterator it=classList.begin(); it!=clEnd; ++it)
    {
        if (FunctionDom def = functionDeclarationAt(*it, line, column))
            return def;
    }

    FunctionList functionList = ns->functionList();
    FunctionList::iterator flEnd = functionList.end();
    for (FunctionList::iterator it=functionList.begin();
            it!=flEnd; ++it )
    {
        if (FunctionDom def = functionDeclarationAt(*it, line, column))
            return def;
    }

    return FunctionDom();
}

FunctionDom CodeModelHelper::functionDeclarationAt(ClassDom klass, int line, int column)
{
    ClassList classList = klass->classList();
    ClassList::iterator clEnd = classList.end();
    for (ClassList::iterator it=classList.begin(); it!=clEnd; ++it)
    {
        if (FunctionDom def = functionDeclarationAt(*it, line, column))
            return def;
    }

    FunctionList functionList = klass->functionList();
    FunctionList::iterator flEnd = functionList.end();
    for (FunctionList::Iterator it=functionList.begin();
            it!=flEnd; ++it)
    {
        if (FunctionDom def = functionDeclarationAt(*it, line, column))
            return def;
    }

    return FunctionDom();
}

FunctionDom CodeModelHelper::functionDeclarationAt(FunctionDom fun, int line, int // column
                                    )
{
    int startLine, startColumn;
    int endLine, endColumn;

    fun->getStartPosition(&startLine, &startColumn);
    fun->getEndPosition(&endLine, &endColumn);

    if (!(line >= startLine && line <= endLine) || fun->fileName() != m_fileName )
        return FunctionDom();


    /*    if (line == startLine && column < startColumn)
    return FunctionDom();

    if (line == endLine && column > endColumn)
    return FunctionDom();*/

    return fun;
}        




ClassDom CodeModelHelper::classAt(NamespaceDom ns, int line, int column)
{
    NamespaceList namespaceList = ns->namespaceList();
    NamespaceList::iterator nsEnd = namespaceList.end();
    
    for (NamespaceList::iterator it=namespaceList.begin(); it!=nsEnd; ++it)
    {
        if (ClassDom def = classAt(*it, line, column))
            return def;
    }

    ClassList classList = ns->classList();
    ClassList::iterator clEnd = classList.end();
    for (ClassList::iterator it=classList.begin(); it!=clEnd; ++it)
    {
        if (ClassDom def = classAt(*it, line, column))
            return def;
    }

    return ClassDom();
}

ClassDom CodeModelHelper::classAt(ClassDom klass, int line, int column)
{
    ClassList classList = klass->classList();
    ClassList::iterator clEnd = classList.end();
    for (ClassList::iterator it=classList.begin(); it!=clEnd; ++it)
    {
        if (ClassDom def = classAt(*it, line, column))
            return def;
    }

    int startLine, startColumn;
    int endLine, endColumn;

    klass->getStartPosition(&startLine, &startColumn);
    klass->getEndPosition(&endLine, &endColumn);

    if (!(line >= startLine && line <= endLine)  || klass->fileName() != m_fileName )
        return ClassDom();
    
    return klass;
}


CodeModelHelper::CodeModelHelper( CodeModel* model, FileDom file ) : m_model( model ) {
    if( !file ) return;
    m_files = file->wholeGroup();
    m_fileName = file->name();
}


FunctionDom CodeModelHelper::functionAt( int line, int column, FunctionTypes types ) {
    if( m_files.isEmpty() ) return FunctionDom();
    
    
    FunctionDom ret;
    FileList::iterator it = m_files.begin();
    while( it != m_files.end() ) {
        if( types & Declaration ) {
            ret = functionDeclarationAt(model_cast<NamespaceDom>(*it), line, column);
            if(ret) return ret;
        }
        if( types & Definition ) {
            FunctionDefinitionDom r  = functionDefinitionAt(model_cast<NamespaceDom>(*it), line, column);
            if(r) {
                ret = model_cast<FunctionDom>( r );
                return ret;
            }
        }
        ++it;
    }
    
    return ret;
}

ClassDom CodeModelHelper::classAt( int line, int column ) {
    if( m_files.isEmpty() ) return ClassDom();
    
    ClassDom ret;
    FileList::iterator it = m_files.begin();
    while( it != m_files.end() ) {
        ret = classAt( model_cast<NamespaceDom>(*it), line, column );
        if(ret) return ret;
        ++it;
    }
    
    return ret;
}

}//end of namespace CodeModeUtils
