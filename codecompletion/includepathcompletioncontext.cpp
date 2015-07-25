/*
 * This file is part of KDevelop
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>
 * Copyright 2015 Milian Wolff <mail@milianw.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "includepathcompletioncontext.h"

#include "duchain/navigationwidget.h"
#include "duchain/clanghelpers.h"

#include <language/codecompletion/abstractincludefilecompletionitem.h>

#include <QDirIterator>
#include <QRegularExpression>

#include <KTextEditor/View>

#include <algorithm>

using namespace KDevelop;

namespace
{

struct IncludePathProperties
{
    // potentially already existing path to a directory
    QString prefixPath;
    // whether we look at a i.e. #include "local"
    // or a #include <global> include line
    bool local = false;
    // whether the line actually includes an #include
    bool valid = false;
    // start offset into @p text where to insert the new item
    int inputFrom = -1;
    // end offset into @p text where to insert the new item
    int inputTo = -1;
};

/**
 * Parse the last line of @p text and extract information about any existing include path from it.
 */
IncludePathProperties includePathProperties(const QString& text, int rightBoundary = -1)
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
            case FindInclude:
                if (text.midRef(idx, 7) != QLatin1String("include")) {
                    return properties;
                }
                idx += 6;
                state = FindType;
                properties.valid = true;
                break;
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

QList<KDevelop::IncludeItem> includeItemsForUrl(const QUrl& url, const IncludePathProperties& properties, const Path::List& includePaths )
{
    QList<IncludeItem> includeItems;
    Path::List paths = includePaths;

    if (properties.local) {
        paths.push_front(Path(url).parent());
    }

    auto last = std::unique(paths.begin(), paths.end());

    int pathNumber = 0;
    for (auto it = paths.begin(); it != last; it++ ) {
        auto searchPath = *it;
        if (!properties.prefixPath.isEmpty()) {
            searchPath.addPath(properties.prefixPath);
        }

        QSet<QString> foundIncludePaths;
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
    IncludeFileCompletionItem(const IncludeItem& include)
        : AbstractIncludeFileCompletionItem<ClangNavigationWidget>(include)
    {}

    virtual void execute(KTextEditor::View* view, const KTextEditor::Range& word) override
    {
        auto document = view->document();
        auto range = word;
        const int lineNumber = word.end().line();
        const QString line = document->line(lineNumber);
        const auto properties = includePathProperties(line, word.end().column());
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
            range.setEnd({lineNumber, line.size()});
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
    const IncludePathProperties properties = includePathProperties(text);

    if (!properties.valid) {
        return;
    }

    m_includeItems = includeItemsForUrl(url, properties, properties.local ?  sessionData->environment().includes().project : sessionData->environment().includes().system);
}

QList< CompletionTreeItemPointer > IncludePathCompletionContext::completionItems(bool& abort, bool)
{
    QList<CompletionTreeItemPointer> items;

    for (const auto& includeItem: m_includeItems) {
        if (abort) {
            return items;
        }

        items << CompletionTreeItemPointer(new IncludeFileCompletionItem(includeItem));
    }

    return items;
}
