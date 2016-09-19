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

#include "replacementparser.h"

namespace ClangTidy
{

const QRegularExpression ReplacementParser::check{ QStringLiteral(
    "---\\s+MainSourceFile:\\s+.+\\s+Replacements:(\\s+.+)+\\s\\.\\.\\.") };

const QRegularExpression ReplacementParser::regex{ (
    QStringLiteral("\\s+\\s+-\\s+FilePath:\\s+(.+\\.cpp)\\s+Offset:\\s+(\\d+)\\s+Length:\\s+"
                   "(\\d+)\\s+ ReplacementText:\\s(.+)")) };

ReplacementParser::ReplacementParser(const QString& yaml_file, const QString& source_file)
    : currentLine{ 0 }
    , currentColumn{ 0 }
    , currentOffset{ 0 }
    , cReplacements{ 0 }
    , m_yamlname{ yaml_file }
    , m_sourceFile{ source_file }
{
    if (m_yamlname.endsWith(".yaml")) {
        QFile yaml(m_yamlname);
        yaml.open(QIODevice::ReadOnly);
        m_yamlContent = yaml.readAll();

        auto checkMatch = check.match(m_yamlContent);
        if (!checkMatch.hasMatch()) {
            m_yamlname.clear();
            m_yamlContent.clear();
        }
    }

    // TODO: Discover a way to get that from KDevelop.
    if (m_sourceFile.endsWith(".cpp")) {
        i_source = IndexedString(m_sourceFile);
        QFile cpp(m_sourceFile);
        cpp.open(QIODevice::ReadOnly);
        m_sourceCode = cpp.readAll();
    }
}

void ReplacementParser::parse()
{
    if (m_yamlContent.isEmpty())
        return; // Nothing to parse.

    for (auto iMatch = regex.globalMatch(m_yamlContent); iMatch.hasNext(); ++cReplacements) {
        auto smatch = iMatch.next();
        auto rep = nextNode(smatch);
        all_replacements.push_back(rep);
    }
}

Replacement ReplacementParser::nextNode(const QRegularExpressionMatch& smatch)
{
    Replacement repl;

    if (smatch.captured(1) != m_sourceFile)
        return repl; // Parsing output from only one file.

    repl.offset = smatch.captured(2).toInt();
    repl.lenght = smatch.captured(3).toInt();
    repl.replacementText = smatch.captured(4);
    if (repl.replacementText.startsWith('\'') && repl.replacementText.endsWith('\'')) {
        repl.replacementText.remove(0, 1);
        repl.replacementText.remove(repl.replacementText.length() - 1, 1);
    }

    repl.range = composeNextNodeRange(repl.offset);
    return repl;
}

KDevelop::DocumentRange ReplacementParser::composeNextNodeRange(size_t offset)
{
    KDevelop::DocumentRange range;

    if (offset < 1)
        return range;

    range.document = i_source;
    QStringRef sourceView(&m_sourceCode, currentOffset, offset - currentOffset);
    size_t line = 0, col = 0;
    for (const auto elem : sourceView) {
        if (elem == QChar('\n')) {
            ++line;
            col = 0;
        } else {
            ++col;
        }
    }

    if (line == 0) {
        currentColumn += col;
    } else {
        currentColumn = col;
    }
    currentLine += line;
    currentOffset = offset;
    range.setBothColumns(currentColumn);
    range.setBothLines(currentLine);

    return range;
}
} //namespace ClangTidy
