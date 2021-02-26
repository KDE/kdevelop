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

#include <interfaces/idocumentcontroller.h>
#include "icore.h"

#include "ktexteditor/cursor.h"

#include <KTextEditor/Document>
#include <KLocalizedString>

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <idocumentationcontroller.h>

namespace shellcheck
{

OutputParser::OutputParser()
{
}

OutputParser::~OutputParser()
{
}

QString textToHtml( const QString & input )
{
    QString output(QStringLiteral("<html>%1</html>").arg(input.toHtmlEscaped()));

    output.replace(QLatin1String("\\012"), QLatin1String("\n"));

    if (output.count(QLatin1Char('\n')) >= 2) {
        output.replace(output.indexOf(QLatin1Char('\n')), 1, QStringLiteral("<pre>") );
        output.replace(output.lastIndexOf(QLatin1Char('\n')), 1, QStringLiteral("</pre><br>") );
    }

    return output;
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

Fixit OutputParser::getFixFromOneReplacement(QJsonArray replacementArray,
                                             KDevelop::DocumentRange problemRange, QString problemDescription)
{
    Q_ASSERT(replacementArray.size() == 1);
    Fixit ret;
    QJsonValue replaceElement = replacementArray.first();
    if(replaceElement.isObject()) {
        QJsonObject replacement = replaceElement.toObject();
        KTextEditor::Cursor fixStartCursor(replacement[QStringLiteral("line")].toInt() - 1,
            replacement[QStringLiteral("column")].toInt() - 1);
        KTextEditor::Cursor fixEndCursor(replacement[QStringLiteral("endLine")].toInt() -1,
            replacement[QStringLiteral("endColumn")].toInt());

        QString currentDocText = KDevelop::ICore::self()->documentController()->activeDocument()->text(problemRange);

        KDevelop::DocumentRange fixRange;
        fixRange.document = problemRange.document;
        fixRange.setStart(fixStartCursor);
        fixRange.setEnd(fixEndCursor);
        Fixit newFix(problemDescription, fixRange,
                     currentDocText,
                     replacement[QStringLiteral("replacement")].toString());
        //fixes.push_back(newFix);
        qWarning(PLUGIN_SHELLCHECK) << newFix;
        ret = newFix;
    }
    return ret;
}

Fixit shellcheck::OutputParser::getFixFromMultipleReplacements(QJsonArray replacementArray,
                                                               KDevelop::DocumentRange problemRange, QString problemDescription)
{
    Q_ASSERT(replacementArray.size() > 1);
    Fixit ret;
    for( auto const replaceElement : replacementArray ) {
        if(replaceElement.isObject()) {
            QJsonObject replacement = replaceElement.toObject();
            KTextEditor::Cursor fixStartCursor(replacement[QStringLiteral("line")].toInt() - 1,
                replacement[QStringLiteral("column")].toInt() - 1);
            KTextEditor::Cursor fixEndCursor(replacement[QStringLiteral("endLine")].toInt() -1,
                replacement[QStringLiteral("endColumn")].toInt());
            KDevelop::DocumentRange fixRange;
            fixRange.document = KDevelop::IndexedString(problemRange.document.str());
            fixRange.setStart(fixStartCursor);
            fixRange.setEnd(fixEndCursor);
            //QString currentDocText = QStringLiteral("/hej/sa/Messages.sh");
            auto* test = KDevelop::ICore::self()->documentController()->activeDocument();
            if(test == nullptr) {
                qWarning(PLUGIN_SHELLCHECK) << "--- We need to have an active document in the documentController during unittesting";
            }
            QString currentDocText = KDevelop::ICore::self()->documentController()->activeDocument()->text(problemRange);
            qWarning(PLUGIN_SHELLCHECK) << "We are in getFixFromMultipleReplacements,and the problemDescription: " << problemDescription;
            qWarning(PLUGIN_SHELLCHECK) << "and the file: " << problemRange.document.str();

            Fixit newFix(problemDescription, fixRange,
                            currentDocText,
                            replacement[QStringLiteral("replacement")].toString());

            ret = newFix;
            qWarning(PLUGIN_SHELLCHECK) << newFix;
        }
    }

    return ret;
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
                QString problemText = currentProblem[QStringLiteral("message")].toString();
                QString problemCode = QString::number( currentProblem[QStringLiteral("code")].toInt());
                QString problemMessage = problemCode + QLatin1String(": ") + problemText;
                problem->setDescription(problemMessage);
                problem->setExplanation(textToHtml(problemText));

                if (!currentProblem[QStringLiteral("fix")].isNull())
                {
                    QJsonObject fix = currentProblem[QStringLiteral("fix")].toObject();
                    QJsonArray replacementArray = fix[QStringLiteral("replacements")].toArray();
                    qWarning(PLUGIN_SHELLCHECK) << "There are " << replacementArray.size() << " fix(es) associated with this problem: ";
                    QVector<Fixit> fixes;
                    Fixit fixForCurrentProblem;
                    if(replacementArray.size() == 1) {
                        fixForCurrentProblem = getFixFromOneReplacement(replacementArray, range, textToHtml(problemText));
                    } else if (replacementArray.size() == 2) {
                        fixForCurrentProblem = getFixFromMultipleReplacements(replacementArray, range, textToHtml(problemText));
                    }

                    fixes.push_back(fixForCurrentProblem);

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

