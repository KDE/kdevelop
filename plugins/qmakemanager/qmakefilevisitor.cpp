/*
    SPDX-FileCopyrightText: 2010 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qmakefilevisitor.h"

#include "qmakefile.h"
#include "qmakeincludefile.h"

#include "parser/ast.h"

#include <QFileInfo>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QStringList>

#include <debug.h>
#define ifDebug(x)

// BEGIN QMakeFileVisitor

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
    if (type == VariableInfo::QMakeVariable) {
        const auto variableValueIt = m_variableValues.find(variable);
        if (variableValueIt != m_variableValues.end()) {
            return *variableValueIt;
        }
    }

    return m_resolver->resolveVariable(variable, type);
}

QStringList QMakeFileVisitor::getValueList(const QList<QMake::ValueAST*>& list) const
{
    QStringList result;
    for (QMake::ValueAST* v : list) {
        result += resolveVariables(v->value);
    }
    return result;
}

void QMakeFileVisitor::visitFunctionCall(QMake::FunctionCallAST* node)
{
    if (node->identifier->value == QLatin1String("include") || node->identifier->value == QLatin1String("!include")) {
        if (node->args.isEmpty())
            return;
        QStringList arguments = getValueList(node->args);

        ifDebug(qCDebug(KDEV_QMAKE) << "found include" << node->identifier->value << arguments;)
        QString argument = arguments.join(QString()).trimmed();
        if (!argument.isEmpty() && QFileInfo(argument).isRelative()) {
            argument = QFileInfo(m_baseFile->absoluteDir() + QLatin1Char('/') + argument).canonicalFilePath();
        }
        if (argument.isEmpty()) {
            qCWarning(KDEV_QMAKE) << "empty include file detected in line" << node->startLine;
            if (node->identifier->value.startsWith(QLatin1Char('!'))) {
                visitNode(node->body);
            }
            return;
        }
        ifDebug(qCDebug(KDEV_QMAKE) << "Reading Include file:" << argument;)
            QMakeIncludeFile includefile(argument, m_baseFile, m_variableValues);
        bool read = includefile.read();
        ifDebug(qCDebug(KDEV_QMAKE) << "successfully read:" << read;) if (read)
        {
            // TODO: optimize by using variableMap and iterator, don't compare values
            const auto vars = includefile.variables();
            for (const auto& var : vars) {
                if (m_variableValues.value(var) != includefile.variableValues(var)) {
                    m_variableValues[var] = includefile.variableValues(var);
                }
            }
            if (!node->identifier->value.startsWith(QLatin1Char('!'))) {
                visitNode(node->body);
            }
        }
        else if (node->identifier->value.startsWith(QLatin1Char('!'))) { visitNode(node->body); }
    } else if (node->body && (node->identifier->value == QLatin1String("defineReplace")
                              || node->identifier->value
                                  == QLatin1String("defineTest"))) { // TODO: differentiate between replace and test functions?
        QStringList args = getValueList(node->args);
        if (!args.isEmpty()) {
            m_userMacros[args.first()] = node->body;
        } // TODO: else return error
    } else if (node->identifier->value == QLatin1String("return")) {
        m_lastReturn = getValueList(node->args);
    } else { // TODO: only visit when test function returned true?
        qCWarning(KDEV_QMAKE) << "unhandled function call" << node->identifier->value;
        visitNode(node->body);
    }
}

void QMakeFileVisitor::visitAssignment(QMake::AssignmentAST* node)
{
    QString op = node->op->value;
    const QStringList values = getValueList(node->values);
    if (op == QLatin1String("=")) {
        m_variableValues[node->identifier->value] = values;
    } else if (op == QLatin1String("+=")) {
        m_variableValues[node->identifier->value] += values;
    } else if (op == QLatin1String("-=")) {
        for (const QString& value : values) {
            m_variableValues[node->identifier->value].removeAll(value);
        }
    } else if (op == QLatin1String("*=")) {
        for (const QString& value : values) {
            if (!m_variableValues.value(node->identifier->value).contains(value)) {
                m_variableValues[node->identifier->value].append(value);
            }
        }
    } else if (op == QLatin1String("~=")) {
        if (values.isEmpty())
            return;
        QString value = values.first().trimmed();
        QString regex = value.mid(2, value.indexOf(QLatin1Char('/'), 2));
        QString replacement = value.mid(value.indexOf(QLatin1Char('/'), 2) + 1, value.lastIndexOf(QLatin1Char('/')));
        m_variableValues[node->identifier->value].replaceInStrings(QRegularExpression(regex), replacement);
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

QStringList QMakeFileVisitor::resolveVariables(const QString& var) const
{
    VariableReferenceParser parser;
    parser.setContent(var);
    if (!parser.parse()) {
        qCWarning(KDEV_QMAKE) << "Couldn't parse" << var << "to replace variables in it";
        return QStringList() << var;
    }
    const auto variableReferences = parser.variableReferences();
    if (variableReferences.isEmpty()) {
        return QStringList() << var;
    }

    /// TODO: multiple vars in one place will make the offsets go bonkers
    QString value = var;
    for (const auto& variable : variableReferences) {
        const auto vi = parser.variableInfo(variable);
        QString varValue;

        switch (vi.type) {
        case VariableInfo::QMakeVariable:
            if (int arg = functionArgument(variable)) {
                if (arg > 0 && arg <= m_arguments.size()) {
                    varValue = m_arguments.at(arg - 1);
                } else {
                    qCWarning(KDEV_QMAKE) << "undefined macro argument:" << variable;
                }
            } else {
                varValue = resolveVariable(variable, vi.type).join(QLatin1Char(' '));
            }
            break;
        case VariableInfo::ShellVariableResolveQMake:
        case VariableInfo::ShellVariableResolveMake:
            /// TODO: make vs qmake time
            varValue = QProcessEnvironment::systemEnvironment().value(variable);
            break;
        case VariableInfo::QtConfigVariable:
            varValue = resolveVariable(variable, vi.type).join(QLatin1Char(' '));
            break;
        case VariableInfo::FunctionCall: {
            QStringList arguments;
            arguments.reserve(vi.positions.size());
            for (const auto& pos : vi.positions) {
                int start = pos.start + 3 + variable.length();
                QString args = value.mid(start, pos.end - start);
                varValue = resolveVariables(args).join(QLatin1Char(' '));
                arguments << varValue;
            }
            varValue = evaluateMacro(variable, arguments).join(QLatin1Char(' '));
            break;
        }
        case VariableInfo::Invalid:
            qCWarning(KDEV_QMAKE) << "invalid qmake variable:" << variable;
            continue;
        }

        for (const auto& pos : vi.positions) {
            value.replace(pos.start, pos.end - pos.start + 1, varValue);
        }
    }

    const QStringList ret = value.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    ifDebug(qCDebug(KDEV_QMAKE) << "resolved variable" << var << "to" << ret;) return ret;
}

QStringList QMakeFileVisitor::evaluateMacro(const QString& function, const QStringList& arguments) const
{
    if (function == QLatin1String("qtLibraryTarget")) {
        return QStringList() << arguments.first();
    } ///  TODO: support more built-in qmake functions

    QHash<QString, QMake::ScopeBodyAST*>::const_iterator it = m_userMacros.find(function);
    if (it != m_userMacros.constEnd()) {
        qCDebug(KDEV_QMAKE) << "calling user macro:" << function << arguments;
        QMakeFileVisitor visitor(this, m_baseFile);
        return visitor.visitMacro(it.value(), arguments);
    } else {
        qCWarning(KDEV_QMAKE) << "unhandled macro call:" << function << arguments;
    }
    return QStringList();
}
