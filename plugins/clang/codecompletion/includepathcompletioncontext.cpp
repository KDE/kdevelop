/*
    SPDX-FileCopyrightText: 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>
    SPDX-FileCopyrightText: 2015 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "includepathcompletioncontext.h"

#include "duchain/navigationwidget.h"
#include "duchain/clanghelpers.h"

#include <language/codecompletion/abstractincludefilecompletionitem.h>
#include <util/stringviewhelpers.h>

#include <KTextEditor/Document>
#include <KTextEditor/View>

#include <QDirIterator>

#include <algorithm>

using namespace KDevelop;

/**
 * Parse the last line of @p text and extract information about any existing include path from it.
 */
IncludePathProperties IncludePathProperties::parseText(const QString& text, int rightBoundary)
{
    IncludePathProperties properties;

    int idx = text.lastIndexOf(QLatin1Char('\n'));
    if (idx == -1) {
        idx = 0;
    }
    if (rightBoundary == -1) {
        rightBoundary = text.length();
    }

    // what follows is a relatively simple parser for include lines that may contain comments, i.e.:
    // /*comment*/ #include /*comment*/ "path.h" /*comment*/
    enum FindState {
        FindBang,
        FindInclude,
        FindType,
        FindTypeEnd
    };
    FindState state = FindBang;
    QChar expectedEnd = QLatin1Char('>');
    for (; idx < text.size(); ++idx) {
        const auto c = text.at(idx);
        if (c.isSpace()) {
            continue;
        }
        if (c == QLatin1Char('/') && state != FindTypeEnd) {
            // skip comments
            if (idx >= text.length() - 1 || text.at(idx + 1) != QLatin1Char('*')) {
                properties.valid = false;
                return properties;
            }
            idx += 2;
            while (idx < text.length() - 1 && (text.at(idx) != QLatin1Char('*') || text.at(idx + 1) != QLatin1Char('/'))) {
                ++idx;
            }
            if (idx >= text.length() - 1 || text.at(idx) != QLatin1Char('*') || text.at(idx + 1) != QLatin1Char('/')) {
                properties.valid = false;
                return properties;
            }
            ++idx;
            continue;
        }
        switch (state) {
            case FindBang:
                if (c != QLatin1Char('#')) {
                    return properties;
                }
                state = FindInclude;
                break;
            case FindInclude: {
                constexpr QLatin1String includeString("include", 7);
                if (!matchesAtOffset(text, idx, includeString)) {
                    return properties;
                }
                idx += includeString.size() - 1;
                state = FindType;
                properties.valid = true;
                break;
            }
            case FindType:
                properties.inputFrom = idx + 1;
                if (c == QLatin1Char('"')) {
                    expectedEnd = QLatin1Char('"');
                    properties.local = true;
                } else if (c != QLatin1Char('<')) {
                    properties.valid = false;
                    return properties;
                }
                state = FindTypeEnd;
                break;
            case FindTypeEnd:
                if (c == expectedEnd) {
                    properties.inputTo = idx;
                    // stop iteration
                    idx = text.size();
                }
                break;
        }
    }

    if (!properties.valid) {
        return properties;
    }

    // properly append to existing paths without overriding it
    // i.e.: #include <foo/> should become #include <foo/bar.h>
    // or: #include <header.h> should again become #include <header.h>
    // see unit tests for more examples
    if (properties.inputFrom != -1) {
        int end = properties.inputTo;
        if (end >= rightBoundary || end == -1) {
            end = text.lastIndexOf(QLatin1Char('/'), rightBoundary - 1) + 1;
        }
        if (end > 0) {
            properties.prefixPath = text.mid(properties.inputFrom, end - properties.inputFrom);
            properties.inputFrom += properties.prefixPath.length();
        }
    }

    return properties;
}

namespace
{

QVector<KDevelop::IncludeItem> includeItemsForUrl(const QUrl& url, const IncludePathProperties& properties,
                                                  const ClangParsingEnvironment::IncludePaths& includePaths)
{
    QVector<IncludeItem> includeItems;
    Path::List paths;

    if (properties.local) {
        paths.reserve(1 + includePaths.project.size() + includePaths.system.size());
        paths.push_back(Path(url).parent());
        paths += includePaths.project;
        paths += includePaths.system;
    } else {
        paths = includePaths.system + includePaths.project;
    }

    // ensure we don't add duplicate paths
    QSet<Path> handledPaths; // search paths
    QSet<QString> foundIncludePaths; // found items

    int pathNumber = 0;
    for (auto searchPath : qAsConst(paths)) {
        if (handledPaths.contains(searchPath)) {
            continue;
        }
        handledPaths.insert(searchPath);

        if (!properties.prefixPath.isEmpty()) {
            searchPath.addPath(properties.prefixPath);
        }

        QDirIterator dirIterator(searchPath.toLocalFile());
        while (dirIterator.hasNext()) {
            dirIterator.next();
            KDevelop::IncludeItem item;
            item.name = dirIterator.fileName();

            if (item.name.startsWith(QLatin1Char('.')) || item.name.endsWith(QLatin1Char('~'))) { //filter out ".", "..", hidden files, and backups
                continue;
            }

            const auto info = dirIterator.fileInfo();
            item.isDirectory = info.isDir();

            // filter files that are not a header
            // note: system headers sometimes don't have any extension, and we still want to show those
            if (!item.isDirectory && item.name.contains(QLatin1Char('.')) && !ClangHelpers::isHeader(item.name)) {
                continue;
            }

            const QString fullPath = info.canonicalFilePath();
            if (foundIncludePaths.contains(fullPath)) {
                continue;
            } else {
                foundIncludePaths.insert(fullPath);
            }

            item.basePath = searchPath.toUrl();
            item.pathNumber = pathNumber;

            includeItems << item;
        }
        ++pathNumber;
    }

    return includeItems;
}
}

class IncludeFileCompletionItem : public AbstractIncludeFileCompletionItem<ClangNavigationWidget>
{
public:
    explicit IncludeFileCompletionItem(const IncludeItem& include)
        : AbstractIncludeFileCompletionItem<ClangNavigationWidget>(include)
    {}

    void execute(KTextEditor::View* view, const KTextEditor::Range& word) override
    {
        auto document = view->document();
        auto range = word;
        const int lineNumber = word.end().line();
        const QString line = document->line(lineNumber);
        const auto properties = IncludePathProperties::parseText(line, word.end().column());
        if (!properties.valid) {
            return;
        }

        QString newText = includeItem.isDirectory ? (includeItem.name + QLatin1Char('/')) : includeItem.name;

        if (properties.inputFrom == -1) {
            newText.prepend(QLatin1Char('<'));
        } else {
            range.setStart({lineNumber, properties.inputFrom});
        }
        if (properties.inputTo == -1) {
            // Add suffix
            if (properties.local) {
                newText += QLatin1Char('"');
            } else {
                newText += QLatin1Char('>');
            }

            // replace the whole line
            range.setEnd({lineNumber, static_cast<int>(line.size())});
        } else {
            range.setEnd({lineNumber, properties.inputTo});
        }

        document->replaceText(range, newText);

        if (includeItem.isDirectory) {
            // ensure we can continue to add files/paths when we just added a directory
            int offset = (properties.inputTo == -1) ? 1 : 0;
            view->setCursorPosition(range.start() + KTextEditor::Cursor(0, newText.length() - offset));
        } else {
            // place cursor at end of line
            view->setCursorPosition({lineNumber, document->lineLength(lineNumber)});
        }
    }
};

IncludePathCompletionContext::IncludePathCompletionContext(const DUContextPointer& context,
                                                           const ParseSessionData::Ptr& sessionData,
                                                           const QUrl& url,
                                                           const KTextEditor::Cursor& position,
                                                           const QString& text)
    : CodeCompletionContext(context, text, CursorInRevision::castFromSimpleCursor(position), 0)
{
    const IncludePathProperties properties = IncludePathProperties::parseText(text);

    if (!properties.valid) {
        return;
    }

    m_includeItems = includeItemsForUrl(url, properties, sessionData->environment().includes());
}

QList< CompletionTreeItemPointer > IncludePathCompletionContext::completionItems(bool& abort, bool)
{
    QList<CompletionTreeItemPointer> items;

    for (const auto& includeItem: qAsConst(m_includeItems)) {
        if (abort) {
            return items;
        }

        items << CompletionTreeItemPointer(new IncludeFileCompletionItem(includeItem));
    }

    return items;
}
