/*
    SPDX-FileCopyrightText: 2008 David Nolden <david.nolden.kdevelop@art-master.de>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ilanguagesupport.h"
#include "../duchain/duchain.h"
#include "../duchain/stringhelpers.h"

#include <QReadWriteLock>
#include <KTextEditor/Document>

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
    const auto joinedWord = [doc, &changedRange] {
        return doc->wordRangeAt(changedRange.start()).isEmpty() || doc->wordRangeAt(changedRange.end()).isEmpty();
    };
    return consistsOfWhitespace(doc->text(changedRange)) && !joinedWord() ? NoUpdateRequired : DefaultDelay;
}
}
