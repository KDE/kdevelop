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

#include <interfaces/icore.h>
#include <interfaces/ilanguagecontroller.h>

#include <QReadWriteLock>

namespace KDevelop {

class ILanguageSupportPrivate
{
public:
    mutable QReadWriteLock lock;
};

ILanguageSupport::ILanguageSupport()
    : d(new ILanguageSupportPrivate)
{
}

ILanguageSupport::~ILanguageSupport()
{
}

TopDUContext* ILanguageSupport::standardContext(const QUrl& url, bool proxyContext) {
  Q_UNUSED(proxyContext)
  return DUChain::self()->chainForDocument(url);
}

KTextEditor::Range ILanguageSupport::specialLanguageObjectRange(const QUrl& url, const KTextEditor::Cursor& position) {
    Q_UNUSED(url)
    Q_UNUSED(position)
    return KTextEditor::Range::invalid();
}

QPair<QUrl, KTextEditor::Cursor> ILanguageSupport::specialLanguageObjectJumpCursor(const QUrl& url, const KTextEditor::Cursor& position) {
    Q_UNUSED(url)
    Q_UNUSED(position)
    return QPair<QUrl, KTextEditor::Cursor>(QUrl(), KTextEditor::Cursor::invalid());
}

QWidget* ILanguageSupport::specialLanguageObjectNavigationWidget(const QUrl& url, const KTextEditor::Cursor& position) {
    Q_UNUSED(url)
    Q_UNUSED(position)
    return 0;
}

ICodeHighlighting* ILanguageSupport::codeHighlighting() const {
    return 0;
}

BasicRefactoring* ILanguageSupport::refactoring() const
{
    return nullptr;
}

ICreateClassHelper* ILanguageSupport::createClassHelper() const {
    return 0;
}

ILanguageSupport::WhitespaceSensitivity ILanguageSupport::whitespaceSensititivy() const
{
    return ILanguageSupport::Insensitive;
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
    return &d->lock;
}

}
