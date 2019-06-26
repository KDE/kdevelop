/***************************************************************************
*   Copyright 2008 David Nolden  <david.nolden.kdevelop@art-master.de>    *
*   Copyright 2014 Kevin Funk <kfunk@kde.org>                             *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Library General Public License as       *
*   published by the Free Software Foundation; either version 2 of the    *
*   License, or (at your option) any later version.                       *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU Library General Public     *
*   License along with this program; if not, write to the                 *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
***************************************************************************/

#include "ilanguagesupport.h"
#include "../duchain/duchain.h"

#include <QReadWriteLock>

namespace KDevelop {
class ILanguageSupportPrivate
{
public:
    mutable QReadWriteLock lock;
};

ILanguageSupport::ILanguageSupport()
    : d_ptr(new ILanguageSupportPrivate)
{
}

ILanguageSupport::~ILanguageSupport()
{
}

TopDUContext* ILanguageSupport::standardContext(const QUrl& url, bool proxyContext)
{
    Q_UNUSED(proxyContext)
    return DUChain::self()->chainForDocument(url);
}

KTextEditor::Range ILanguageSupport::specialLanguageObjectRange(const QUrl& url, const KTextEditor::Cursor& position)
{
    Q_UNUSED(url)
    Q_UNUSED(position)
    return KTextEditor::Range::invalid();
}

QPair<QUrl, KTextEditor::Cursor> ILanguageSupport::specialLanguageObjectJumpCursor(const QUrl& url,
                                                                                   const KTextEditor::Cursor& position)
{
    Q_UNUSED(url)
    Q_UNUSED(position)
    return QPair<QUrl, KTextEditor::Cursor>(QUrl(), KTextEditor::Cursor::invalid());
}

QPair<QWidget*, KTextEditor::Range> ILanguageSupport::specialLanguageObjectNavigationWidget(const QUrl& url,
                                                                                            const KTextEditor::Cursor& position)
{
    Q_UNUSED(url)
    Q_UNUSED(position)
    return {
               nullptr, KTextEditor::Range::invalid()
    };
}

ICodeHighlighting* ILanguageSupport::codeHighlighting() const
{
    return nullptr;
}

BasicRefactoring* ILanguageSupport::refactoring() const
{
    return nullptr;
}

ICreateClassHelper* ILanguageSupport::createClassHelper() const
{
    return nullptr;
}

SourceFormatterItemList ILanguageSupport::sourceFormatterItems() const
{
    return SourceFormatterItemList();
}

QString ILanguageSupport::indentationSample() const
{
    return QString();
}

QReadWriteLock* ILanguageSupport::parseLock() const
{
    Q_D(const ILanguageSupport);

    return &d->lock;
}

int ILanguageSupport::suggestedReparseDelayForChange(KTextEditor::Document* doc,
                                                     const KTextEditor::Range& changedRange,
                                                     const QString& /*removedText*/, bool /*removal*/) const
{
    auto text = doc->text(changedRange);
    bool joinedWord = doc->wordRangeAt(changedRange.start()).isEmpty() ||
                      doc->wordRangeAt(changedRange.end()).isEmpty();

    auto isWhitespace = std::all_of(text.begin(), text.end(), [](const QChar& c) {
            return c.isSpace();
        });
    return (isWhitespace && !joinedWord) ? NoUpdateRequired : DefaultDelay;
}
}
