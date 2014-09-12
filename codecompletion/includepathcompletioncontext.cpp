/*
 * This file is part of KDevelop
 * Copyright 2014 Sergey Kalinichev <kalinichev.so.0@gmail.com>
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
//TODO: Try to reuse ClangHelpers::headerExtensions() instead
static const QStringList headerExtensions = {"h", "H", "hh", "hxx", "hpp", "tlh", "h++"};

struct IncludePathProperties
{
    QString prefixPath;
    bool local = false;
    bool valid = false;
};

IncludePathProperties includePathProperties(const QString& text)
{
    IncludePathProperties properties;

    QString line;
    const int idx = text.lastIndexOf('\n');
    if (idx != -1) {
        line = text.mid(idx + 1).trimmed();
    } else {
        line = text.trimmed();
    }

    const static QRegularExpression includeRegexp("^\\s*#\\s*include\\s*(<|\")(.*)");
    auto math = includeRegexp.match(line);
    if(!math.hasMatch()){
        return properties;
    }

    properties.valid = true;
    properties.local = math.captured(1) == "\"";

    const int slashIdx = math.captured(2).lastIndexOf('/');
    if (slashIdx != -1) {
        properties.prefixPath = math.captured(2).left(slashIdx);
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

            if (item.name.startsWith('.') || item.name.endsWith("~")) { //filter out ".", "..", hidden files, and backups
                continue;
            }

            const auto suffix = dirIterator.fileInfo().suffix();
            if (!suffix.isEmpty() && !headerExtensions.contains(suffix)) {
                continue;
            }

            const QString fullPath = dirIterator.fileInfo().canonicalFilePath();
            if (foundIncludePaths.contains(fullPath)) {
                continue;
            } else {
                foundIncludePaths.insert(fullPath);
            }

            item.basePath = searchPath.toUrl();
            item.isDirectory = dirIterator.fileInfo().isDir();
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
        QString newText = includeItem.isDirectory ? includeItem.name + '/' : includeItem.name;

        if (!includeItem.isDirectory) {
            auto line = document->line(word.start().line());
            //check whether closing '"' or '>' already inserted
            if (line.size() <= word.end().column() || (line[word.end().column()] != '\"' && line[word.end().column()] != '>')) {
                const static QRegularExpression includeRegexp("^\\s*#\\s*include\\s*(<|\")(.*)");
                QString lineText = document->text( {{word.start().line(), 0}, word.end()});
                auto match = includeRegexp.match(lineText);
                if (match.captured(1) == "\"") {
                    newText += '\"';
                } else if (match.captured(1) == "<") {
                    newText += '>';
                }
            }
        }

        document->replaceText(word, newText);
    }
};

IncludePathCompletionContext::IncludePathCompletionContext(const DUContextPointer& context,
                                                           const ParseSessionData::Ptr& sessionData,
                                                           const KTextEditor::Cursor& position,
                                                           const QString& text)
    : CodeCompletionContext(context, text, CursorInRevision::castFromSimpleCursor(position), 0)
{
    ParseSession session(sessionData);
    const IncludePathProperties properties = includePathProperties(text);

    if (!properties.valid) {
        return;
    }

    m_includeItems = includeItemsForUrl(context->url().toUrl(), properties, properties.local ?  session.environment().includes().project : session.environment().includes().system);
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
