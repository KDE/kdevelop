/* KDevelop CMake Support
 *
 * Copyright 2006 Matt Rogers <mattr@kde.org>
 * Copyright 2008 Aleix Pol <aleixpol@gmail.com>
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

QMap<QChar, QChar> whatToScape()
{
    //Only add those where we're not scaping the next character
    QMap<QChar, QChar> ret;
    ret['n']='\n';
    ret['r']='\r';
    ret['t']='\t';
    return ret;
}

const QMap<QChar, QChar> CMakeFunctionArgument::scapings=whatToScape();

static const QChar scapingChar='\\';
QString CMakeFunctionArgument::unescapeValue(const QString& value)
{
    int firstScape=value.indexOf(scapingChar);
    if (firstScape<0)
    {
        return value;
    }
    
    QString newValue;
    int last=0;
    QMap<QChar, QChar>::const_iterator itEnd = scapings.constEnd();
    for(int i=firstScape; i<value.size()-1 && i>=0; i=value.indexOf(scapingChar, i+2))
    {
        newValue+=value.mid(last, i-last);
        const QChar current=value[i+1];
        QMap<QChar, QChar>::const_iterator it = scapings.constFind(current);
        
        if(it!=itEnd)
            newValue += *it;
        else
            newValue += current;

        last=i+2;
    }
    newValue+=value.mid(last, value.size());
//     qDebug() << "escapiiiiiiiiing" << value << newValue;
    return newValue;
}

void CMakeFunctionDesc::addArguments( const QStringList& args, bool addEvenIfEmpty )
{
    if(addEvenIfEmpty && args.isEmpty())
        arguments += QString();
    else foreach( const QString& arg, args )
    {
        CMakeFunctionArgument cmakeArg( arg );
        arguments.append( cmakeArg );
    }
}

QString CMakeFunctionDesc::writeBack() const
{
    QString output=name+"( ";
    foreach(const CMakeFunctionArgument& arg, arguments)
    {
        QString o = arg.value;
        if(arg.quoted)
            o='"'+o+'"';
        output += o+' ';
    }
    output += ')';
    return output;
}

CMakeFileContent CMakeListsParser::readCMakeFile(const QString & fileName)
{
    cmListFileLexer* lexer = cmListFileLexer_New();
    if ( !lexer )
        return CMakeFileContent();
    if ( !cmListFileLexer_SetFileName( lexer, qPrintable( fileName ) ) ) {
        kDebug(9042) << "cmake read error. could not read " << fileName;
        cmListFileLexer_Delete(lexer);
        return CMakeFileContent();
    }

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
                function.name = QString::fromLocal8Bit(token->text).toLower();
                function.filePath = fileName;
                function.line = token->line;
                function.column = token->column;

                readError = !readCMakeFunction( lexer, function, fileName );
                ret.append(function);

                if(readError)
                {
                    kDebug(9032) << "Error while parsing:" << function.name << "at" << function.line;
                }
            }
        }
    }
    cmListFileLexer_Delete(lexer);

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
    int parenthesis=1;
    while((token = cmListFileLexer_Scan(lexer)))
    {
        switch(token->type)
        {
            case cmListFileLexer_Token_ParenRight:
                parenthesis--;
                if(parenthesis==0) {
                    func.endLine=token->line;
                    func.endColumn=token->column;
                    return true;
                } else if(parenthesis<0)
                    return false;
                else
                    func.arguments << CMakeFunctionArgument( QString::fromLocal8Bit(token->text), false, fileName, token->line, token->column );
                break;
            case cmListFileLexer_Token_ParenLeft:
                parenthesis++;
                func.arguments << CMakeFunctionArgument( QString::fromLocal8Bit(token->text), false, fileName, token->line, token->column );
                break;
            case cmListFileLexer_Token_Identifier:
            case cmListFileLexer_Token_ArgumentUnquoted:
                func.arguments << CMakeFunctionArgument( QString::fromLocal8Bit(token->text), false, fileName, token->line, token->column );
                break;
            case cmListFileLexer_Token_ArgumentQuoted:
                func.arguments << CMakeFunctionArgument( QString::fromLocal8Bit(token->text), true, fileName, token->line, token->column+1 );
                break;
            case cmListFileLexer_Token_Newline:
                break;
            default:
                return false;
        }
    }

    return false;

}

CMakeFunctionDesc::CMakeFunctionDesc(const QString& name, const QStringList& args)
    : name(name)
{
    addArguments(args);
}

CMakeFunctionDesc::CMakeFunctionDesc()
{}

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


/*CMakeFunctionArgument::CMakeFunctionArgument(const CMakeFunctionArgument & r)
    : value(r.value), quoted(r.quoted), filePath(r.filePath), line(r.line), column(r.column)
{
    value=unescapeValue(value);
}*/

CMakeFunctionArgument::CMakeFunctionArgument(const QString & v, bool q, const QString &, quint32 l, quint32 c)
    : value(v), quoted(q)/*, filePath(file)*/, line(l), column(c)
{
    value=unescapeValue(value);
}


