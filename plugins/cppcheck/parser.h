/*
    SPDX-FileCopyrightText: 2013 Christoph Thielecke <crissi99@gmx.de>
    SPDX-FileCopyrightText: 2016 Anton Anikin <anton.anikin@htower.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CPPCHECK_PARSER_H
#define CPPCHECK_PARSER_H

#include <interfaces/iproblem.h>

#include <QStack>
#include <QXmlStreamReader>

namespace cppcheck
{

/// A class which parses cppcheck's XML output
class CppcheckParser : protected QXmlStreamReader
{
public:
    CppcheckParser();
    ~CppcheckParser();

    using QXmlStreamReader::addData;

    QVector<KDevelop::IProblem::Ptr> parse();

private:
    void storeError(QVector<KDevelop::IProblem::Ptr>& problems);
    KDevelop::IProblem::Ptr getProblem(int locationIdx = 0) const;

    // XML parsing
    bool endElement(QVector<KDevelop::IProblem::Ptr>& problems);
    bool startElement();
    void clear();

    enum State {
        Unknown,
        Results,
        CppCheck,
        Errors,
        Error,
        Location
    };

    QStack<State> m_stateStack;

    // error info
    QString m_errorSeverity;
    QString m_errorMessage;
    QString m_errorVerboseMessage;
    bool    m_errorInconclusive = false;
    QStringList  m_errorFiles;
    QVector<int> m_errorLines;
};

}

#endif
