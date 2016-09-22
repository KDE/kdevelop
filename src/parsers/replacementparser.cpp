/* 
 * This file is part of KDevelop
 *
 * Copyright 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "replacementparser.h"
#include "qCDebug/debug.h"
// See <https://github.com/CarlosNihelton/kdev-clang-tidy/issues/1>
#include <algorithm>
#include <fstream>
#include <iterator>

#ifdef BOOST_NO_EXCEPTIONS
// Because we are using boost we need to provide an implementation of this
// function, because KDE disables exceptions on
// boost libraries.
namespace boost
{
void throw_exception(std::exception const& e)
{
    qCDebug(KDEV_CLANGTIDY) << e.what();
}
}//namespace boost
#endif

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
        // See <https://github.com/CarlosNihelton/kdev-clang-tidy/issues/1>
        std::ifstream cpp;
        cpp.open(m_sourceFile.toUtf8());
        std::copy(std::istreambuf_iterator<char>(cpp), std::istreambuf_iterator<char>(),
                  std::back_insert_iterator<std::string>(m_sourceCode));
        m_sourceView = boost::string_ref(m_sourceCode);
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
    repl.length = smatch.captured(3).toInt();
    repl.replacementText = smatch.captured(4);
    if (repl.replacementText.startsWith('\'') && repl.replacementText.endsWith('\'')) {
        repl.replacementText.remove(0, 1);
        repl.replacementText.remove(repl.replacementText.length() - 1, 1);
    }

    repl.range = composeNextNodeRange(repl.offset, repl.length);
    return repl;
}

KDevelop::DocumentRange ReplacementParser::composeNextNodeRange(size_t offset, size_t length)
{
    KDevelop::DocumentRange range;

    if (offset < 1)
        return range;

    range.document = i_source;
    auto sourceView = m_sourceView.substr(currentOffset, offset - currentOffset);
    qDebug() << "sourceView.length(): " << sourceView.length() << '\n';
    size_t line = 0, col = 0;
    for (const auto elem : sourceView) {
        if (elem == char('\n')) {
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

    if (length == 0) {
        range.setBothColumns(currentColumn);
        range.setBothLines(currentLine);
        return range;
    }

    if(offset+length < m_sourceView.length()){
        sourceView = m_sourceView.substr(offset, length);
    
        line = 0;
        col = 0;
        for (const auto elem : sourceView) {
            if (elem == char('\n')) {
                ++line;
                col = 0;
            } else {
                ++col;
            }
        }
    }
        
    
    

    KTextEditor::Cursor start(currentLine, currentColumn);

    size_t endCol;

    if (line == 0) {
        endCol = currentColumn + col;
    } else {
        endCol = col;
    }

    KTextEditor::Cursor end(currentLine + line, endCol);

    range.setRange(start, end);

    return range;
}
} // namespace ClangTidy
