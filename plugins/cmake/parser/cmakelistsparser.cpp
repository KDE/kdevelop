/*
    SPDX-FileCopyrightText: 2006 Matt Rogers <mattr@kde.org>
    SPDX-FileCopyrightText: 2008 Aleix Pol <aleixpol@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "cmakelistsparser.h"
#include "cmListFileLexer.h"

#include <debug.h>

#include <QDir>

QMap<QChar, QChar> whatToScape()
{
    //Only add those where we're not scaping the next character
    QMap<QChar, QChar> ret{
        {QLatin1Char('n'), QLatin1Char('\n')},
        {QLatin1Char('r'), QLatin1Char('\r')},
        {QLatin1Char('t'), QLatin1Char('\t')},
    };
    return ret;
}

const QMap<QChar, QChar> CMakeFunctionArgument::scapings=whatToScape();

constexpr QChar scapingChar = QLatin1Char('\\');

QString CMakeFunctionArgument::unescapeValue(const QString& value)
{
    int firstScape=value.indexOf(scapingChar);
    if (firstScape<0)
    {
        return value;
    }

    const QStringView valueView = value;
    QString newValue;
    int last=0;
    QMap<QChar, QChar>::const_iterator itEnd = scapings.constEnd();
    for(int i=firstScape; i<value.size()-1 && i>=0; i=value.indexOf(scapingChar, i+2))
    {
        newValue += valueView.sliced(last, i - last);
        const QChar current=value[i+1];
        QMap<QChar, QChar>::const_iterator it = scapings.constFind(current);

        if(it!=itEnd)
            newValue += *it;
        else
            newValue += current;

        last=i+2;
    }
    newValue += valueView.sliced(last);
//     qCDebug(CMAKE) << "escaping" << value << newValue;
    return newValue;
}

void CMakeFunctionDesc::addArguments( const QStringList& args, bool addEvenIfEmpty )
{
    if(addEvenIfEmpty && args.isEmpty())
        arguments += CMakeFunctionArgument();
    else {
        arguments.reserve(arguments.size() + args.size());
        for (const auto& arg : args) {
            CMakeFunctionArgument cmakeArg(arg);
            arguments.append(cmakeArg);
        }
    }
}

QString CMakeFunctionDesc::writeBack() const
{
    QStringList args;
    args.reserve(arguments.size());
    for (const auto& arg : arguments) {
        if (arg.quoted) {
            args.append(QLatin1Char('"') + arg.value + QLatin1Char('"'));
        } else {
            args.append(arg.value);
        }
    }
    return name + QLatin1String("( ") + args.join(QLatin1Char(' ')) + QLatin1String(" )");
}

namespace CMakeListsParser
{

static bool readCMakeFunction( cmListFileLexer* lexer, CMakeFunctionDesc& func);

CMakeFileContent readCMakeFile(const QString & _fileName)
{
    cmListFileLexer* lexer = cmListFileLexer_New();
    if ( !lexer )
        return CMakeFileContent();
    if ( !cmListFileLexer_SetFileName( lexer, qPrintable( _fileName ), nullptr ) ) {
        qCDebug(CMAKE) << "cmake read error. could not read " << _fileName;
        cmListFileLexer_Delete(lexer);
        return CMakeFileContent();
    }

    CMakeFileContent ret;
    QString fileName = QDir::cleanPath(_fileName);

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
                function.name = QString::fromLocal8Bit(token->text, token->length).toLower();
                function.filePath = fileName;
                function.line = token->line;
                function.column = token->column;

                readError = !readCMakeFunction( lexer, function);
                ret.append(function);

                if(readError)
                {
                    qCDebug(CMAKE) << "Error while parsing:" << function.name << "at" << function.line;
                }
            }
        }
    }
    cmListFileLexer_Delete(lexer);

    return ret;
}

}

bool CMakeListsParser::readCMakeFunction(cmListFileLexer *lexer, CMakeFunctionDesc &func)
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
                    func.arguments << CMakeFunctionArgument( QString::fromLocal8Bit(token->text, token->length), false, token->line, token->column );
                break;
            case cmListFileLexer_Token_ParenLeft:
                parenthesis++;
                func.arguments << CMakeFunctionArgument( QString::fromLocal8Bit(token->text, token->length), false, token->line, token->column );
                break;
            case cmListFileLexer_Token_Identifier:
            case cmListFileLexer_Token_ArgumentBracket:
            case cmListFileLexer_Token_ArgumentUnquoted:
                func.arguments << CMakeFunctionArgument( QString::fromLocal8Bit(token->text, token->length), false, token->line, token->column );
                break;
            case cmListFileLexer_Token_ArgumentQuoted:
                func.arguments << CMakeFunctionArgument( QString::fromLocal8Bit(token->text, token->length), true, token->line, token->column+1 );
                break;
            case cmListFileLexer_Token_Space:
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

    auto it=arguments.constBegin();
    auto itOther=other.arguments.constBegin();
    for(;it!=arguments.constEnd(); ++it, ++itOther)
    {
        if(*it!=*itOther)
            return false;
    }
    return true;
}

CMakeFunctionArgument::CMakeFunctionArgument(const QString& v, bool q, quint32 l, quint32 c)
    : value(unescapeValue(v)), quoted(q), line(l), column(c)
{
}

CMakeFunctionArgument::CMakeFunctionArgument(const QString& v)
    : value(v)
{
}
