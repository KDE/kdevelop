/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "cmakelistsparser.h"
#include "cmakeast.h"
#include "cmakeprojectvisitor.h"

#include <kdebug.h>

void CMakeFunctionDesc::addArguments( const QStringList& args )
{
    foreach( QString arg, args )
    {
        CMakeFunctionArgument cmakeArg( arg );
        arguments.append( cmakeArg );
    }
}

CMakeListsParser::CMakeListsParser(QObject *parent)
 : QObject(parent)
{
}


CMakeListsParser::~CMakeListsParser()
{
}

bool CMakeListsParser::parseCMakeFile( CMakeAst* root, const QString& fileName )
{
    if ( root == 0 )
        return false;
    cmListFileLexer* lexer = cmListFileLexer_New();
    if ( !lexer )
        return false;
    if ( !cmListFileLexer_SetFileName( lexer, qPrintable( fileName ) ) )
        return false;

    bool parseError = false;
    bool haveNewline = true;
    cmListFileLexer_Token* token;
    while(!parseError && (token = cmListFileLexer_Scan(lexer)))
    {
        if(token->type == cmListFileLexer_Token_Newline)
        {
            haveNewline = true;
        }
        else if(token->type == cmListFileLexer_Token_Identifier)
        {
            if(haveNewline)
            {
                haveNewline = false;
                CMakeFunctionDesc function;
                function.name = token->text;
                function.filePath = fileName;
                function.line = token->line;
                if ( parseCMakeFunction( lexer, function, fileName ) )
                    parseError = false;
                else
                    parseError = true;
            }
            else
                parseError = true;
        }
        else
            parseError = true;
    }

    return parseError;


}

bool CMakeListsParser::parseCMakeFunction( cmListFileLexer* lexer,
                                           CMakeFunctionDesc& func,
                                           const QString fileName )
{
    // Command name has already been parsed.  Read the left paren.
    cmListFileLexer_Token* token;
    if(!(token = cmListFileLexer_Scan(lexer)))
    {
        return false;
    }
    if(token->type != cmListFileLexer_Token_ParenLeft)
    {
        return false;
    }

    // Arguments.
    unsigned long lastLine = cmListFileLexer_GetCurrentLine(lexer);
    while((token = cmListFileLexer_Scan(lexer)))
    {
        if(token->type == cmListFileLexer_Token_ParenRight)
        {
            return true;
        }
        else if(token->type == cmListFileLexer_Token_Identifier ||
                token->type == cmListFileLexer_Token_ArgumentUnquoted)
        {
            CMakeFunctionArgument a( token->text, false, fileName, token->line );
            func.arguments << a;
        }
        else if(token->type == cmListFileLexer_Token_ArgumentQuoted)
        {
            CMakeFunctionArgument a( token->text, true, fileName, token->line );
            func.arguments << a;
        }
        else if(token->type != cmListFileLexer_Token_Newline)
        {
            return false;
        }
        lastLine = cmListFileLexer_GetCurrentLine(lexer);
    }

    return false;

}

ProjectInfo CMakeListsParser::parse( const KUrl& file )
{
    ProjectInfo pi;
    CMakeAst *ast = new CMakeAst;

    kDebug(9032) << "Parsing file: " << file.path() << endl;

    if ( !CMakeListsParser::parseCMakeFile( ast, file.toLocalFile() ) )
    {
        pi = parseProject( ast );
    } else {
        //FIXME: Put here the error.
	kDebug(9032) << "Parsing error." << endl;
    }

    if(!pi.name.isEmpty())
            kDebug(9032) << "Parsed: " << file << endl;

    delete ast;
    return pi;
}

ProjectInfo CMakeListsParser::parseProject( const CMakeAst* ast )
{
    ProjectInfo pi;
    if ( ast )
    {
        CMakeProjectVisitor v;
	ast->accept(&v);
    }
    return pi;
}

FolderInfo CMakeListsParser::parseFolder( const CMakeAst*  )
{
    FolderInfo mainInfo;
    if ( false )
    {

    }
    return mainInfo;
}

TargetInfo CMakeListsParser::parseTarget( const CMakeAst* )
{
    TargetInfo ti;
    if ( false )
    {

    }

    return ti;
}

