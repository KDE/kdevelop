/*
    SPDX-FileCopyrightText: 2007 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "variablereferenceparser.h"

bool isVarNameChar(QChar* c)
{
    bool ret = c->isLetterOrNumber() || c->unicode() == '_';
    return ret;
}

VariableInfo::VariableInfo()
{
}

VariableReferenceParser::VariableReferenceParser()
{
}

void VariableReferenceParser::setContent(const QString& content)
{
    m_content = content;
}

bool VariableReferenceParser::parse()
{
    int size = m_content.size();
    int curpos = 0;
    if (size == 0 || size < 3) {
        return true;
    }
    QChar* it = m_content.data();
    do {
        if (it->unicode() == '$' && size > curpos + 2) {
            it++;
            curpos++;
            if (it->unicode() == '$') {
                int begin = curpos - 1;
                it++;
                curpos++;
                QString variable;
                VariableInfo::VariableType type = VariableInfo::QMakeVariable;
                if (it->unicode() == '(') {
                    do {
                        it++;
                        curpos++;
                    } while (curpos < size && it->unicode() != ')');
                    type = VariableInfo::ShellVariableResolveQMake;
                    variable = m_content.mid(begin + 3, curpos - begin - 3);
                    ++curpos;
                } else if (it->unicode() == '{') {
                    do {
                        it++;
                        curpos++;
                        if (it->unicode() == '(') {
                            type = VariableInfo::FunctionCall;
                        }
                    } while (curpos < size && it->unicode() != '}');
                    variable = m_content.mid(begin + 3, curpos - begin - 3);
                    ++curpos;
                } else if (it->unicode() == '[') {
                    do {
                        it++;
                        curpos++;
                    } while (curpos < size && it->unicode() != ']');
                    type = VariableInfo::QtConfigVariable;
                    variable = m_content.mid(begin + 3, curpos - begin - 3);
                    ++curpos;
                } else {
                    do {
                        it++;
                        curpos++;
                    } while (curpos < size && isVarNameChar(it));
                    variable = m_content.mid(begin + 2, curpos - begin - 2);

                    if (it->unicode() == '(') {
                        type = VariableInfo::FunctionCall;
                        int braceCount = 0;
                        do {
                            it++;
                            curpos++;
                            if (it->unicode() == ')') {
                                braceCount--;
                            } else if (it->unicode() == '(') {
                                braceCount++;
                            }
                        } while (curpos < size && (it->unicode() != ')' || braceCount == 0));
                        // count the current position one further if we have it
                        // at the closing brace, this is needed for proper end-calculation
                        if (curpos < size && it->unicode() == ')') {
                            it++;
                            curpos++;
                        }
                    }
                }
                int end = curpos - 1;
                appendPosition(variable, begin, end, type);
            } else if (it->unicode() == '(') {
                int begin = curpos - 1;
                do {
                    it++;
                    curpos++;
                } while (curpos < size && it->unicode() != ')');
                int end = curpos - 1;
                appendPosition(m_content.mid(begin + 2, end - (begin + 2)), begin, end,
                               VariableInfo::ShellVariableResolveMake);
            }
        } else {
            curpos++;
        }
    } while (curpos < size);
    return true;
}

QStringList VariableReferenceParser::variableReferences() const
{
    return m_variables.keys();
}

VariableInfo VariableReferenceParser::variableInfo(const QString& var) const
{
    return m_variables.value(var, VariableInfo());
}

void VariableReferenceParser::appendPosition(const QString& var, int start, int end, VariableInfo::VariableType type)
{
    auto variableIt = m_variables.find(var);
    if (variableIt == m_variables.end()) {
        VariableInfo vi;
        vi.type = type;
        variableIt = m_variables.insert(var, vi);
    }
    variableIt->positions << VariableInfo::Position(start, end);
}
