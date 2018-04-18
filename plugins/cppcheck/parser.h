/* This file is part of KDevelop

   Copyright 2013 Christoph Thielecke <crissi99@gmx.de>
   Copyright 2016 Anton Anikin <anton.anikin@htower.ru>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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
