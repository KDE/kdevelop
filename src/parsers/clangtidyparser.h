/* This file is part of KDevelop
 * Copyright 2013 Christoph Thielecke <crissi99@gmx.de>
 * Copyright 2015 Anton Anikin <anton.anikin@htower.ru>

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

#ifndef _CLANGTIDYPARSER_H_
#define _CLANGTIDYPARSER_H_

#include <QStack>
#include <QXmlStreamReader>

#include <interfaces/iproblem.h>

namespace ClangTidy
{

/// A class which parses clangtidy's XML output
class ClangtidyParser : public QXmlStreamReader
{
public:
    explicit ClangtidyParser(QObject* parent = nullptr);
    ~ClangtidyParser();

    QVector<KDevelop::IProblem::Ptr> problems() const { return m_problems; }

    void parse();

private:
    void storeError();

    // XML parsing
    bool endElement();
    bool startElement();
    void clear();

    enum State { Unknown, Results, Clangtidy, Errors, Error, Location };

    QStack<State> m_stateStack;

    // error info
    int m_errorLine;
    bool m_errorInconclusive;
    QString m_errorFile;
    QString m_errorMessage;
    QString m_errorVerboseMessage;
    QString m_errorSeverity;

    QVector<KDevelop::IProblem::Ptr> m_problems;
};
}

#endif
