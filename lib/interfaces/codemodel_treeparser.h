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
#ifndef CODEMODEL_TREEPARSER_H
#define CODEMODEL_TREEPARSER_H

class FileModel;
class NamespaceModel;
class ClassModel;
class FunctionModel;
class FunctionDefinitionModel;
class VariableModel;

class CodeModelTreeParser
{
public:
    CodeModelTreeParser();
    virtual ~CodeModelTreeParser();
    
    virtual void parseCode( const CodeModel* model );
    virtual void parseFile( const FileModel* file );
    virtual void parseNamespace( const NamespaceModel* ns );
    virtual void parseClass( const ClassModel* klass );
    virtual void parseFunction( const FunctionModel* fun );
    virtual void parseFunctionDefinition( const FunctionDefinitionModel* fun );
    virtual void parseVariable( const VariableModel* var );

private:
    CodeModelTreeParser( const CodeModelTreeParser& source );
    void operator = ( const CodeModelTreeParser& source );
};

#endif
