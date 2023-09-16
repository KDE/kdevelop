/*
    SPDX-FileCopyrightText: 2016 Carlos Nihelton <carlosnsoliveira@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "replacementparser.h"

// plugin
#include <debug.h>
// Std
// See <https://github.com/CarlosNihelton/kdev-clang-tidy/issues/1>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <tuple>

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
} // namespace boost
#endif

namespace
{

/**
* @function
* @brief For a given string_ref representing the source code, it counts the number of rows and the column where the
* string_ref ends.
* @param substring : a piece of the source code.
* @return std::pair<size_t,size_t>: <b>first</b>: count of lines and <b>second</b>: column at the end.
*/
inline std::pair<size_t, size_t> countOfRowAndColumnToTheEndOfSubstr(boost::string_ref substring)
{
    size_t line = 0, col = 0;
    for (const auto elem : substring) {
        if (elem == char('\n')) {
            ++line;
            col = 0;
        } else {
            ++col;
        }
    }

    return std::make_pair(line, col);
}

} // namespace

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
    if (m_yamlname.endsWith(QLatin1String(".yaml"))) {
        QFile yaml(m_yamlname);
        yaml.open(QIODevice::ReadOnly);
        m_yamlContent = QString::fromLocal8Bit(yaml.readAll());

        auto checkMatch = check.match(m_yamlContent);
        if (!checkMatch.hasMatch()) {
            m_yamlname.clear();
            m_yamlContent.clear();
        }
    }

    // TODO (CarlosNihelton): Discover a way to get that from KDevelop.
    if (m_sourceFile.endsWith(QLatin1String(".cpp"))) {
        i_source = IndexedString(m_sourceFile);
        // See <https://github.com/CarlosNihelton/kdev-clang-tidy/issues/1>
        std::ifstream cpp;
        cpp.open(m_sourceFile.toUtf8().constData());
        std::copy(std::istreambuf_iterator<char>(cpp), std::istreambuf_iterator<char>(),
                  std::back_insert_iterator<std::string>(m_sourceCode));
        m_sourceView = boost::string_ref(m_sourceCode);
    }
}

void ReplacementParser::parse()
{
    if (m_yamlContent.isEmpty()) {
        return; // Nothing to parse.
    }

    for (auto iMatch = regex.globalMatch(m_yamlContent); iMatch.hasNext(); ++cReplacements) {
        auto smatch = iMatch.next();
        auto rep = nextNode(smatch);
        all_replacements.push_back(rep);
    }
}

Replacement ReplacementParser::nextNode(const QRegularExpressionMatch& smatch)
{
    Replacement repl;

    if (smatch.capturedView(1) != m_sourceFile) {
        return repl; // Parsing output from only one file.
    }
    const int off = smatch.capturedView(2).toInt();
    const int len = smatch.capturedView(3).toInt();
    repl.range = composeNextNodeRange(off, len);
    if (repl.range.isValid()) {
        repl.offset = off;
        repl.length = len;
        repl.replacementText = smatch.captured(4);
        if (repl.replacementText.startsWith(QLatin1Char('\'')) && repl.replacementText.endsWith(QLatin1Char('\''))) {
            repl.replacementText.remove(0, 1);
            repl.replacementText.remove(repl.replacementText.length() - 1, 1);
        }
    } else {
        repl.offset = 0;
        repl.length = 0;
        repl.replacementText.clear();
    }

    return repl;
}

KDevelop::DocumentRange ReplacementParser::composeNextNodeRange(size_t offset, size_t length)
{
    qCDebug(KDEV_CLANGTIDY) << "count: " << cReplacements << "\toffset: " << offset << "\tlength: " << length << '\n';
    KDevelop::DocumentRange range{ KDevelop::IndexedString(), KTextEditor::Range::invalid() };
    /// See https://github.com/CarlosNihelton/kdev-clang-tidy/issues/2.
    if (offset < 1 || offset + length >= m_sourceView.length()) {
        return range;
    }

    range.document = i_source;
    auto sourceView = m_sourceView.substr(currentOffset, offset - currentOffset);

    auto pos = ::countOfRowAndColumnToTheEndOfSubstr(sourceView);

    if (pos.first == 0) {
        currentColumn += pos.second;
    } else {
        currentColumn = pos.second;
    }
    currentLine += pos.first;
    currentOffset = offset;

    if (length == 0) {
        range.setBothColumns(currentColumn);
        range.setBothLines(currentLine);
        return range;
    }

    sourceView = m_sourceView.substr(offset, length);
    pos = ::countOfRowAndColumnToTheEndOfSubstr(sourceView);

    KTextEditor::Cursor start(currentLine, currentColumn);

    size_t endCol;
    if (pos.first == 0) {
        endCol = currentColumn + pos.second;
    } else {
        endCol = pos.second;
    }

    KTextEditor::Cursor end(currentLine + pos.first, endCol);

    range.setRange(start, end);

    return range;
}
} // namespace ClangTidy
