/**************************************************************************************
 *    Copyright (C) 2016 by Carlos Nihelton <carlosnsoliveira@gmail.com>               *
 *                                                                                     *
 *    This program is free software; you can redistribute it and/or                    *
 *    modify it under the terms of the GNU General Public License                      *
 *    as published by the Free Software Foundation; either version 2                   *
 *    of the License, or (at your option) any later version.                           *
 *                                                                                     *
 *    This program is distributed in the hope that it will be useful,                  *
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *    GNU General Public License for more details.                                     *
 *                                                                                     *
 *    You should have received a copy of the GNU General Public License                *
 *    along with this program; if not, write to the Free Software                      *
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 ***************************************************************************************/

#ifndef CLANGTIDY_REPLACEMENT_H
#define CLANGTIDY_REPLACEMENT_H

#include <QFile>
#include <QRegularExpression>
#include <QVector>
#include <language/editor/documentrange.h>

using KDevelop::DocumentRange;
using KDevelop::IndexedString;

namespace ClangTidy
{

struct Replacement {
    size_t offset, lenght; // read from YAML.
    size_t line, column; // calculated by parser. Might drop eventually.
    QString replacementText; // read from YAML.
    DocumentRange range; // created from line and colum.
};

using Replacements = QVector<Replacement>;

class ReplacementParser
{
private:
    size_t currentLine;
    size_t currentColumn;
    size_t currentOffset;
    size_t cReplacements;

    QString m_yamlname;
    QString m_sourceFile;
    IndexedString i_source;
    QString m_yamlContent;
    QString m_sourceCode;
    static const QRegularExpression regex, check;
    Replacements all_replacements;

protected:
    Replacement nextNode(const QRegularExpressionMatch& smatch);
    KDevelop::DocumentRange composeNextNodeRange(size_t offset);

public:
    ReplacementParser() = default;
    explicit ReplacementParser(const QString& yaml_file, const QString& source_file);
    void parse();
    Replacements allReplacements() { return all_replacements; }
};
}

#endif // CLANGTIDY_REPLACEMENT_H
