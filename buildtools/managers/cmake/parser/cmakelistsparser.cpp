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
// #include "cmakeprojectvisitor.h"
#include "astfactory.h"

#include <QStack>
#include <KDebug>

void CMakeFunctionDesc::addArguments( const QStringList& args )
{
    foreach( QString arg, args )
    {
        CMakeFunctionArgument cmakeArg( arg );
        arguments.append( cmakeArg );
    }
}

QString CMakeFunctionDesc::writeBack() const
{
    QString output=name+"( ";
    foreach(CMakeFunctionArgument arg, arguments)
    {
        QString o = arg.value;
        if(arg.quoted)
            o='"'+o+'"';
        output += o+' ';
    }
    output += ')';
    return output;
}

CMakeListsParser::CMakeListsParser(QObject *parent)
 : QObject(parent)
{
}


CMakeListsParser::~CMakeListsParser()
{
}

#if 0
enum RecursivityType { No, Yes, ElseIf, End };

RecursivityType recursivity(const QString& functionName)
{
    if(functionName.toUpper()=="IF" || functionName.toUpper()=="WHILE" ||
           functionName.toUpper()=="FOREACH" || functionName.toUpper()=="MACRO")
        return Yes;
    else if(functionName.toUpper()=="ELSE" || functionName.toUpper()=="ELSEIF")
        return ElseIf;
    else if(functionName.toUpper().startsWith("END"))
        return End;
    return No;
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
    QStack<CMakeAst*> parent;
    QStack<IfAst*> parentIf;
    parent.push(root);

    while(!parseError && (token = cmListFileLexer_Scan(lexer)))
    {
        parseError=false;
        if(token->type == cmListFileLexer_Token_Newline)
        {
            parseError=false;
            haveNewline = true;
        }
        else if(token->type == cmListFileLexer_Token_Identifier)
        {
            if(haveNewline)
            {
                CMakeAst* currentParent=parent.top();
                haveNewline = false;
                CMakeFunctionDesc function;
                function.name = token->text;
                function.filePath = fileName;
                function.line = token->line;

                RecursivityType s=recursivity(function.name);
                if(s==End) { //NOTE: We're not checking if ENDIF(<<this>>) is ok.
                    parent.pop();
                    if(function.name.toUpper()=="ENDIF") {
                        parentIf.pop();
                    }
                    parseError = false;
                } else {
                    QString ifWorkaround; //FIXME: Please check it!
                    if(function.name.toUpper().startsWith("ELSE")) {
                        ifWorkaround = function.name;
                        function.name = "IF";
                    }
                    parseError = !parseCMakeFunction( lexer, function, fileName, currentParent );

                    if(parseError) {
                        kDebug(9032) << "Error while parsing: " << function.name;
                    }

                    if(!ifWorkaround.isEmpty())
                        function.name = ifWorkaround;

                    if(!parseError && s!=No) {
                        CMakeAst* lastCreated = currentParent->children().last();
//                         kDebug(9032) << "Lol: " << function.name << s << endl;

                        if(function.name.toUpper()=="IF") {
                            parentIf.append(dynamic_cast<IfAst*>(lastCreated));
                        } else if(function.name.toUpper()=="ELSEIF") {
                            IfAst* ifast = parentIf.top(), *elseif=dynamic_cast<IfAst*>(lastCreated);
                            ifast->conditions().append(elseif->conditions()[0]);
                            lastCreated = ifast;
                        } else if(function.name.toUpper()=="ELSE") {
                            lastCreated = parentIf.top();
                        }

                        CMakeAst* child = new CMakeAst;
                        lastCreated->addChildAst(child);
                        switch(s) {
                            case Yes:
                                parent.push(child);
                                break;
                            case ElseIf: //For else and ifelse
                                parent.pop();
                                parent.push(child);
                                break;
                            case End:
                            case No:
                            default:
                                break;
                        }
                    }
                }
//                 kDebug(9032) << "Parsing: " << function.name << " depth: " << parent.count() << endl;
            }
        }
    }

    return parseError;
}

bool CMakeListsParser::parseCMakeFunction( cmListFileLexer* lexer,
                                           CMakeFunctionDesc& func,
                                           const QString& fileName, CMakeAst *parent )
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
            CMakeAst* newElement = AstFactory::self()->createAst(func.name);
            bool asMacro=false;

            if(!newElement)
            {
                asMacro=true;
                newElement = new MacroCallAst;
            }
            bool err = newElement->parseFunctionInfo(func);
            if(!err)
                kDebug(9032) << "error! found an error while reading " << func.name << " at " << func.line << endl;
            parent->addChildAst(newElement);
//             kDebug(9032) << "Adding: " << func.name << newElement << " as macro :" << asMacro << endl;
            
            return true;
            //FIXME: should return the parseFunctionInfo boolean return value, err
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
#endif

CMakeFileContent CMakeListsParser::readCMakeFile(const QString & fileName)
{
    cmListFileLexer* lexer = cmListFileLexer_New();
    if ( !lexer )
        return CMakeFileContent();
    if ( !cmListFileLexer_SetFileName( lexer, qPrintable( fileName ) ) )
        return CMakeFileContent();

    CMakeFileContent ret;
    bool readError = false, haveNewline = true;
    cmListFileLexer_Token* token;

    while(!readError && (token = cmListFileLexer_Scan(lexer)))
    {
        readError=false;
        if(token->type == cmListFileLexer_Token_Newline)
        {
            readError=false;
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

                readError = !readCMakeFunction( lexer, function, fileName );
                ret.append(function);

                if(readError)
                {
                    kDebug(9032) << "Error while parsing: " << function.name << " at " << function.line << endl;
                }
            }
        }
    }

    return ret;
}

bool CMakeListsParser::readCMakeFunction(cmListFileLexer *lexer, CMakeFunctionDesc &func, const QString & fileName)
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
            //Here we do not convert to an Ast
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

bool CMakeFunctionDesc::operator==(const CMakeFunctionDesc & other) const
{
    if(other.arguments.count()!=arguments.count() || name!=other.name)
        return false;
    
    QList<CMakeFunctionArgument>::const_iterator it=arguments.constBegin();
    QList<CMakeFunctionArgument>::const_iterator itOther=other.arguments.constBegin();
    for(;it!=arguments.constEnd(); ++it, ++itOther)
    {
        if(*it!=*itOther)
            return false;
    }
    return true;
}


