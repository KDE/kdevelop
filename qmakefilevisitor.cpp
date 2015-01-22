/*****************************************************************************
 * Copyright (c) 2010 Milian Wolff <mail@milianw.de>                         *
 *                                                                           *
 * This program is free software; you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation; either version 2 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *****************************************************************************/

#include "qmakefilevisitor.h"

#include "debug.h"
#include "qmakefile.h"
#include "qmakeincludefile.h"

#include "parser/ast.h"

#include <QtCore/QStringList>
#include <QtCore/QDir>

#include <KDebug>
#include <QProcessEnvironment>

#define ifDebug(x)

//BEGIN QMakeFileVisitor

QMakeFileVisitor::QMakeFileVisitor(const QMakeVariableResolver* resolver, QMakeFile* baseFile)
: m_resolver(resolver)
, m_baseFile(baseFile)
{

}

QMakeFileVisitor::~QMakeFileVisitor()
{

}

void QMakeFileVisitor::setVariables(const VariableMap& vars)
{
    m_variableValues = vars;
}

QMakeVariableResolver::VariableMap QMakeFileVisitor::visitFile(QMake::ProjectAST* node)
{
    visitProject(node);
    return m_variableValues;
}

QStringList QMakeFileVisitor::visitMacro(QMake::ScopeBodyAST* node, const QStringList& arguments)
{
    m_arguments = arguments;
    visitScopeBody(node);
    return m_lastReturn;
}

QStringList QMakeFileVisitor::resolveVariable(const QString& variable, VariableInfo::VariableType type) const
{
    if (type == VariableInfo::QMakeVariable && m_variableValues.contains(variable)) {
        return m_variableValues.value(variable);
    } else {
        return m_resolver->resolveVariable(variable, type);
    }
}

QStringList QMakeFileVisitor::getValueList( const QList<QMake::ValueAST*>& list ) const
{
    QStringList result;
    foreach( QMake::ValueAST* v, list)
    {
        result += resolveVariables( v->value );
    }
    return result;
}

void QMakeFileVisitor::visitFunctionCall( QMake::FunctionCallAST* node )
{
    if( node->identifier->value == "include" || node->identifier->value == "!include" )
    {
        if( node->args.isEmpty() )
            return;
        QStringList arguments = getValueList( node->args );

        ifDebug(kDebug(9024) << "found include" << node->identifier->value << arguments;)
        QString argument = arguments.join("").trimmed();
        if(!argument.isEmpty() && QFileInfo( argument ).isRelative() )
        {
            argument = QFileInfo( m_baseFile->absoluteDir() + '/' + argument ).canonicalFilePath();
        }
        if (argument.isEmpty()) {
            kWarning() << "empty include file detected in line" << node->startLine;
            if( node->identifier->value.startsWith('!') ) {
                visitNode( node->body );
            }
            return;
        }
        ifDebug(kDebug(9024) << "Reading Include file:" << argument;)
        QMakeIncludeFile includefile( argument, m_baseFile, m_variableValues );
        bool read = includefile.read();
        ifDebug(kDebug(9024) << "successfully read:" << read;)
        if( read )
        {
            //TODO: optimize by using variableMap and iterator, don't compare values
            foreach( const QString& var, includefile.variables() )
            {
                if( m_variableValues.value( var ) != includefile.variableValues( var ) )
                {
                    m_variableValues[ var ] = includefile.variableValues( var );
                }
            }
            if( !node->identifier->value.startsWith('!') )
            {
                visitNode( node->body );
            }
        }else if( node->identifier->value.startsWith('!') )
        {
            visitNode( node->body );
        }
    }else if (node->body && (node->identifier->value == "defineReplace" || node->identifier->value == "defineTest"))
    { // TODO: differentiate between replace and test functions?
        QStringList args = getValueList(node->args);
        if (!args.isEmpty()) {
            m_userMacros[args.first()] = node->body;
        } // TODO: else return error
    }else if (node->identifier->value == "return")
    {
        m_lastReturn = getValueList(node->args);
    }else
    { // TODO: only visit when test function returned true?
        kWarning() << "unhandled function call" << node->identifier->value;
        visitNode( node->body );
    }
}

void QMakeFileVisitor::visitAssignment( QMake::AssignmentAST* node )
{
    QString op = node->op->value;
    QStringList values = getValueList(node->values);
    if( op == "=" )
    {
        m_variableValues[node->identifier->value] = values;
    }else if( op == "+=" )
    {
        m_variableValues[node->identifier->value] += values;
    }else if( op == "-=" )
    {
        foreach( const QString& value, values )
        {
            m_variableValues[node->identifier->value].removeAll(value);
        }
    }else if( op == "*=" )
    {
        foreach( const QString& value, values )
        {
            if( !m_variableValues.value(node->identifier->value).contains(value) )
            {
                m_variableValues[node->identifier->value].append(value);
            }
        }
    }else if( op == "~=" )
    {
        if( values.isEmpty() )
            return;
        QString value = values.first().trimmed();
        QString regex = value.mid(2,value.indexOf("/", 2));
        QString replacement = value.mid(value.indexOf("/", 2)+1,value.lastIndexOf("/"));
        m_variableValues[node->identifier->value].replaceInStrings( QRegExp(regex), replacement );
    }
}

/// return 1-n for numeric variable, 0 otherwise
int functionArgument(const QString& var)
{
    bool ok;
    int arg = var.toInt(&ok);
    if (!ok) {
        return 0;
    } else {
        return arg;
    }
}

QStringList QMakeFileVisitor::resolveVariables( const QString& var ) const
{
    VariableReferenceParser parser;
    parser.setContent( var );
    if( !parser.parse() ) {
        kWarning(9024) << "Couldn't parse" << var << "to replace variables in it";
        return QStringList() << var;
    }
    if (parser.variableReferences().isEmpty()) {
        return QStringList() << var;
    }

    ///TODO: multiple vars in one place will make the offsets go bonkers
    QString value = var;
    foreach( const QString& variable, parser.variableReferences() ) {
        VariableInfo vi = parser.variableInfo( variable );
        QString varValue;

        switch (vi.type) {
            case VariableInfo::QMakeVariable:
                if (int arg = functionArgument(variable)) {
                    if (arg > 0 && arg <= m_arguments.size()) {
                        varValue = m_arguments.at(arg - 1);
                    } else {
                        kWarning(9024) << "undefined macro argument:" << variable;
                    }
                } else {
                    varValue = resolveVariable(variable, vi.type).join(" ");
                }
                break;
            case VariableInfo::ShellVariableResolveQMake:
            case VariableInfo::ShellVariableResolveMake:
                ///TODO: make vs qmake time
                varValue = QProcessEnvironment::systemEnvironment().value(variable);
                break;
            case VariableInfo::QtConfigVariable:
                varValue = resolveVariable(variable, vi.type).join(" ");
                break;
            case VariableInfo::FunctionCall: {
                QStringList arguments;
                foreach(const VariableInfo::Position& pos, vi.positions ) {
                    int start = pos.start + 3 + variable.length();
                    QString args = value.mid(start , pos.end - start);
                    varValue = resolveVariables( args ).join(" ");
                    arguments << varValue;
                }
                varValue = evaluateMacro(variable, arguments).join(" ");
                break;
            } case VariableInfo::Invalid:
                kWarning(9024) << "invalid qmake variable:" << variable;
                continue;
        }

        foreach(const VariableInfo::Position& pos, vi.positions ) {
            value.replace(pos.start, pos.end - pos.start + 1, varValue);
        }
    }

    QStringList ret = value.split(" ", QString::SkipEmptyParts);
    ifDebug(kDebug(9024) << "resolved variable" << var << "to" << ret;)
    return ret;
}

QStringList QMakeFileVisitor::evaluateMacro(const QString& function, const QStringList& arguments) const
{
    if (function == QLatin1String("qtLibraryTarget")) {
        return QStringList() << arguments.first();
    } ///  TODO: support more built-in qmake functions

    QHash< QString, QMake::ScopeBodyAST* >::const_iterator it = m_userMacros.find(function);
    if (it != m_userMacros.constEnd()) {
        qCDebug(KDEV_QMAKE) << "calling user macro:" << function << arguments;
        QMakeFileVisitor visitor(this, m_baseFile);
        return visitor.visitMacro(it.value(), arguments);
    } else {
        kWarning() << "unhandled macro call:" << function << arguments;
    }
    return QStringList();
}
