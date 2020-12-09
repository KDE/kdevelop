/*
 * This file is part of KDevelop
 * Copyright 2020  Morten Danielsen Volden <mvolden2@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "outputparser.h"
#include "shellcheckproblem.h"
#include "shellcheckdebug.h"


//#include "shell/problem.h"
#include "language/editor/documentrange.h"

#include "ktexteditor/cursor.h"

#include <KLocalizedString>

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace shellcheck 
{

OutputParser::OutputParser()
{
}

OutputParser::~OutputParser()
{
}

void OutputParser::add(const QStringList& lines)
{
    for(QString const& line : lines)
    {
        m_shellcheckoutput += line.toLatin1();
    }
}

KDevelop::IProblem::Severity OutputParser::getSeverityFromString(const QString& str)
{
    if(QLatin1String("error") == str) {
        return KDevelop::IProblem::Error;
    }
    else if(QLatin1String("warning") == str) {
        return KDevelop::IProblem::Warning;
    }
    else if(QLatin1String("info") == str) {
        return KDevelop::IProblem::Hint;
    }
    return KDevelop::IProblem::NoSeverity;
}


QVector<KDevelop::IProblem::Ptr> OutputParser::parse()
{
    QVector<KDevelop::IProblem::Ptr> result;
    if(m_shellcheckoutput.isEmpty())
        return result;
    
    QJsonParseError error;
    QJsonDocument outputJsonDoc = QJsonDocument::fromJson(m_shellcheckoutput, &error);    
    if(error.error != QJsonParseError::NoError) 
    {
        qWarning(PLUGIN_SHELLCHECK) << "Not possible to get any problems from shellcheck output. Parse error at " << error.offset << ":" << error.errorString();
        qWarning(PLUGIN_SHELLCHECK) << m_shellcheckoutput;
        return result;
    }

    QJsonObject jsonObj;
    jsonObj = outputJsonDoc.object();
    
    QJsonValue jsonVal;
    jsonVal = jsonObj.value(QStringLiteral("comments"));
    if(jsonVal.isArray())
    {
        QJsonArray array = jsonVal.toArray();
        for( auto const element : array)  {
            if(element.isObject()) {
                // convert to object and start extracting the problems
                QJsonObject currentProblem = element.toObject();
                ShellCheckProblem::Ptr problem(new ShellCheckProblem());
                KDevelop::DocumentRange range;
                KTextEditor::Cursor startCursor;
                KTextEditor::Cursor endCursor;

                startCursor.setLine(currentProblem[QStringLiteral("line")].toInt() - 1); // 0 start linecount
                startCursor.setColumn(currentProblem[QStringLiteral("column")].toInt() - 1); // 0 start columncount
                endCursor.setLine(currentProblem[QStringLiteral("endLine")].toInt() - 1);// 0 start linecount
                endCursor.setColumn(currentProblem[QStringLiteral("endColumn")].toInt() - 1); // 0 start columncount
                range.document = KDevelop::IndexedString(currentProblem[QStringLiteral("file")].toString());
                range.setStart(startCursor);
                range.setEnd(endCursor);

                problem->setFinalLocation(range);
                problem->setFinalLocationMode(KDevelop::IProblem::Range);
                problem->setSeverity(getSeverityFromString(currentProblem[QStringLiteral("level")].toString()));
                problem->setSource(KDevelop::IProblem::Plugin);
                QString problemMessage = currentProblem[QStringLiteral("message")].toString();
                problem->setDescription(problemMessage);
                problem->setExplanation(problemMessage);

                if (!currentProblem[QStringLiteral("fix")].isNull())
                {
                    qWarning(PLUGIN_SHELLCHECK) << "There is a fix associated with this problem: ";
                    QJsonObject fix = currentProblem[QStringLiteral("fix")].toObject();
                    QJsonArray replacementArray = fix[QStringLiteral("replacements")].toArray();
                    QVector<Fixit> fixes;
                    for( auto const replaceElement : replacementArray ) {
                        if(replaceElement.isObject()) {
                            QJsonObject replacement = replaceElement.toObject();
                            KTextEditor::Cursor fixStartCursor(replacement[QStringLiteral("line")].toInt() - 1,
                                replacement[QStringLiteral("column")].toInt() - 1);
                            KTextEditor::Cursor fixEndCursor(replacement[QStringLiteral("endLine")].toInt() -1,
                                replacement[QStringLiteral("endColumn")].toInt());
                            KDevelop::DocumentRange fixRange;
                            fixRange.document = KDevelop::IndexedString(currentProblem[QStringLiteral("file")].toString());
                            fixRange.setStart(fixStartCursor);
                            fixRange.setEnd(fixEndCursor);
                            Fixit newFix(problemMessage, fixRange,
                                         replacement[QStringLiteral("precedence")].toString(),
                                         replacement[QStringLiteral("replacement")].toString());
                            fixes.push_back(newFix);

                            qWarning(PLUGIN_SHELLCHECK) << " Replacement: ";
                            qWarning(PLUGIN_SHELLCHECK) << "   column " << replacement[QStringLiteral("column")].toInt();
                            qWarning(PLUGIN_SHELLCHECK) << "   endColumn " << replacement[QStringLiteral("endColumn")].toInt();
                            qWarning(PLUGIN_SHELLCHECK) << "   endLine " << replacement[QStringLiteral("endLine")].toInt();
                            qWarning(PLUGIN_SHELLCHECK) << "   insertionPoint " << replacement[QStringLiteral("insertionPoint")].toString();
                            qWarning(PLUGIN_SHELLCHECK) << "   line " << replacement[QStringLiteral("line")].toInt();
                            qWarning(PLUGIN_SHELLCHECK) << "   precedence " << replacement[QStringLiteral("precedence")].toString();
                            qWarning(PLUGIN_SHELLCHECK) << "   replacement " << replacement[QStringLiteral("replacement")].toString();
                        }
                    }
                    problem->setFixits(fixes);
                } else {
                    qWarning(PLUGIN_SHELLCHECK) << "Fix is null. No fix associated with this problem";
                }
                result.push_back(problem);
            } else {
                qWarning(PLUGIN_SHELLCHECK) << "This shellcheck output element is a:" << element.type() << " we expected it to be an object. Cannot parse this";
            }
        }
        //qWarning(PLUGIN_SHELLCHECK) << "Writing out the array:  " << array;
    }
    return result;
}

void OutputParser::clear()
{
    m_shellcheckoutput.clear();
}


}
